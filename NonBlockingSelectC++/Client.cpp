#include "Client.hpp"

Client::Client(int sock) : socket(sock), buffer("") {}

int Client::getSocket() const
{
    return socket;
}

std::string& Client::getBuffer()
{
    return buffer;
}
