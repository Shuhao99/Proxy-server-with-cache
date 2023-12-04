#ifndef PARSER_UTIL_H
#define PARSER_UTIL_H

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>

std::vector<std::string> split(const std::string& str, const std::string& delimiter);

std::string trim(const std::string& str);

std::string get_first_line(const std::string&message);

std::map<std::string, std::string> parse_headers(const std::string& message);

std::string get_body(const std::string& httpResponse);

std::string get_header_msg(const std::string& httpResponse);

std::string de_chunked(const std::string& chunked);
#endif
