// main.cpp

#include <iostream>
#include <cstdlib>
#include "args.h"
#include "database.h"
#include "document_manager.h"

#ifdef _WIN32
#include <windows.h>
#endif

int wmain(int argc, wchar_t* argv[]) {
#ifdef _WIN32
    // Set console output code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    std::wcout.imbue(std::locale("en_US.UTF-8"));
#endif

    // Read OpenAI API key
    char* api_key_cstr = std::getenv("OPENAI_API_KEY");
    if (!api_key_cstr) {
        std::cerr << "Please set the OPENAI_API_KEY environment variable." << std::endl;
        return -1;
    }
    std::string api_key(api_key_cstr);

    // Parse command-line arguments
    ProgramOptions options = parse_arguments(argc, argv);

    // Initialize the database
    sqlite3* db = nullptr;
    initialize_database(db);

    if (options.embed) {
        process_paths(options.file_paths, api_key, db);
    }
    else if (options.delete_docs) {
        delete_documents(options.doc_ids_to_delete, db);
    }
    else if (options.query) {
        generate_answer(options.user_query, api_key, db);
    }
    else if (options.list_docs) {
        list_documents(db);
    }
    else if (options.monitor_progress) {
        monitor_progress(db);
    }

    // Close the database
    sqlite3_close(db);

    return 0;
}
