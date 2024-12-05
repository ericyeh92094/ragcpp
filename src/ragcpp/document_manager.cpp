// document_manager.cpp

#include "database.h"
#include "document_manager.h"
#include "text_processing.h"
#include "openai_api.h"
#include "file_handler.h"
#include "utils.h"
#include "encoding_utils.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cstring>
#include <thread>
#include <chrono>

// Define SimilarityResult struct and retrieve_similar_embeddings function
struct SimilarityResult {
    int id;
    int doc_id;
    float similarity;
};

std::vector<SimilarityResult> retrieve_similar_embeddings(
    const std::vector<float>& query_embedding,
    sqlite3* db) {

    std::vector<SimilarityResult> results;

    const char* select_sql = "SELECT id, doc_id, embedding FROM embeddings;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return results;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int doc_id = sqlite3_column_int(stmt, 1);
        const void* blob_data = sqlite3_column_blob(stmt, 2);
        int blob_size = sqlite3_column_bytes(stmt, 2);

        int num_floats = blob_size / sizeof(float);
        std::vector<float> embedding(num_floats);
        memcpy(embedding.data(), blob_data, blob_size);

        float sim = cosine_similarity(query_embedding, embedding);
        results.push_back({ id, doc_id, sim });
    }

    sqlite3_finalize(stmt);

    std::sort(results.begin(), results.end(), [](const SimilarityResult& a, const SimilarityResult& b) {
        return a.similarity > b.similarity;
        });

    return results;
}

void monitor_progress(sqlite3* db) {
    while (true) {
        // Query the progress table
        const char* select_sql = "SELECT doc_id, total_paragraphs, paragraphs_processed, status, last_updated FROM progress;";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
            break;
        }

        // Clear the console screen (optional)
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        std::wcout << L"Embedding Progress:\n";
        std::wcout << L"Doc ID | Status     | Progress | Last Updated\n";
        std::wcout << L"----------------------------------------------\n";

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            int doc_id = sqlite3_column_int(stmt, 0);
            int total_paragraphs = sqlite3_column_int(stmt, 1);
            int paragraphs_processed = sqlite3_column_int(stmt, 2);
            const unsigned char* status_text = sqlite3_column_text(stmt, 3);
            const unsigned char* last_updated_text = sqlite3_column_text(stmt, 4);

            std::string status = reinterpret_cast<const char*>(status_text);
            std::string last_updated = reinterpret_cast<const char*>(last_updated_text);

            double progress = (static_cast<double>(paragraphs_processed) / total_paragraphs) * 100;

            std::wcout << doc_id << L"      | " << utf8_to_wstring(status) << L" | "
                << progress << L"%    | " << utf8_to_wstring(last_updated) << L"\n";
        }

        sqlite3_finalize(stmt);

        // Wait before the next update
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

// Helper function to check if the extension is supported
bool is_supported_extension(const std::wstring& extension) {
    static const std::vector<std::wstring> supported_extensions = { L".txt", L".pdf", L".png", L".jpg", L".jpeg", L".bmp" };
    return std::find(supported_extensions.begin(), supported_extensions.end(), extension) != supported_extensions.end();
}

void process_paths(const std::vector<std::wstring>& paths, const std::string& api_key, sqlite3* db) {
    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            if (std::filesystem::is_regular_file(path)) {
                // Process a single file
                embed_file(path, api_key, db);
            }
            else if (std::filesystem::is_directory(path)) {
                // Recursively process directory
                for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file()) {
                        // Check if the file has a supported extension
                        std::wstring extension = entry.path().extension().wstring();
                        if (is_supported_extension(extension)) {
                            embed_file(entry.path().wstring(), api_key, db);
                        }
                        else {
                            std::wcerr << L"Unsupported file format: " << extension << L" Skipping file: " << entry.path().wstring() << std::endl;
                        }
                    }
                }
            }
            else {
                std::wcerr << L"Unsupported path type (not a file or directory): " << path << std::endl;
            }
        }
        else {
            std::wcerr << L"Path does not exist: " << path << std::endl;
        }
    }
}

void embed_file(const std::wstring& file_path, const std::string& api_key, sqlite3* db) {
    // Get file name
    std::wstring file_name = std::filesystem::path(file_path).filename().wstring();

    // Insert document info, get doc_id
    int doc_id = insert_document(db, file_name);

    if (doc_id == -1) {
        std::cerr << "Document insertion failed, skipping embedding." << std::endl;
        return;
    }

    std::wcout << L"Embedding document (ID: " << doc_id << L"): " << file_name << std::endl;

    // Detect file extension
    std::wstring extension = get_file_extension(file_path);

    std::wstring file_content;

    if (extension == L".txt") {
        // Handle text files
        file_content = read_text_file(file_path);
    }
    else if (extension == L".pdf") {
        // Handle PDF files
        file_content = extract_text_from_pdf(file_path);
    }
    /*
    else if (extension == L".png" || extension == L".jpg" || extension == L".jpeg" || extension == L".bmp") {
        // Handle image files
        file_content = extract_text_from_image(file_path);
    }
    */
    else {
        std::wcerr << L"Unsupported file format: " << extension << std::endl;
        return;
    }

    if (file_content.empty()) {
        std::cerr << "File content is empty, cannot process." << std::endl;
        return;
    }

    // Split into paragraphs
    std::vector<std::wstring> paragraphs = split_paragraphs(file_content);

    int total_paragraphs = static_cast<int>(paragraphs.size());
    int paragraphs_processed = 0;
    // Insert initial progress record
    insert_progress(db, doc_id, total_paragraphs, paragraphs_processed, "In Progress");

    // Generate embeddings for each paragraph and store in the database
    for (const auto& paragraph : paragraphs) {
        if (paragraph.empty()) continue;

        // Tokenize the paragraph
        std::wstring tokenized_paragraph = tokenize_text(paragraph);

        // Generate embedding using the tokenized text
        auto embedding = get_embedding(tokenized_paragraph, api_key);
        if (!embedding.empty()) {
            insert_embedding(db, doc_id, paragraph, embedding);
            paragraphs_processed++;

            // Update progress after each paragraph
            update_progress(db, doc_id, paragraphs_processed);

            std::wcout << L"Embedded paragraph: " << paragraph.substr(0, 30) << L"..." << std::endl;
        }
        else {
            std::cerr << "Failed to generate embedding, skipping this paragraph." << std::endl;
        }
    }
    // After processing all paragraphs
    update_progress_status(db, doc_id, "Completed");
}

void generate_answer(const std::wstring& user_query, const std::string& api_key, sqlite3* db) {
    // Tokenize user query
    std::wstring tokenized_query = tokenize_text(user_query);

    // Generate query embedding
    auto query_embedding = get_embedding(tokenized_query, api_key);
    if (query_embedding.empty()) {
        std::cerr << "Failed to generate query embedding." << std::endl;
        return;
    }

    // Retrieve most similar embeddings
    auto similar_results = retrieve_similar_embeddings(query_embedding, db);

    if (similar_results.empty()) {
        std::cerr << "No relevant embeddings found." << std::endl;
        return;
    }

    // Get top relevant paragraphs and document info
    int top_k = 5; // Adjust as needed
    std::wstring context;
    std::wstring citations;
    for (int i = 0; i < top_k && i < similar_results.size(); ++i) {
        int id = similar_results[i].id;
        int doc_id = similar_results[i].doc_id;
        std::wstring text = get_text_by_id(db, id);
        std::wstring file_name = get_document_info(db, doc_id);

        context += L"[" + std::to_wstring(i + 1) + L"] " + text + L"\n";
        citations += L"[" + std::to_wstring(i + 1) + L"] From document: " + file_name + L"\n";
    }

    // Generate answer
    std::wstring answer = generate_answer_from_context(context, user_query, api_key);

    // Output result with citations
    std::wcout << L"Answer:\n" << answer << std::endl;
    std::wcout << L"Citations:\n" << citations << std::endl;
}


