#include "request.h"

request(std::string msg){
    std::map<std::string, std::string> info = parse_headers(msg);
    
    //get method
    std::vector<std::string> first_line_tokens = split(get_first_line(msg), " ");
    this->method = first_line_tokens[0];

    //get host info
    if (!info.count("Host"))
    {
        this->host = "";
        this->port = "";
        this->method = "";
        return;
    }
    std::string host_line = info["Host"]
    if (host_line.find(":") == std::string::npos) {
        this->host = host_line;
        //default prot 80 for http/https
        this->port = "80";
        return;
    }
    //get port number
    std::vector<std::string> host_info = split(host_line, ":");
    this->host = host_info[0];
    this->port = host_info[1];
    return;
}