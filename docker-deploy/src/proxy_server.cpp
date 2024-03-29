#include "proxy_server.h"

Cache proxy_server::cache = Cache();
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
std::ofstream log_file("/var/log/erss/proxy.log");

void proxy_server::run(){
    this->connection_lisn_fd = build_listener(lisn_port);
    if (this->connection_lisn_fd == -1)
    {
        pthread_mutex_lock(&mutex);
        log_file << "ERROR proxy server initialize failed." << std::endl;
        pthread_mutex_unlock(&mutex);
    }

    while (true)
    {
        std::string ip;
        int session_fd = create_session(connection_lisn_fd, &ip);
        if (session_fd < 0)
        {
            pthread_mutex_lock(&mutex);
            log_file << "ERROR proxy server connect failed." << std::endl;
            pthread_mutex_unlock(&mutex);
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
    if (req_len < 0)
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id <<": ERROR Get request failed." << std::endl;
        pthread_mutex_unlock(&mutex);
        close(require_fd);
        return NULL;
    }

    if (req_len == 0)
    {
        close(require_fd);
        return NULL;
    }
    
    //Chunck the buffer to actual size
    std::string request_msg = std::string(buffer, req_len);
    
    //Generate request object
    request* req = new request(request_msg);
    if (req->get_host() == "")
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id <<
        ": WARNING Request format wrong, no request has been processed." 
        << std::endl;
        pthread_mutex_unlock(&mutex);
        send_400(require_fd, curr_session);
        return NULL;
    }

    //BlackList
    // Read the blacklist from the file
    std::vector<std::string> blacklist = read_blacklist("/var/log/erss/blacklist.txt");
    // Check if the request contains any blacklisted URLs
    if (is_blacklisted(blacklist, req->get_host())) {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id <<" " << curr_session->ip << "'s request to \"" << req->get_host() << "\"" << " is blocked." << std::endl;
        pthread_mutex_unlock(&mutex);
        send_404(require_fd, curr_session);
        return NULL;
    }
    
    //get curr time string
    time_t rawtime;
    struct tm * timeinfo;
		
    std::time_t current_time = std::time(nullptr);
    std::tm* utc_time = std::gmtime(&current_time);
    const char* curr_time =  std::asctime(utc_time);

    //print to log
    pthread_mutex_lock(&mutex);
    log_file << curr_session->id <<" " << curr_session->ip << " visits: \"" << req->get_host() << "\"" << " at " << curr_time;
    pthread_mutex_unlock(&mutex);

    //Connect to remote server.
    int remote_fd = build_sender(req->get_host().c_str(), req->get_port().c_str());
    if (remote_fd < 0)
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id <<
        ": ERROR Can not connect to remote server, check request format." 
        << std::endl;
        pthread_mutex_unlock(&mutex);
        send_400(require_fd, curr_session);
        return NULL;
    }

    //TODO: record request in log file
    if (req->get_method() == "GET")
    {
        /* check cache */
        //check cache -> cache_response
        cacheResponse cache_response = cache.getResponseFromCache(*req);
        
        //Find response from cache 
        if (!cache_response.isFind)
        {
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": not in cache" << std::endl;
            pthread_mutex_unlock(&mutex);
            response * resp = get_response(remote_fd, request_msg, req, curr_session);
        
            if (resp->get_fist_line().find("200 OK") == std::string::npos)
            {
                send(require_fd, resp->get_msg().c_str(), resp->get_msg().length() + 1, 0);
                pthread_mutex_lock(&mutex);
                log_file << curr_session->id <<
                ": Respond " << "\"" << resp->get_fist_line() << "\""
                << " to " << curr_session->ip << std::endl;
                pthread_mutex_unlock(&mutex);
                delete resp;
                return NULL;
            }
            
            if (resp == NULL)
            {
                pthread_mutex_lock(&mutex);
                log_file << curr_session->id <<
                ": ERROR Can not get response from remote server." 
                << std::endl;
                pthread_mutex_unlock(&mutex);
                delete resp;
                return NULL;
            }
            
            //if chunked
            if (
                resp->get_header().count("Transfer-Encoding") 
                && resp->get_header()["Transfer-Encoding"] == "chunked"
            ){
                forward_chunked_data(remote_fd, require_fd, resp, curr_session);
            }
            else{
                //send to client
                send(require_fd, resp->get_msg().c_str(), resp->get_msg().length() + 1, 0);
            }
            
            if(resp->get_header().count("Cache-Control") || req->get_header().count("Cache-Control"))
            {
                if (
                    resp->get_header()["Cache-Control"].find("no-store") != std::string::npos
                    || req->get_header()["Cache-Control"].find("no-store") != std::string::npos
                )
                {
                    pthread_mutex_lock(&mutex);
                    log_file << curr_session->id << ": not cacheable because it said no-store" << std::endl;
                    pthread_mutex_unlock(&mutex);
                }
                else if (resp->get_header()["Cache-Control"].find("private") != std::string::npos)
                {
                    pthread_mutex_lock(&mutex);
                    log_file << curr_session->id << ": not cacheable because it is private" << std::endl;
                    pthread_mutex_unlock(&mutex);
                }
                else{
                    update_cache(curr_session, req, resp);
                }
            }
            else if(
                !resp->get_header().count("ETag")
                && !resp->get_header().count("Last-Modified")
            ){
                pthread_mutex_lock(&mutex);
                log_file << curr_session->id << ": not cacheable because don't have ETag nor Last-Modified" << std::endl;
                pthread_mutex_unlock(&mutex);
            }
            else{
                update_cache(curr_session, req, resp);
            }
            
            
            //print to log
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": Respond \"" << resp->get_fist_line() << " to " << curr_session->ip << std::endl;
            pthread_mutex_unlock(&mutex);
            
                
                
            //print to log
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": Respond \"" << resp->get_fist_line() << " to " << curr_session->ip << std::endl;
            pthread_mutex_unlock(&mutex);
            
                
            delete resp;
        }
        
        if (cache_response.isFind && !cache_response.isFresh){
            //re-validate
            response * validate_resp = validate(remote_fd, &cache_response.res, req, request_msg, curr_session);
            // is valid send back
            if (validate_resp->get_fist_line().find("200") != std::string::npos)
            {
                update_cache(curr_session, req, validate_resp);

                send(require_fd, validate_resp->get_msg().c_str(), validate_resp->get_msg().length() + 1, 0);

                pthread_mutex_lock(&mutex);
                log_file << curr_session->id << ": Respond \"" << validate_resp->get_fist_line() << " to " << curr_session->ip << std::endl;
                pthread_mutex_unlock(&mutex);
            }
            else if (validate_resp->get_fist_line().find("304") != std::string::npos){
                
                send(require_fd, cache_response.res.get_msg().c_str(), cache_response.res.get_msg().length() + 1, 0);
                
                pthread_mutex_lock(&mutex);
                log_file << curr_session->id << ": Respond \"" 
                << cache_response.res.get_fist_line() <<  " to " << curr_session->ip << std::endl;
                pthread_mutex_unlock(&mutex);

            }
            else{
                std::cout<<"Error when revalidate.";
            }
            delete validate_resp;
        }

        if (cache_response.isFind && cache_response.isFresh){
            // foward back
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": in cache, valid" << std::endl;
            pthread_mutex_unlock(&mutex);

            send(require_fd, cache_response.res.get_msg().c_str(), cache_response.res.get_msg().length() + 1, 0);
                
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": Respond \"" << cache_response.res.get_fist_line() << " to " << curr_session->ip << std::endl;
            pthread_mutex_unlock(&mutex);
        } 
    }
    
    else if (req->get_method() == "CONNECT")
    {    
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id << ": "
                << "Connecting to " << req->get_host() << std::endl;
        pthread_mutex_unlock(&mutex);
        
        if(make_connection(require_fd, remote_fd, curr_session)){
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": "
                    << "ERROR connection failed." << std::endl;
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
    }
    
    else if(req->get_method() == "POST"){            
        if (req->get_header().count("Content-Length"))
        {
            //send(remote_fd, req->get_msg().c_str(), req->get_msg().length() + 1, 0);
            response * resp = get_response(remote_fd, req->get_msg(), req, curr_session);
            
            if (resp == NULL)
            {
                pthread_mutex_lock(&mutex);
                log_file << curr_session->id <<
                ": ERROR Met trouble when submitting your post." 
                << std::endl;
                pthread_mutex_unlock(&mutex);
                delete resp;
                return NULL;
            }
            //send back response
            send(require_fd, resp->get_msg().c_str(), resp->get_msg().length() + 1, 0);
            
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": Respond " << "\""
                     << resp->get_fist_line() << "\"" << " to " << curr_session->ip << std::endl;
            pthread_mutex_unlock(&mutex);
            
            delete resp;
        }
        else{
            pthread_mutex_lock(&mutex);
            log_file << curr_session->id << ": "
                    << "WARNING receive a blank POST request." << std::endl;
            pthread_mutex_unlock(&mutex);
        }  
    }
    
    else {
        // Can only handle GET POST CONNECT
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id <<
        ": ERROR Can only handle GET POST CONNECT." 
        << std::endl;
        pthread_mutex_unlock(&mutex);
        send_400(require_fd, curr_session);
    }
    
    close(remote_fd);
    close(require_fd);
    delete req;
    return NULL;
}

void proxy_server::update_cache(
    const session * curr_session,
    request * req,
    response * resp
)
{
    pthread_mutex_lock(&mutex);
    cache.updateCache(*req, *resp);
    pthread_mutex_unlock(&mutex);

    std::string expire_time = cache.getExpires(*req);
    if (expire_time.empty())
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id << 
        ": cached, but requires re-validation" << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id << ": cached, expires at " 
        << expire_time <<std::endl;
        pthread_mutex_unlock(&mutex);
    }
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
    const std::string &request_msg,
    request * req,
    const session* curr_session)
{
    //Forward request to remote server
    send(remote_fd, request_msg.c_str(), request_msg.length() + 1, 0);
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

    int actual_len = resp->get_length();
    //get the whole msg package
    if (
        !resp->get_header().count("Transfer-Encoding") ||
        ( resp->get_header().count("Transfer-Encoding") && resp->get_header()["Transfer-Encoding"] != "chunked")
    ){
        while ( (resp->get_body_len() + 1) < actual_len && rec_len > 0)
        {
            rec_len = recv(remote_fd, buffer, sizeof(buffer), 0);
            if (rec_len > 0){
                std::string temp(buffer, rec_len);
                resp->msg_push_back(temp);
            }
        }
    }
    return resp;
}

void proxy_server::forward_chunked_data(
    const int &remote_fd,
    const int &client_fd,
    response *resp,
    const session * curr_session)
{
    std::string first_pkg = resp->get_msg();
    send(client_fd, first_pkg.c_str(), first_pkg.length(), 0);
    
    char buffer[MAX_BUFFER_SIZE] = {0};
    while (true)
    {
        
        int len = recv(remote_fd, buffer, sizeof(buffer), 0);
        if(len <= 0){
            break;
        }

        std::string resp_msg(buffer, len);
        std::size_t chunk_size = std::stoul(buffer, nullptr, 16);
        if (chunk_size == 0) {
            break;
        }
        
        int send_len = send(client_fd, buffer, len, 0);
        if (send_len <= 0) {
            break;
        }
        
        resp->msg_push_back(resp_msg);
    }
    resp->set_len(resp->get_msg().length() + 1); 
}

int proxy_server::make_connection(
    const int &client_fd, 
    const int &server_fd, 
    const session* curr_session
)
{
    // if error return 1 else return 0
    std::string ok = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, ok.c_str(), ok.length() + 1, 0);
    pthread_mutex_lock(&mutex);
    log_file << curr_session->id << ": Respond \"HTTP/1.1 200 OK\"" << " to " << curr_session->ip << std::endl;
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

response* proxy_server::validate(
    const int & remote_fd, 
    response * resp, 
    request *req,
    const std::string &request_,
    const session* curr_session
)
{
    std::string expire_time = cache.getExpires(*req);
    if (expire_time.empty())
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id << 
        ": in cache, requires validation" << std::endl;
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        pthread_mutex_lock(&mutex);
        log_file << curr_session->id << ": in cache, but expired at " 
        << expire_time <<std::endl;
        pthread_mutex_unlock(&mutex);
    }

    size_t pos = request_.find_last_not_of(" \t\r\n");
    std::string new_req_msg = request_.substr(0, pos+1);
    
    if (resp->get_header().count("ETag") && new_req_msg.find("ETag") == std::string::npos) {
        new_req_msg = new_req_msg + "\r\nIf-None-Match: " + resp->get_header()["ETag"];
    }
    
    if (resp->get_header().count("Last-Modified") && new_req_msg.find("Last-Modified") == std::string::npos) {
        new_req_msg = new_req_msg + "\r\nIf-Modified-Since: " + resp->get_header()["Last-Modified"];
    }
    new_req_msg = new_req_msg + "\r\n\r\n";
    //get response frome remote server
    request* new_req = new request(new_req_msg);

    response* new_resp = get_response(remote_fd, new_req_msg, new_req, curr_session);

    delete new_req;
    return new_resp;
}

// Function to read URLs from blacklist.txt and store them in a vector
std::vector<std::string> proxy_server::read_blacklist(const std::string& filename) {
    std::vector<std::string> blacklist;
    std::ifstream file;
    std::string line;

    try {
        file.open(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        while (getline(file, line)) {
            blacklist.push_back(line);
        }
        file.close();
    } catch (const std::ifstream::failure& e) {
        log_file << "File I/O error occurred: " << e.what();
    } catch (const std::exception& e) {
        log_file << "Error: " << e.what();
    }

    return blacklist;
}

// Function to check if the string 'req' contains any of the blacklisted URLs
bool proxy_server::is_blacklisted(const std::vector<std::string>& blacklist, const std::string& req) {
    for (const auto& url : blacklist) {
        if (req.find(url) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// When error send 502 bad gateway
void proxy_server::send_502(const int & client_fd, session * curr_session)
{
    const char * bad502 = "HTTP/1.1 502 Bad Gateway\r\n Connection: close\r\n";
    send(client_fd, bad502, sizeof(bad502), 0);
    pthread_mutex_lock(&mutex);
    log_file << curr_session->id << ": Respond \"HTTP/1.1 502 Bad Gateway\"" << " to " << curr_session->ip << std::endl;
    pthread_mutex_unlock(&mutex);
}

// When receive malformed request send 400 bad request
void proxy_server::send_400(const int & client_fd, session * curr_session)
{
    const char * bad400 = "HTTP/1.1 400 Bad Request\r\n Connection: close\r\n";
    send(client_fd, bad400, sizeof(bad400), 0);
    pthread_mutex_lock(&mutex);
    log_file << curr_session->id << ": Responding \"HTTP/1.1 400 Bad Request\"" << " to " << curr_session->ip << std::endl;
    pthread_mutex_unlock(&mutex);
}

// When request has been blocked
void proxy_server::send_404(const int & client_fd, session * curr_session)
{
    const char * bad404 = "HTTP/1.1 400 Not Found\r\n Connection: close\r\n";
    send(client_fd, bad404, sizeof(bad404), 0);
    pthread_mutex_lock(&mutex);
    log_file << curr_session->id << ": Responding \"HTTP/1.1 404 Not Found\"" << " to " << curr_session->ip << std::endl;
    pthread_mutex_unlock(&mutex);
}
