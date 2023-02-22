#ifndef SOCKET_H
#define SOCKET_H
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int build_listener(const char * port);
int build_sender(const char * hostname, const char * port);
#endif