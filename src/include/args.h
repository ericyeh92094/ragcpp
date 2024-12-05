#pragma once
// args.h

#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <vector>

struct ProgramOptions {
    bool embed = false;
    std::vector<std::wstring> file_paths;
    bool query = false;
    std::wstring user_query;
    bool delete_docs = false;
    std::vector<int> doc_ids_to_delete;
    bool list_docs = false;
    bool monitor_progress = false;
};
ProgramOptions parse_arguments(int argc, wchar_t* argv[]);

#endif // ARGS_H
