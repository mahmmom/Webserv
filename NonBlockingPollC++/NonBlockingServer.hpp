#ifndef NONBLOCKINGSERVER_HPP
#define NONBLOCKINGSERVER_HPP

#include <map>
#include <vector>
#include <poll.h>
#include "Client.hpp"

class NonBlockingServer
{
    private:
        int server_socket;
        int opt;
        std::vector<struct pollfd> poll_fds;
        std::map<int, Client> clients;
        std::vector<int> to_remove;

        void setNonBlocking(int socket);
        void acceptNewClient();
        void handleClientRead(int client_socket);
        void handleClientWrite(int client_socket);
        void removeDisconnectedClients();

    public:
        NonBlockingServer(int port);
        void run();
        ~NonBlockingServer();
};

#endif
