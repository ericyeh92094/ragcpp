// utils.cpp

#include "utils.h"
#include <cmath>
#include <cstring>
#include <string>

float cosine_similarity(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    float dot_product = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
    for (size_t i = 0; i < vec1.size(); ++i) {
        dot_product += vec1[i] * vec2[i];
        norm_a += vec1[i] * vec1[i];
        norm_b += vec2[i] * vec2[i];
    }
    return dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b) + 1e-8);
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), total_size);
    return total_size;
}
