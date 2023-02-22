#include "proxy_server.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream log_file("proxy.log");

void proxy_server::run(){
    this->connection_lisn_fd = build_listener(lisn_port);
    if (this->connection_lisn_fd == -1)
    {
        pthread_mutex_lock(&mutex);
        log_file << "(no id): ERROR proxy server initialize failed." << std::endl;
        pthread_mutex_lock(&mutex);
    }
    

    while (true)
    {
        std::string ip;
        int session_fd = create_session(connection_lisn_fd, &ip);
        if (session_fd < 0)
        {
            pthread_mutex_lock(&mutex);
            log_file << "(no id): ERROR proxy server connect failed." << std::endl;
            pthread_mutex_lock(&mutex);
        }
        
        session temp = {id_counter, session_fd, ip};
        this->session_queue.push_back(temp);
        id_counter++;
        pthread_t thread;
        pthread_create(&thread, NULL, handle, &(this->session_queue.back()));
    }
    
}
void * proxy_server::handle(void *curr_session_){
    curr_session * curr_session_ = (session*)curr_session_;
    int requie_fd = curr_session->fd;
    char buffer[MAX_BUFFER_SIZE];
    
    //Receive request from client.
    int req_len = recv(requie_fd, & buffer, sizeof(buffer), 0);
    if (req_len <= 0)
    {
        pthread_mutex_lock(&mutex);
        log_file <<"("<< curr_session_->id << ")"<<": ERROR Get request failed." << std::endl;
        pthread_mutex_lock(&mutex);
        return NULL;
    }
    
    //Chunck the buffer to actual size
    std::string request_msg = std::string(buffer, req_len);
    
    //Generate request object
    request* req = new request(request_msg);
    if (req->host == "")
    {
        pthread_mutex_lock(&mutex);
        log_file <<"("<< curr_session_->id << ")"<<
        ": WARNING Request format wrong, no request has been processed." 
        << std::endl;
        pthread_mutex_lock(&mutex);
        return NULL;
    }
    
    //Connect to remote server.
    remote_fd = build_sender(req->host.c_str(), req->port.c_str());
    if (remote_fd < 0)
    {
        pthread_mutex_lock(&mutex);
        log_file <<"("<< curr_session_->id << ")"<<
        ": ERROR Can not connect to remote server, check request format." 
        << std::endl;
        pthread_mutex_lock(&mutex);
    }

    //record request in log file
    if (req->method == "GET")
    {
        /* check cache */
        // record in log bla bla
        //Forward request to remote server
        send(remote_fd, request_msg, request_msg.length(), 0);
        //receive msg merge together
        //generate response object
        // if is chunk
        // else
        
    }
    else if (req->method == "CONNECT")
    {
        /* code */
    }
    else if(req->method == "POST")

    //receive response from remote server

    
    delete req;
    return NULL;
}

int proxy_server::create_session(int listener_fd, std::string * ip){
    struct sockaddr_storage socket_addr;

    socklen_t socket_addr_len = sizeof(socket_addr);

    int client_connect_fd = accept(listener_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    
    struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
    
    *ip = inet_ntoa(addr->sin_addr);
    return client_connect_fd;
}



