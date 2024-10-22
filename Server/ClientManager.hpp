#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../HTTP/HTTPRequest.hpp"
#include "Server.hpp"

#define MAX_REQUEST_HEADERS_SIZE 16384 // 16 KB

#define MAX_REQUEST_BODY_SIZE 16384 // 16 KB

class ClientManager {
    private:
        int                                                  _fd;
        std::string                                         _clientIpAddr;
        int                                                 _requestCount;
        // std::chrono::time_point<std::chrono::steady_clock>  _lastRequestTime;

        HTTPRequest                                         _request;
        std::string                                         _requestHeaders;
        std::string                                         _requestBody;
        std::ofstream                                       _requestBodyFile;
        size_t                                              _requestBodySize;
        std::string                                         _requestBodyFilePath;
        bool                                                _areHeaderComplete;
        bool                                                _isBodyComplete;

    public:
        ClientManager(int fd, const std::string &clientIpAddr);
        ~ClientManager();

        void resetClientManager();
        void updateLastRequestTime();
        void incrementRequestCount();

        void processIncomingData(Server &server, const char *buffer, size_t bytesRead);
        void processHeaders(Server &server, const char *buffer, size_t bytesRead);
        void processBody(Server &server, const char *buffer, size_t bytesRead);
        void parseHeaders(Server &server);
        void initializeBodyStorage(Server &server);

        void handleGetRequest(Server &server);
        void handleHeadRequest(Server &server);
        void handlePostRequest(Server &server);

        int getFd() const;
        const std::string &getClientIpAddr() const;
        int getRequestCount() const;
        const std::string &getPostRequestFileName();
        bool isTimedOut(size_t keepalive_timeout) const;
};


#endif