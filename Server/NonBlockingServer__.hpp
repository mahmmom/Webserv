#ifndef NONBLOCKINGSERVER_HPP
#define NONBLOCKINGSERVER_HPP

#include <map>
#include <vector>
#include <sys/select.h>
#include "Client.hpp"

#include <sys/socket.h>
#include <netinet/in.h>	// for using struct sockaddr_in
#include <arpa/inet.h> // for inet_pton
#include <cstdlib>
#include <cstring>
#include <cstdio> // for perror
#include <unistd.h> // for close
#include <iostream>
#include <iterator>
#include <sys/event.h>  // for kqueue
#include <sys/types.h>
#include <algorithm> // for find
#include <fcntl.h>

#define PORT 9000
#define BACKLOG 10
#define MAXDATASIZE 1024

class NonBlockingServer
{
    private:
        static int count;
        int server_socket;
        int opt;
        int kq; // check if it was implemented as part of the class
	    std::vector<struct kevent> eventlist;
        std::vector<struct kevent> changelist;
        std::vector<struct kevent> templist;;
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