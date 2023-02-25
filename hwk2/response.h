#ifndef RESPONSE_H
#define RESPONSE_H
#include <unordered_map>
class response {
public:
  std::unordered_map<std::string, std::string> headers;
  std::string msg;
  std::unordered_map<std::string, std::string> getHeaders() const {
    return headers;
  }
  std::string getMsg() { return msg; }
};
#endif
