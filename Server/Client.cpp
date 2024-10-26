#include "Client.hpp"

Client::Client(int sock) : socket(sock), buffer("") 
{
    lastRequestTime = std::time(0);
}

int Client::getSocket() const
{
    return socket;
}

std::string& Client::getBuffer()
{
    return buffer;
}

bool    Client::isTimedout(size_t& keepaliveTimeout)
{
    std::time_t now = std::time(0);
    std::time_t elapsedTime = now - lastRequestTime;

    std::cout << "now is " << now << " and elapsedTime is " << elapsedTime << std::endl;

    bool timedOut = elapsedTime > static_cast<std::time_t>(keepaliveTimeout);
    if (timedOut) {
        std::cout << "Now is " << now 
                    << " and lastRequestTime is " << lastRequestTime 
                    << " and keepalive_timeout is " << keepaliveTimeout 
                    << std::endl;
    }
    return (timedOut);
}
