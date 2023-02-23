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
    session * curr_session = (session*)curr_session_;
    int require_fd = curr_session->fd;
    char buffer[MAX_BUFFER_SIZE];
    
    //Receive request from client.
    int req_len = recv(require_fd, & buffer, sizeof(buffer), 0);
    if (req_len <= 0)
    {
        pthread_mutex_lock(&mutex);
        log_file <<"("<< curr_session->id << ")"<<": ERROR Get request failed." << std::endl;
        pthread_mutex_lock(&mutex);
        return NULL;
    }
    
    //Chunck the buffer to actual size
    std::string request_msg = std::string(buffer, req_len);
    
    //Generate request object
    request* req = new request(request_msg);
    if (req->get_host() == "")
    {
        pthread_mutex_lock(&mutex);
        log_file <<"("<< curr_session->id << ")"<<
        ": WARNING Request format wrong, no request has been processed." 
        << std::endl;
        pthread_mutex_lock(&mutex);
        return NULL;
    }
    
    //Connect to remote server.
    int remote_fd = build_sender(req->get_host().c_str(), req->get_port().c_str());
    if (remote_fd < 0)
    {
        pthread_mutex_lock(&mutex);
        log_file <<"("<< curr_session->id << ")"<<
        ": ERROR Can not connect to remote server, check request format." 
        << std::endl;
        pthread_mutex_lock(&mutex);
        return NULL;
    }

    //TODO: record request in log file
    if (req->get_method() == "GET")
    {
        /* check cache */
        //check cache -> cache_response
            //if !is_find
                // get response

                /*---------- get response start ----------*/
        response * resp = get_response(remote_fd, request_msg);
        if (resp == NULL)
        {
            pthread_mutex_lock(&mutex);
            log_file <<"("<< curr_session->id << ")"<<
            ": ERROR Get response from remote server failed." 
            << std::endl;
            pthread_mutex_lock(&mutex);
            return NULL;
        }
        
        //if chunked
        if (
            resp->get_header().count("Transfer-Encoding") 
            && resp->get_header()["Transfer-Encoding"] == "chunked"
        )
        {
            forward_chunked_data(remote_fd, require_fd, resp->get_msg(), curr_session);
        }

        // if not chunked
        else if (resp->get_header().count("Content-Length")) 
        {   
            // update cache
            send(require_fd, resp->get_msg().c_str(), resp->get_length(), 0);
            
        }
                /*---------- get response end ----------*/

            //if is_find && !is_fresh 
                // revalidate
            //if is_find && is_fresh 
                // foward back
                
        // record in log bla bla 
        delete resp;
    }
    
    else if (req->get_method() == "CONNECT")
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id << ": "
                << "Requesting \"" << req->get_fist_line() << "\" from " << req->get_host() << std::endl;
        pthread_mutex_unlock(&mutex);
        
        if(make_connection(require_fd, remote_fd, curr_session)){
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": "
                    << "ERROR connection failed." << std::endl;
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id << ": Tunnel closed" << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    
    else if(req->get_method() == "POST"){

    }
    close(remote_fd);
    close(require_fd);
    delete req;
    return NULL;
}

int proxy_server::create_session(const int &listener_fd, std::string * ip){
    struct sockaddr_storage socket_addr;

    socklen_t socket_addr_len = sizeof(socket_addr);

    int client_connect_fd = accept(listener_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    
    struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
    
    *ip = inet_ntoa(addr->sin_addr);
    return client_connect_fd;
}

/*
    return the full response if the resp is not chunked
    return the first part of the msg if the resp is chunked
*/
response* proxy_server::get_response(
    const int &remote_fd, 
    const std::string &request_msg)
{
    //Forward request to remote server
    send(remote_fd, request_msg.c_str(), request_msg.length(), 0);
    //receive first part of message
    char buffer[BUFFER_SIZE];
    int rec_len = recv(remote_fd, buffer, sizeof(buffer), 0);
    if (rec_len <= 0)
    {
        return NULL;
    }
    //convert response messge to string
    std::string resp_msg(buffer, rec_len);
    //parse and generate response
    response* resp = new response(resp_msg);
    if(resp->get_brok_flag()){
        return NULL;
    }

    int actual_len = resp->get_length();
    //get the whole msg package
    while (resp->get_msg().length() < actual_len && rec_len > 0)
    {
        rec_len = recv(remote_fd, buffer, sizeof(buffer), 0);
        if (rec_len > 0){
            std::string temp(buffer, rec_len);
            resp->msg_push_back(temp);
        }
    }
    return resp;
}

void proxy_server::forward_chunked_data(
    const int &remote_fd,
    const int &client_fd,
    const std::string &first_pkg,
    const session * curr_session)
{
    //TODO: record in log data
    pthread_mutex_lock(&mutex);
    log_file << curr_session->id << ": not cacheable because it is chunked" << std::endl;
    pthread_mutex_unlock(&mutex);
    //send first package
    send(remote_fd, first_pkg.c_str(), first_pkg.length(), 0);
    char buffer[BUFFER_SIZE];
    int len = 1;
    while (len)
    {
        len = recv(remote_fd, buffer, sizeof(buffer), 0);
        if(len > 0){
            send(client_fd, buffer, len, 0);
        }
    }
    
}

int proxy_server::make_connection(
    const int &client_fd, 
    const int &server_fd, 
    session* curr_session)
{
    // if error return 1 else return 0
    std::string ok = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, ok.c_str(), ok.length() + 1, 0);
    pthread_mutex_lock(&mutex);
    log_file << curr_session->id << ": Responding \"HTTP/1.1 200 OK\"" << std::endl;
    pthread_mutex_unlock(&mutex);

    fd_set read_fds;
    int nfds = std::max(server_fd, client_fd);
    int nread = 0;
    char buffer[MAX_BUFFER_SIZE];

    while (1) {
        FD_ZERO(&read_fds);
        // add the fds in read set
        FD_SET(server_fd, &read_fds);
        FD_SET(client_fd, &read_fds);
        // check who is ready to read
        if(select(nfds + 1, &read_fds, NULL, NULL, NULL) == -1){
            return 1;
        }

        if (FD_ISSET(client_fd, &read_fds)) {
            nread = recv(client_fd, buffer, sizeof(buffer), 0);
            if (nread <= 0) {
                break;
            }
            send(server_fd, buffer, nread, 0);
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            nread = recv(server_fd, buffer, sizeof(buffer), 0);
            if (nread <= 0) {
                break;
            }
            send(client_fd, buffer, nread, 0);
        } 
    }
    return 0;
}