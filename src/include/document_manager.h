#pragma once
// document_manager.h

#ifndef DOCUMENT_MANAGER_H
#define DOCUMENT_MANAGER_H

#include <string>
#include "sqlite3.h"

void embed_file(const std::wstring& file_path, const std::string& api_key, sqlite3* db);

void process_paths(const std::vector<std::wstring>& paths, const std::string& api_key, sqlite3* db);

void generate_answer(const std::wstring& user_query, const std::string& api_key, sqlite3* db);

void monitor_progress(sqlite3* db);

#endif // DOCUMENT_MANAGER_H
