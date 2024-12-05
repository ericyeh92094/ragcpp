// ragcpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <codecvt> // Deprecated in C++17 but still usable in C++20

#include "sqlite3.h"
#include "curl/curl.h"

#include "cppjieba/Jieba.hpp"
#include "nlohmann/json.hpp"

const char* const DICT_PATH = "./dict/jieba.dict.utf8";
const char* const HMM_PATH = "./dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
const char* const IDF_PATH = "./dict/idf.utf8";
const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";

cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);

// Conversion function using codecvt
std::string ConvertToUTF8(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

/*
std::string ConvertToUTF8(const std::wstring& wideString) {
    if (wideString.empty()) return std::string();
    int sizeRequired = WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8String(sizeRequired, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideString.c_str(), -1, &utf8String[0], sizeRequired, nullptr, nullptr);
    utf8String.pop_back(); // Remove the null terminator added by WideCharToMultiByte
    return utf8String;
}
*/

std::wstring stringToWstring(const std::string& str) {
    std::mbstate_t state = std::mbstate_t();
    const char* src = str.data();
    size_t len = std::mbsrtowcs(nullptr, &src, 0, &state);
    if (len == static_cast<size_t>(-1)) {
        throw std::runtime_error("Conversion error");
    }
    std::wstring result(len, L'\0');
    std::mbsrtowcs(&result[0], &src, len, &state);
    return result;
}

std::string tokenize_text(const std::string& text) {
    std::vector<std::string> words;
    jieba.Cut(text, words, true); // 使用精确模式分词

    // 可以选择去除停用词
    // std::vector<std::string> filtered_words;
    // for (const auto& word : words) {
    //     if (!jieba.IsStopWord(word)) {
    //         filtered_words.push_back(word);
    //     }
    // }

    // 将分词结果拼接为字符串，词之间以空格分隔
    std::string result = "";
    for (const auto& word : words) {
        result += word + " ";
    }
    return result;
}


std::string read_text_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Cannot open file：" << file_path << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

std::vector<std::string> split_paragraphs(const std::string& text) {
    std::vector<std::string> paragraphs;
    size_t start = 0, end = 0;
    while ((end = text.find('\n', start)) != std::string::npos) {
        std::string paragraph = text.substr(start, end - start);
        if (!paragraph.empty()) {
            paragraphs.push_back(paragraph);
        }
        start = end + 1;
    }
    if (start < text.size()) {
        paragraphs.push_back(text.substr(start));
    }
    return paragraphs;
}

float cosine_similarity(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    float dot_product = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
    for (size_t i = 0; i < vec1.size(); ++i) {
        dot_product += vec1[i] * vec2[i];
        norm_a += vec1[i] * vec1[i];
        norm_b += vec2[i] * vec2[i];
    }
    return dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b));
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void initialize_database(sqlite3*& db, bool newdb = false) {
    int rc = sqlite3_open("embeddings.db", &db);
    if (rc) {
        std::cerr << "Cannot open db：" << sqlite3_errmsg(db) << std::endl;
        return;
    }

    if (newdb)
    {
        const char* sql = "CREATE TABLE IF NOT EXISTS embeddings ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "text TEXT, "
            "embedding BLOB);";
        char* err_msg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error：" << err_msg << std::endl;
            sqlite3_free(err_msg);
        }
    }
}

std::vector<float> get_embedding(const std::string& text, const std::string& api_key) {
    CURL* curl = curl_easy_init();
    std::vector<float> embedding;

    if (curl) {
        std::string read_buffer;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/embeddings");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        nlohmann::json json_data;
        json_data["input"] = text;
        json_data["model"] = "text-embedding-ada-002";

        std::string post_fields = json_data.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed：" << curl_easy_strerror(res) << std::endl;
        }
        else {
            auto response_json = nlohmann::json::parse(read_buffer);
            if (response_json.contains("data")) {
                embedding = response_json["data"][0]["embedding"].get<std::vector<float>>();
            }
            else {
                std::cerr << "Cannot get embedding：" << response_json.dump() << std::endl;
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return embedding;
}

void insert_embedding(sqlite3* db, const std::string& text, const std::vector<float>& embedding) {
    sqlite3_stmt* stmt;
    const char* insert_sql = "INSERT INTO embeddings (text, embedding) VALUES (?, ?);";
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL query error：" << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, text.c_str(), -1, SQLITE_TRANSIENT);

    const void* blob_data = static_cast<const void*>(embedding.data());
    int blob_size = static_cast<int>(embedding.size() * sizeof(float));

    sqlite3_bind_blob(stmt, 2, blob_data, blob_size, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "exec SQL fail：" << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

std::vector<std::pair<int, std::vector<float>>> load_embeddings(sqlite3* db) {
    std::vector<std::pair<int, std::vector<float>>> embeddings;
    const char* select_sql = "SELECT id, embedding FROM embeddings;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL query failed：" << sqlite3_errmsg(db) << std::endl;
        return embeddings;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const void* blob_data = sqlite3_column_blob(stmt, 1);
        int blob_size = sqlite3_column_bytes(stmt, 1);

        int num_floats = blob_size / sizeof(float);
        std::vector<float> embedding(num_floats);
        memcpy(embedding.data(), blob_data, blob_size);

        embeddings.emplace_back(id, embedding);
    }

    sqlite3_finalize(stmt);
    return embeddings;
}

struct SimilarityResult {
    int id;
    float similarity;
};

std::vector<SimilarityResult> retrieve_similar_embeddings(
    const std::vector<float>& query_embedding,
    const std::vector<std::pair<int, std::vector<float>>>& embeddings) {

    std::vector<SimilarityResult> results;
    for (const auto& [id, embedding] : embeddings) {
        float sim = cosine_similarity(query_embedding, embedding);
        results.push_back({ id, sim });
    }

    std::sort(results.begin(), results.end(), [](const SimilarityResult& a, const SimilarityResult& b) {
        return a.similarity > b.similarity;
        });

    return results;
}

std::string get_text_by_id(sqlite3* db, int id) {
    const char* select_sql = "SELECT text FROM embeddings WHERE id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL query failed：" << sqlite3_errmsg(db) << std::endl;
        return "";
    }

    sqlite3_bind_int(stmt, 1, id);

    std::string text;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* text_data = sqlite3_column_text(stmt, 0);
        text = reinterpret_cast<const char*>(text_data);
    }
    else {
        std::cerr << "Cannot find ID=" << id << " text." << std::endl;
    }

    sqlite3_finalize(stmt);
    return text;
}

std::string generate_answer(const std::wstring& context, const std::wstring& question, const std::string& api_key) {
    CURL* curl = curl_easy_init();
    std::string answer;

    if (curl) {
        std::string read_buffer;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::wstring promptW = L"請根據以下內容回答問題：\n" + context + L"\n問題：" + question + L"\n答案：";
        std::string prompt = ConvertToUTF8(promptW);
        //std::string prompt = "Answer the question based on the context：\n" + context + "\nquestion：" + question + "\nanswer：";

        nlohmann::json json_data;
        json_data["model"] = "gpt-4";
        json_data["messages"] = nlohmann::json::array({
            {
                {"role", "system"},
                {"content", "You are a useful assistant."}
            },
            {
                {"role", "user"},
                {"content", prompt}
            }
            });

        std::string post_fields = json_data.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed：" << curl_easy_strerror(res) << std::endl;
        }
        else {
            auto response_json = nlohmann::json::parse(read_buffer);
            if (response_json.contains("choices")) {
                answer = response_json["choices"][0]["message"]["content"].get<std::string>();
            }
            else {
                std::cerr << "Cannot get answer：" << response_json.dump() << std::endl;
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return answer;
}


int main() {
    // 读取 OpenAI API 密钥
    std::string api_key = "sk-r7ZDLUo1wl297PhpX-PBae-fYuxL7vrSDLFy7RbL1zT3BlbkFJN7HB2Hc0KTgIIPi4PdrXCRXVlEDDJ9B1SEObIFyIQA";

    // 初始化数据库
    sqlite3* db = nullptr;
    initialize_database(db, false);
    SetConsoleCP(CP_UTF8);
#
    // 读取并预处理文本文件
    std::string file_content = read_text_file("he.txt");
    if (file_content.empty()) {
        std::cerr << "Empty text。" << std::endl;
        return -1;
    }
    std::vector<std::string> paragraphs = split_paragraphs(file_content);

    // 为每个段落生成嵌入并存储到数据库
    for (const auto& paragraph : paragraphs) {
        if (paragraph.empty()) continue;
        std::string tokenized_paragraph = tokenize_text(paragraph);

        // 使用分词后的文本生成嵌入
        auto embedding = get_embedding(tokenized_paragraph, api_key);
        if (!embedding.empty()) {
            insert_embedding(db, paragraph, embedding);
        }
        else {
            std::cerr << "Embedding failed. Skip the text" << std::endl;
        }
    }


    // 接受用户查询
    std::string user_query;
    std::cout << "User：" << std::endl;
    std::getline(std::cin, user_query);

    // 生成查询的嵌入向量
    std::string tokenized_query = tokenize_text(user_query);

    // 生成查询的嵌入向量
    auto query_embedding = get_embedding(tokenized_query, api_key);
    if (query_embedding.empty()) {
        std::cerr << "Get answer failed" << std::endl;
        return -1;
    }

    // 检索最相似的段落
    auto embeddings = load_embeddings(db);
    auto similar_results = retrieve_similar_embeddings(query_embedding, embeddings);

    // 获取最相关的段落文本
    int top_k = 5; // 您可以根据需要调整
    std::string context;
    for (int i = 0; i < top_k && i < similar_results.size(); ++i) {
        int id = similar_results[i].id;
        std::string text = get_text_by_id(db, id);
        context += text + "\n";
    }

    std::wstring contextW = stringToWstring(context);
    std::wstring user_queryW = stringToWstring(user_query);
 
    // 生成回答
    std::string answer = generate_answer(contextW, user_queryW, api_key);

    // 输出结果
    std::cout << "回答：" << answer << std::endl;

    // 关闭数据库
    sqlite3_close(db);

    return 0;
}

