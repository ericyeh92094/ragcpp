// openai_api.cpp

#include "openai_api.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include "utils.h"
#include "encoding_utils.h"

std::vector<float> get_embedding(const std::wstring& wtext, const std::string& api_key) {
    std::string text = wstring_to_utf8(wtext);

    CURL* curl = curl_easy_init();
    std::vector<float> embedding;

    if (curl) {
        std::string read_buffer;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/embeddings");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        struct curl_slist* headers = nullptr;
        std::string auth_header = "Authorization: Bearer " + api_key;
        headers = curl_slist_append(headers, auth_header.c_str());
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
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            auto response_json = nlohmann::json::parse(read_buffer);
            if (response_json.contains("data")) {
                embedding = response_json["data"][0]["embedding"].get<std::vector<float>>();
            }
            else {
                std::cerr << "Failed to get embedding: " << response_json.dump() << std::endl;
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return embedding;
}

std::wstring generate_answer_from_context(const std::wstring& wcontext, const std::wstring& wquestion, const std::string& api_key) {
    std::string context = wstring_to_utf8(wcontext);
    std::string question = wstring_to_utf8(wquestion);

    CURL* curl = curl_easy_init();
    std::wstring answer;

    if (curl) {
        std::string read_buffer;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        struct curl_slist* headers = nullptr;
        std::string auth_header = "Authorization: Bearer " + api_key;
        headers = curl_slist_append(headers, auth_header.c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string system_prompt = "You are an assistant that provides answers with proper citations from the provided context.";
        nlohmann::json json_data;
        json_data["model"] = "gpt-4";
        json_data["messages"] = {
            {{"role", "system"}, {"content", system_prompt}},
            {{"role", "user"}, {"content", "Context:\n" + context + "\n\nQuestion: " + question}}
        };
        json_data["temperature"] = 0.7;

        std::string post_fields = json_data.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            auto response_json = nlohmann::json::parse(read_buffer);
            if (response_json.contains("choices")) {
                std::string utf8_answer = response_json["choices"][0]["message"]["content"].get<std::string>();
                answer = utf8_to_wstring(utf8_answer);
            }
            else {
                std::cerr << "Failed to get answer: " << response_json.dump() << std::endl;
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return answer;
}
