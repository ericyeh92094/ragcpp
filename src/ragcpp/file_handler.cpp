// file_handler.cpp

#include "file_handler.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include "encoding_utils.h"

#ifdef _WIN32
// Windows-specific includes
extern "C" {
#include <mupdf/fitz.h>
}
//#include <tesseract/baseapi.h>
//#include <leptonica/allheaders.h>
#endif

std::wstring read_text_file(const std::wstring& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        std::wcerr << L"Cannot open file: " << file_path << std::endl;
        return L"";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    return utf8_to_wstring(content);
}

std::wstring extract_text_from_pdf(const std::wstring& pdf_path) {
    std::wstring extracted_text;

#ifdef _WIN32
    // Windows: Use MuPDF
    std::string pdf_path_utf8 = wstring_to_utf8(pdf_path);

    fz_context* ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!ctx) {
        std::cerr << "Cannot create MuPDF context." << std::endl;
        return L"";
    }

    fz_register_document_handlers(ctx);

    fz_document* doc = fz_open_document(ctx, pdf_path_utf8.c_str());
    if (!doc) {
        std::cerr << "Cannot open PDF file: " << pdf_path_utf8 << std::endl;
        fz_drop_context(ctx);
        return L"";
    }

    int page_count = fz_count_pages(ctx, doc);

    for (int i = 0; i < page_count; ++i) {
        fz_rect bbox;

        fz_page* page = fz_load_page(ctx, doc, i);
        bbox = fz_bound_page(ctx, page);
        fz_stext_page* text_page = fz_new_stext_page(ctx, bbox);
        fz_stext_options options;
        memset(&options, 0, sizeof(options));
        fz_device* text_device = fz_new_stext_device(ctx, text_page, &options);

        fz_run_page(ctx, page, text_device, fz_identity, NULL);

        //char* page_text = fz_copy_selected_text(ctx, text_page, NULL);
         // Extract text from the text page
        fz_buffer* buf = fz_new_buffer(ctx, 8096);
        fz_output* fout = fz_new_output_with_buffer(ctx, buf);
        fz_print_stext_page_as_text(ctx, fout, text_page);

        char* page_text = (char *)fz_string_from_buffer(ctx, buf);
  
        if (page_text) {
            std::string utf8_text(page_text);
            extracted_text += utf8_to_wstring(utf8_text) + L"\n";
            //fz_free(ctx, page_text);
            fz_drop_buffer(ctx, buf);
        }
        fz_close_output(ctx, fout);
        fz_close_device(ctx, text_device);

        fz_drop_output(ctx, fout);
        fz_drop_device(ctx, text_device);
        fz_drop_stext_page(ctx, text_page);
        fz_drop_page(ctx, page);
    }

    fz_drop_document(ctx, doc);
    fz_drop_context(ctx);

#endif

    return extracted_text;
}

#if 0
std::wstring extract_text_from_image(const std::wstring& image_path) {
    std::wstring outText;

    // Create Tesseract API object
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();

    // Initialize Tesseract
    if (api->Init(NULL, "chi_sim")) {
        std::cerr << "Could not initialize Tesseract OCR." << std::endl;
        delete api;
        return L"";
    }

    // Read image
    std::string image_path_utf8 = wstring_to_utf8(image_path);
    Pix* image = pixRead(image_path_utf8.c_str());
    if (!image) {
        std::cerr << "Cannot open image file: " << image_path_utf8 << std::endl;
        api->End();
        delete api;
        return L"";
    }

    api->SetImage(image);
    // Get OCR result
    char* text = api->GetUTF8Text();
    if (text) {
        std::string utf8_text(text);
        outText = utf8_to_wstring(utf8_text);
        delete[] text;
    }
    else {
        std::cerr << "OCR recognition failed." << std::endl;
    }

    // Release resources
    pixDestroy(&image);
    api->End();
    delete api;

    return outText;
}
#endif

std::wstring get_file_extension(const std::wstring& file_path) {
    return std::filesystem::path(file_path).extension().wstring();
}
