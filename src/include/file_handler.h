#pragma once
// file_handler.h

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>

std::wstring read_text_file(const std::wstring& file_path);

std::wstring extract_text_from_pdf(const std::wstring& pdf_path);

//std::wstring extract_text_from_image(const std::wstring& image_path);

std::wstring get_file_extension(const std::wstring& file_path);

#endif // FILE_HANDLER_H
