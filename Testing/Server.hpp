#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

class Server
{
public:
    Server(const std::string& configFile);
    ~Server();
    void run();

private:
    int kqueue_fd;
    std::vector<int> server_sockets;

    void setupSockets();  // Setup and bind server sockets
    void setupKqueue();   // Initialize kqueue
    void acceptConnection(int server_socket);  // Accept new client connection
    void handleClient(int client_socket);      // Handle client request
    void sendResponse(int client_socket);      // Send response to client
};

#endif