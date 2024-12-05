#pragma once
// encoding_utils.h

#ifndef ENCODING_UTILS_H
#define ENCODING_UTILS_H

#include <string>

std::string wstring_to_utf8(const std::wstring& wstr);
std::wstring utf8_to_wstring(const std::string& str);

#endif // ENCODING_UTILS_H
