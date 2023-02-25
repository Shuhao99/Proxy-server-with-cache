#ifndef REQUEST_H
#define REQUEST_H
#include <string>

class request {
public:
  std::string host;
  std::string port;
  std::string method;
  std::string msg;
  request(std::string msg);
};

#endif
