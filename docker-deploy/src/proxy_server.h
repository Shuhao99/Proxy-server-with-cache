#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <pthread.h>
#include <time.h> 
#include <iostream>
#include <sstream>

#include <fstream>
#include <map>
#include <unordered_map>
#include <vector> 

#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h> 

#include "request.h"
#include "response.h"
#include "session.h"
#include "socket.h"
#include "parser_util.h"
#include "cache.hpp"

class proxy_server {
private:
    static const int MAX_BUFFER_SIZE = 65536;
    static const int BUFFER_SIZE = 4096;
    static Cache cache;
    const char * lisn_port;
    int connection_lisn_fd;
    int id_counter;
    std::vector<session> session_queue;

public:
    proxy_server(const char * port) : 
        lisn_port(port), 
        connection_lisn_fd(-1), 
        id_counter(1){}
    //~proxy_server() //delete cache
    
    void run();
    int create_session(const int &listener_fd, std::string * ip);
    
    static void * handle(void* info);
    
    static void send_502(const int & client_fd, session * curr_session);
    static void send_400(const int & client_fd, session * curr_session);

    static void update_cache(
        const session * curr_session,
        request * req,
        response * resp);
    
    static response* get_response(
        const int &remote_fd, 
        const std::string &request_msg,
        request * req,
        const session* curr_session);
    
    static void forward_chunked_data(
        const int &remote_fd,
        const int &client_fd,
        response *resp,
        const session * curr_session);

    static int make_connection(
        const int &client_fd, 
        const int &server_fd, 
        const session* curr_session);
    
    static response* validate(
        const int & remote_fd, 
        response * resp, 
        request *req,
        const std::string &input,
        const session* curr_session);
};

#endif

