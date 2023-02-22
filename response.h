#ifndef RESPONSE_H
#define RESPONSE_H
#include "parser_util.h"

class response
{
private:
    map<std::string, std::string> header;
    int length; // >0 for length, -1 for chunk msg
    std::string msg;
    bool broken;

public:
    response(std::string msg);
    void msg_push_back(std::string &add);
    int get_length() { return this->length; }
    map<std::string, std::string> get_header() { return this->header; }
    std::string get_msg() { return this->msg; }
    bool get_brok_flag() { return this->broken; }
};
#endif