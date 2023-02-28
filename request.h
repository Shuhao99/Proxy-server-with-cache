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

#endif
