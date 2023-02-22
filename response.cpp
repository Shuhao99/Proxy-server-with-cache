#include "response.h"

response::response(std::string msg){
    this->broken = false;
    this->header = parse_headers(msg);
    this->msg = msg;
    //int length; >0 for actual length, -1 for chunk msg
    if (this->header.count("Content-Length"))
    {
        try
        {
            this->length = std::stoi(header["Content-Length"]);
        }
        catch(const std::exception& e)
        {
            this->broken = true;
            std::cerr << "ERROR the http response is broken" << '\n';
        } 
    }
    else{
        this->length = -1;
    }
    return;
}

void response::msg_push_back(std::string &add){
    this->msg = msg + add;
}