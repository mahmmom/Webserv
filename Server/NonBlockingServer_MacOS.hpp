// NonBlockingServer_MacOS.hpp

#ifndef NONBLOCKINGSERVER_MACOS_HPP
#define NONBLOCKINGSERVER_MACOS_HPP

#include <map>
#include <vector>
#include "Client.hpp"
#include <sys/event.h>

#define PORT 9000
#define BACKLOG 10
#define MAXDATASIZE 1024
#define MAX_EVENTS 10

class NonBlockingServer_MacOS {
private:
    static int count;
    int server_socket;
    int opt;
    int kq;
    std::vector<struct kevent> changelist;
    std::vector<struct kevent> eventlist;
    std::map<int, Client> clients;
    std::vector<int> to_remove;

    void setNonBlocking(int socket);
    void acceptNewClient();
    void handleClientRead(int client_socket);
    void handleClientWrite(int client_socket);
    void removeDisconnectedClients();

public:
    NonBlockingServer_MacOS(int port);
    void run();
    ~NonBlockingServer_MacOS();
};

#endif
