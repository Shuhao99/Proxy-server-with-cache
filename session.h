#ifndef SESSION_H
#define SESSION_H
#include <cstdlib>
#include <string>

struct session
{
  int id;
  int fd;
  std::string ip;
};

#endif

// class client {
//  private:
//   int id;
//   int fd;
//   const char * ip;

//  public:
  
//   client(const char * ip_) : ip(ip) {}
//   const char * get_ip() { return ip; }
  
//   void set_fd(int fd_) { fd = fd_; }
//   int get_fd() { return fd; }

//   void set_id(int id_) { id = id_; }
//   int get_id() { return id; }

// };