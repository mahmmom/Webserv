#ifndef NONBLOCKINGSERVER_HPP
#define NONBLOCKINGSERVER_HPP

#include <map>
#include <vector>
#include "Client.hpp"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <fcntl.h>

#ifdef __APPLE__
    #include <sys/event.h>
#elif __linux__
    #include <sys/epoll.h>
#else
    #error "Unsupported platform"
#endif

#define PORT 9000
#define BACKLOG 10
#define MAXDATASIZE 1024
#define MAX_EVENTS 10

class NonBlockingServer {
private:
    static int count;
    int server_socket;
    int opt;
    
    #ifdef __APPLE__
        int kq;
        std::vector<struct kevent> changelist;
        std::vector<struct kevent> eventlist;
    #elif __linux__
        int epoll_fd;
        std::vector<struct epoll_event> events;
    #endif
    
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