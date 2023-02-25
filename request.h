#ifndef REQUEST_H
#define REQUEST_H
#include <string>
#include "parser_util.h"

class request
{
private:
    std::string host;
    std::string port;
    std::string method;
    std::string msg;
    std::map<std::string, std::string> header;

public:
    request(std::string msg);
    std::string get_host() { return this->host; }
    std::string get_port() { return this->port; }
    std::string get_method() { return this->method; }
    std::string get_msg() const { return this->msg; }
    std::map<std::string, std::string> get_header() { return this->header; }
    std::string get_fist_line() { return get_first_line(this->msg); }
};
// std::istringstream iss(request);
// std::string method, url, version;
// iss >> method >> url >> version;

// std::string method, url, version;
// std::vector<std::string> lines = split_string(request, '\n');
// std::vector<std::string> tokens = split_string(lines[0], ' ');
// if (tokens.size() >= 3) {
//     method = tokens[0];
//     url = tokens[1];
//     version = tokens[2];
// }

// if (method != "GET") {
//     error("Only GET method is supported");
// }

// std::vector<std::string> url_tokens = split_string(url, '/');
// if (url_tokens.size() < 2) {
//     error("Invalid URL");
// }

// std::string hostname = url_tokens[0];
// std::string path = url.substr(hostname.length() + 1)

#endif
