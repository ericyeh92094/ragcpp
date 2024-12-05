#pragma once
// openai_api.h

#ifndef OPENAI_API_H
#define OPENAI_API_H

#include <string>
#include <vector>

std::vector<float> get_embedding(const std::wstring& text, const std::string& api_key);

std::wstring generate_answer_from_context(const std::wstring& context, const std::wstring& question, const std::string& api_key);

#endif // OPENAI_API_H
