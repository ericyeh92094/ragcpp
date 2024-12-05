// database.cpp

#include "database.h"
#include <iostream>
#include "encoding_utils.h"

void initialize_database(sqlite3*& db) {
    int rc = sqlite3_open("embeddings.db", &db);
    if (rc) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS documents ("
        "doc_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "file_name TEXT);"
        "CREATE TABLE IF NOT EXISTS embeddings ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "doc_id INTEGER, "
        "text TEXT, "
        "embedding BLOB, "
        "FOREIGN KEY(doc_id) REFERENCES documents(doc_id));"
        "CREATE TABLE IF NOT EXISTS progress("
        "doc_id INTEGER PRIMARY KEY,"
        "total_paragraphs INTEGER,"
        "paragraphs_processed INTEGER,"
        "status TEXT,"
        "last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    char* err_msg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
}

int insert_document(sqlite3* db, const std::wstring& file_name) {
    sqlite3_stmt* stmt;
    const char* insert_sql = "INSERT INTO documents (file_name) VALUES (?);";
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    std::string utf8_file_name = wstring_to_utf8(file_name);
    sqlite3_bind_text(stmt, 1, utf8_file_name.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute SQL statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    int doc_id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);

    return doc_id;
}

void insert_embedding(sqlite3* db, int doc_id, const std::wstring& text, const std::vector<float>& embedding) {
    sqlite3_stmt* stmt;
    const char* insert_sql = "INSERT INTO embeddings (doc_id, text, embedding) VALUES (?, ?, ?);";
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::string utf8_text = wstring_to_utf8(text);

    sqlite3_bind_int(stmt, 1, doc_id);
    sqlite3_bind_text(stmt, 2, utf8_text.c_str(), -1, SQLITE_TRANSIENT);

    const void* blob_data = static_cast<const void*>(embedding.data());
    int blob_size = static_cast<int>(embedding.size() * sizeof(float));

    sqlite3_bind_blob(stmt, 3, blob_data, blob_size, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to execute SQL statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void delete_documents(const std::vector<int>& doc_ids, sqlite3* db) {
    sqlite3_stmt* stmt;

    // Delete from embeddings
    const char* delete_embeddings_sql = "DELETE FROM embeddings WHERE doc_id = ?;";
    int rc = sqlite3_prepare_v2(db, delete_embeddings_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    for (int doc_id : doc_ids) {
        sqlite3_bind_int(stmt, 1, doc_id);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to delete embeddings: " << sqlite3_errmsg(db) << std::endl;
        }
        else {
            std::cout << "Deleted embeddings for document ID " << doc_id << "." << std::endl;
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);

    // Delete from documents
    const char* delete_documents_sql = "DELETE FROM documents WHERE doc_id = ?;";
    rc = sqlite3_prepare_v2(db, delete_documents_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    for (int doc_id : doc_ids) {
        sqlite3_bind_int(stmt, 1, doc_id);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to delete document: " << sqlite3_errmsg(db) << std::endl;
        }
        else {
            std::cout << "Deleted document ID " << doc_id << "." << std::endl;
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
}

void list_documents(sqlite3* db) {
    const char* select_sql = "SELECT doc_id, file_name FROM documents;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::wcout << L"Existing documents:" << std::endl;
    std::wcout << L"Doc ID\tFile Name" << std::endl;
    std::wcout << L"-------------------------" << std::endl;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int doc_id = sqlite3_column_int(stmt, 0);
        const unsigned char* file_name_text = sqlite3_column_text(stmt, 1);
        std::string utf8_file_name = reinterpret_cast<const char*>(file_name_text);
        std::wstring file_name = utf8_to_wstring(utf8_file_name);

        std::wcout << doc_id << L"\t" << file_name << std::endl;
    }

    sqlite3_finalize(stmt);
}

std::wstring get_text_by_id(sqlite3* db, int id) {
    const char* select_sql = "SELECT text FROM embeddings WHERE id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return L"";
    }

    sqlite3_bind_int(stmt, 1, id);

    std::wstring text;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* text_data = sqlite3_column_text(stmt, 0);
        std::string utf8_text = reinterpret_cast<const char*>(text_data);
        text = utf8_to_wstring(utf8_text);
    }
    else {
        std::cerr << "No text found for ID " << id << "." << std::endl;
    }

    sqlite3_finalize(stmt);
    return text;
}

std::wstring get_document_info(sqlite3* db, int doc_id) {
    const char* select_sql = "SELECT file_name FROM documents WHERE doc_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return L"";
    }

    sqlite3_bind_int(stmt, 1, doc_id);

    std::wstring file_name;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* text_data = sqlite3_column_text(stmt, 0);
        std::string utf8_file_name = reinterpret_cast<const char*>(text_data);
        file_name = utf8_to_wstring(utf8_file_name);
    }
    else {
        std::cerr << "No document info found for doc ID " << doc_id << "." << std::endl;
    }

    sqlite3_finalize(stmt);
    return file_name;
}

void insert_progress(sqlite3* db, int doc_id, int total_paragraphs, int paragraphs_processed, const std::string& status) {
    const char* insert_sql = "INSERT INTO progress (doc_id, total_paragraphs, paragraphs_processed, status) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, doc_id);
    sqlite3_bind_int(stmt, 2, total_paragraphs);
    sqlite3_bind_int(stmt, 3, paragraphs_processed);
    sqlite3_bind_text(stmt, 4, status.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to insert progress: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void update_progress(sqlite3* db, int doc_id, int paragraphs_processed) {
    const char* update_sql = "UPDATE progress SET paragraphs_processed = ?, last_updated = CURRENT_TIMESTAMP WHERE doc_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, paragraphs_processed);
    sqlite3_bind_int(stmt, 2, doc_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to update progress: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

void update_progress_status(sqlite3* db, int doc_id, const std::string& status) {
    const char* update_sql = "UPDATE progress SET status = ?, last_updated = CURRENT_TIMESTAMP WHERE doc_id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot prepare SQL statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, doc_id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to update progress status: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}
