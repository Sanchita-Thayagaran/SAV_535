#include "Parser.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace Parser {

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::string stripComment(const std::string& s) {
    const auto p1 = s.find(';');
    const auto p2 = s.find('#');
    size_t p = std::string::npos;
    if (p1 != std::string::npos) p = p1;
    if (p2 != std::string::npos) p = (p == std::string::npos ? p2 : std::min(p, p2));
    return trim(s.substr(0, p));
}

std::string upper(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return out;
}

std::vector<std::string> tokenize(const std::string& s) {
    std::string cleaned;
    cleaned.reserve(s.size());
    for (char c : s) {
        if (c == ',' || c == '(' || c == ')') cleaned.push_back(' ');
        else cleaned.push_back(c);
    }

    std::istringstream iss(cleaned);
    std::vector<std::string> toks;
    std::string tok;
    while (iss >> tok) toks.push_back(tok);
    return toks;
}

}
