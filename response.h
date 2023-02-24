#ifndef RESPONSE_H
#define RESPONSE_H
#include <climits>
#include "parser_util.h"

class response
{
private:
    std::map<std::string, std::string> header;
    int length; // >0 for length, -1 for chunk msg
    std::string msg;
    bool miss_length;

public:
    response(std::string msg);
    void msg_push_back(std::string &add);
    int get_length() { return this->length; }
    std::map<std::string, std::string> get_header() { return this->header; }
    std::string get_msg() { return this->msg; }
    std::string get_fist_line() { return get_first_line(this->msg); }
};
#endif