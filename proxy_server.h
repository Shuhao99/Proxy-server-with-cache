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

#include "request.h"
#include "response.h"
#include "session.h"
#include "socket.h"

class proxy_server {
private:
    static const int MAX_BUFFER_SIZE = 65536;
    static const int BUFFER_SIZE = 4096;
    const char * lisn_port;
    int connection_lisn_fd;
    int id_counter;
    std::vector<session> session_queue;
    //cache
    

public:
    proxy_server(const char * port) : 
        lisn_port(port), 
        connection_lisn_fd(-1), 
        id_counter(1)
    {
        //build cache
    }
    //~proxy_server() //delete cache
    
    void run();
    
    static void * handle(void* info);
    
    int create_session(const int &listener_fd, std::string * ip);
    
    static response* get_response(
        const int &remote_fd, 
        const std::string &request_msg
    );
    
    static void forward_chunked_data(
        const int &remote_fd, 
        const int &client_fd,
        const std::string &first_pkg,
        const session * curr_session
    );
};

#endif


// const std::string HTTP_VERSION = "HTTP/1.1";

// void error(const char* msg) {
//     perror(msg);
//     exit(1);
// }

// int main(int argc, char* argv[]) {
//     if (argc != 3) {
//         std::cerr << "Usage: " << argv[0] << " [listen_port] [remote_host:remote_port]\n";
//         exit(1);
//     }

//     // 解析命令行参数
//     int listen_port = atoi(argv[1]);
//     std::string remote_host = "";
//     int remote_port = 0;
//     char* pos = strchr(argv[2], ':');
//     if (pos != nullptr) {
//         remote_host = std::string(argv[2], pos - argv[2]);
//         remote_port = atoi(pos + 1);
//     }

//     if (listen_port <= 0 || remote_host.empty() || remote_port <= 0) {
//         std::cerr << "Invalid arguments\n";
//         exit(1);
//     }

//     // 创建监听套接字
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) {
//         error("ERROR opening socket");
//     }

//     // 绑定地址和端口
//     sockaddr_in serv_addr;
//     memset((char*)&serv_addr, 0, sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr.s_addr = INADDR_ANY;
//     serv_addr.sin_port = htons(listen_port);
//     if (bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
//         error("ERROR on binding");
//     }

//     // 监听连接
//     listen(sockfd, 5);

//     std::cout << "Proxy server is listening on port " << listen_port << std::endl;

//     while (true) {
//         // 接收客户端连接
//         sockaddr_in cli_addr;
//         socklen_t clilen = sizeof(cli_addr);
//         int newsockfd = accept(sockfd, (sockaddr*)&cli_addr, &clilen);
//         if (newsockfd < 0) {
//             error("ERROR on accept");
//         }

//         std::cout << "Accepted a new connection from " << inet_ntoa(cli_addr.sin_addr) << ":" << ntohs(cli_addr.sin_port) << std::endl;

//         // 接收客户端的请求
//         char buf[MAX_BUFFER_SIZE];
//         int n = recv(newsockfd, buf, sizeof(buf) - 1, 0);
//         if (n < 0) {
//             error("ERROR reading from socket");
//         }

//         std::string request(buf, n);
//         std::cout << "Received request:\n" << request << std::endl;

//         // 解析客户端请求
//         std::istringstream iss(request);
//         std::string method, url, version;
//         iss >> method >> url >> version;

//         // 构造远程服务器的地址
//         addrinfo hints, *servinfo;
//         memset(&hints, 0, sizeof(hints));
//         hints.ai_family = AF_UNSPEC;
//         hints.ai_socktype = SOCK_STREAM;
//         if (getaddrinfo(remote_host.c_str(), std::to_string(remote_port).c_str(), &hints, &servinfo) != 0){
//             error("ERROR getting address info");
//         }

//         // 连接远程服务器
//         int remote_sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
//         if (remote_sockfd < 0) {
//             error("ERROR opening socket");
//         }
//         if (connect(remote_sockfd, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
//             error("ERROR connecting");
//         }

//         std::cout << "Connected to remote server " << remote_host << ":" << remote_port << std::endl;

//         // 发送客户端请求到远程服务器
//         if (send(remote_sockfd, request.c_str(), request.size(), 0) < 0) {
//             error("ERROR writing to socket");
//         }

//         // 接收远程服务器的响应
//         std::string response = "";
//         while (true) {
//             n = recv(remote_sockfd, buf, sizeof(buf) - 1, 0);
//             if (n < 0) {
//                 error("ERROR reading from socket");
//             }
//             if (n == 0) {
//                 break;
//             }

//             // 处理 chunked 响应
//             std::string chunked_response(buf, n);
//             size_t pos = 0;
//             while (true) {
//                 size_t len_pos = chunked_response.find("\r\n", pos);
//                 if (len_pos == std::string::npos) {
//                     break;
//                 }
//                 int chunk_len = strtol(chunked_response.substr(pos, len_pos - pos).c_str(), nullptr, 16);
//                 if (chunk_len == 0) {
//                     // 最后一个 chunk
//                     response += chunked_response.substr(len_pos + 2);
//                     break;
//                 } else {
//                     response += chunked_response.substr(len_pos + 2, chunk_len);
//                     pos = len_pos + 2 + chunk_len + 2;
//                 }
//             }
//         }

//         std::cout << "Received response:\n" << response << std::endl;

//         // 把远程服务器的响应发送给客户端
//         if (send(newsockfd, response.c_str(), response.size(), 0) < 0) {
//             error("ERROR writing to socket");
//         }

//         close(newsockfd);
//         close(remote_sockfd);
//         freeaddrinfo(servinfo);

//         std::cout << "Connection closed\n";
//     }

//     close(sockfd);
//     return 0;
// }
