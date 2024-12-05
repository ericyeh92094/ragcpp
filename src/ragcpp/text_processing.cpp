// text_processing.cpp

#include "text_processing.h"
#include "cppjieba/Jieba.hpp"
#include <sstream>
#include "encoding_utils.h"

const char* const DICT_PATH = "./dict/jieba.dict.utf8";
const char* const HMM_PATH = "./dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
const char* const IDF_PATH = "./dict/idf.utf8";
const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";

// Initialize Jieba tokenizer
cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);

std::wstring tokenize_text(const std::wstring& wtext) {
    // Convert wstring to UTF-8 string
    std::string text = wstring_to_utf8(wtext);

    std::vector<std::string> words;
    jieba.Cut(text, words, true); // Use accurate mode

    // Join the words with spaces
    std::string result;
    for (const auto& word : words) {
        result += word + " ";
    }

    // Convert result back to wstring
    return utf8_to_wstring(result);
}

std::vector<std::wstring> split_paragraphs(const std::wstring& text) {
    std::vector<std::wstring> paragraphs;
    std::wstringstream ss(text);
    std::wstring line;
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            paragraphs.push_back(line);
        }
    }
    return paragraphs;
}
