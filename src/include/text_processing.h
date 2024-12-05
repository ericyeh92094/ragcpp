#pragma once
// text_processing.h

#ifndef TEXT_PROCESSING_H
#define TEXT_PROCESSING_H

#include <string>
#include <vector>

std::wstring tokenize_text(const std::wstring& text);

std::vector<std::wstring> split_paragraphs(const std::wstring& text);

#endif // TEXT_PROCESSING_H
