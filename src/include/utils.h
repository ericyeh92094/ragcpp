#pragma once
// utils.h

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <cstddef>

float cosine_similarity(const std::vector<float>& vec1, const std::vector<float>& vec2);

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

#endif // UTILS_H
