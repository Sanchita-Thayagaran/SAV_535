#pragma once
#include <string>
#include <vector>

namespace Parser {
std::string trim(const std::string& s);
std::string stripComment(const std::string& s);
std::vector<std::string> tokenize(const std::string& s);
std::string upper(const std::string& s);
}
