#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
    private:
        int socket;
        std::string buffer;
    public:
        Client(int sock);
        int getSocket() const;
        std::string& getBuffer();
};

#endif // CLIENT_HPP