#pragma once
// database.h

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "sqlite3.h"

void initialize_database(sqlite3*& db);

int insert_document(sqlite3* db, const std::wstring& file_name);

void insert_embedding(sqlite3* db, int doc_id, const std::wstring& text, const std::vector<float>& embedding);

void delete_documents(const std::vector<int>& doc_ids, sqlite3* db);

void list_documents(sqlite3* db);

std::wstring get_text_by_id(sqlite3* db, int id);

std::wstring get_document_info(sqlite3* db, int doc_id);

void insert_progress(sqlite3* db, int doc_id, int total_paragraphs, int paragraphs_processed, const std::string& status);
void update_progress(sqlite3* db, int doc_id, int paragraphs_processed);
void update_progress_status(sqlite3* db, int doc_id, const std::string& status);

#endif // DATABASE_H
