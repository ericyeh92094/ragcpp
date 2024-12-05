
// args.cpp

#include "args.h"
#include <iostream>
#include <algorithm>

// args.cpp

#include "args.h"
#include <iostream>
#include <algorithm>
#include <cstdlib>

ProgramOptions parse_arguments(int argc, wchar_t* argv[]) {
    ProgramOptions options;

    std::vector<std::wstring> args(argv + 1, argv + argc);

    for (size_t i = 0; i < args.size(); ++i) {
        const std::wstring& arg = args[i];

        if (arg == L"-h" || arg == L"--help") {
            // Display help message
            std::wcout << L"Usage: RAGSystem [options]\n"
                L"Options:\n"
                L"  -e, --embed FILE_OR_DIR [FILE_OR_DIR...]  Embed files or directories\n"
                L"  -d, --delete ID [ID...]                   Delete documents\n"
                L"  -q, --query QUERY                         Query and generate an answer\n"
                L"  -l, --list                                List existing documents\n"
                L"  -m, --monitor                             Monitor embedding progress\n"
                L"  -h, --help                                Display this help message\n";
            exit(0);
        }
        else if (arg == L"-e" || arg == L"--embed") {
            options.embed = true;
            // Collect all subsequent arguments that are not options (do not start with '-')
            while (i + 1 < args.size() && args[i + 1][0] != L'-') {
                options.file_paths.push_back(args[++i]);
            }
            if (options.file_paths.empty()) {
                std::wcerr << L"Error: --embed option requires at least one file or directory." << std::endl;
                exit(1);
            }
        }
        else if (arg == L"-d" || arg == L"--delete") {
            options.delete_docs = true;
            // Collect all subsequent arguments that are not options
            while (i + 1 < args.size() && args[i + 1][0] != L'-') {
                std::wstring id_str = args[++i];
                try {
                    int id = std::stoi(id_str);
                    options.doc_ids_to_delete.push_back(id);
                }
                catch (const std::invalid_argument&) {
                    std::wcerr << L"Invalid document ID: " << id_str << std::endl;
                    exit(1);
                }
            }
            if (options.doc_ids_to_delete.empty()) {
                std::wcerr << L"Error: --delete option requires at least one document ID." << std::endl;
                exit(1);
            }
        }
        else if (arg == L"-q" || arg == L"--query") {
            options.query = true;
            if (i + 1 < args.size() && args[i + 1][0] != L'-') {
                options.user_query = args[++i];
            }
            else {
                std::wcerr << L"Error: --query option requires a query string." << std::endl;
                exit(1);
            }
        }
        else if (arg == L"-l" || arg == L"--list") {
            options.list_docs = true;
        }
        else if (arg == L"-m" || arg == L"--monitor") {
            options.monitor_progress = true;
        }
        else {
            std::wcerr << L"Unknown option or argument: " << arg << std::endl;
            exit(1);
        }
    }

    // Validate that only one primary option is selected
    int command_count = options.embed + options.delete_docs + options.query + options.list_docs + options.monitor_progress;
    if (command_count > 1) {
        std::wcerr << L"Options --embed, --delete, --query, --list, and --monitor cannot be used together." << std::endl;
        exit(1);
    }

    if (command_count == 0) {
        std::wcerr << L"You must specify one of the options: --embed, --delete, --query, --list, or --monitor." << std::endl;
        exit(1);
    }

    return options;
}
