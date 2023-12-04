#include "parser_util.h"

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

std::string get_body(const std::string& httpResponse) {
    std::string httpBody = "";
    std::string delimiter = "\r\n\r\n";
    size_t pos = httpResponse.find(delimiter); 

    if (pos != std::string::npos) {
        httpBody = httpResponse.substr(pos + delimiter.length());
    }

    return httpBody;
}

std::string get_header_msg(const std::string& httpResponse){
    std::string httphead = "";
    std::string delimiter = "\r\n\r\n";
    size_t pos = httpResponse.find(delimiter); 

    if (pos != std::string::npos) {
        httphead = httpResponse.substr(0, pos + delimiter.length());
    }

    return httphead;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    return str.substr(start, end - start + 1);
}

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

std::string de_chunked(const std::string& chunked_data){
    std::string decoded_data;
    std::size_t i = 0;
    while (i < chunked_data.size()) {
        // 解析当前 chunk 的大小
        std::size_t chunk_size_end = chunked_data.find("\r\n", i);
        std::string chunk_size_str = chunked_data.substr(i, chunk_size_end - i);
        std::size_t chunk_size = std::stoul(chunk_size_str, nullptr, 16);
        i = chunk_size_end + 2;
        
        // 如果 chunk 的大小为 0，则表示所有数据都已经处理完毕
        if (chunk_size == 0) {
            break;
        }
        
        // 将 chunk 的数据复制到 decoded_data 中
        decoded_data.append(chunked_data.substr(i, chunk_size));
        i += chunk_size + 2;
    }
    return decoded_data;
}