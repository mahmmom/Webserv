#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <ctime>
#include "../HTTP/HTTPRequest.hpp"

class Client
{
    private:
        int socket;
        std::string buffer;
    public:
        Client(int sock);

        HTTPRequest request;

        std::time_t lastRequestTime;
        bool         isTimedout(size_t& keepaliveTimeout);

        int          getSocket() const;
        std::string& getBuffer();
};

#endif // CLIENT_HPP