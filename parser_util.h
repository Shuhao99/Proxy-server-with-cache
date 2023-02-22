#include<map>
#include<string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>

std::string get_first_line(const std::string&message){
    std::vector<std::string> lines = split(message, "\r\n");
    return lines[0];
}

std::map<std::string, std::string> parse_headers(const std::string& message) {
    std::map<std::string, std::string> headers;
    std::vector<std::string> lines = split(message, "\r\n");
    for (size_t i = 1; i < lines.size(); i++) {
        std::string line = lines[i];
        size_t colon_pos = line.find(":");
        if (colon_pos == std::string::npos) {
            continue;
        }
        std::string name = trim(line.substr(0, colon_pos));
        std::string value = trim(line.substr(colon_pos + 1));
        headers[name] = value;
    }
    return headers;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t pos = 0;
    size_t last_pos = 0;
    while ((pos = str.find(delimiter, last_pos)) != std::string::npos) {
        std::string token = str.substr(last_pos, pos - last_pos);
        tokens.push_back(token);
        last_pos = pos + delimiter.size();
    }
    std::string last_token = str.substr(last_pos);
    if (!last_token.empty()) {
        tokens.push_back(last_token);
    }
    return tokens;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    return str.substr(start, end - start + 1);
}