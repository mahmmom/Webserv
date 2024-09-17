#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>  // For std::remove

class Server
{
    public:
        Server(int port);
        ~Server();
        void start();

    private:
        int server_fd;
        int kqueue_fd;
        std::vector<int> clients;

        void setupServerSocket(int port);
        void setupKqueue();  // New method for kqueue initialization
        void handleNewConnection();
        void handleClientData(int client_fd);
        void registerEvent(int fd, int16_t filter, uint16_t flags);
};

Server::Server(int port)
{
    // Step 1: Setup server socket first
    setupServerSocket(port);
    
    // Step 2: Initialize kqueue after socket is ready
    setupKqueue();
    
    // Step 3: Register the server socket with kqueue for incoming connections
    registerEvent(server_fd, EVFILT_READ, EV_ADD);
}

Server::~Server()
{
    close(server_fd);
    for (std::vector<int>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        close(*it);
    }
    close(kqueue_fd);
}

void Server::setupServerSocket(int port)
{
    struct sockaddr_in server_addr;
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create server socket! Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Failed to set socket options! Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Bind failed! Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0)
    {
        std::cerr << "Listen failed! Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::setupKqueue()
{
    // Initialize kqueue after socket is setup
    kqueue_fd = kqueue();
    if (kqueue_fd == -1)
    {
        std::cerr << "Error creating kqueue! Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::registerEvent(int fd, int16_t filter, uint16_t flags)
{
    struct kevent ev;
    EV_SET(&ev, fd, filter, flags, 0, 0, NULL);
    if (kevent(kqueue_fd, &ev, 1, NULL, 0, NULL) == -1)
    {
        std::cerr << "Failed to register event! Error: " << strerror(errno)
                  << " for fd: " << fd << " and kqueue_fd: " << kqueue_fd << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::start()
{
    struct kevent events[10];

    while (true)
    {
        int event_count = kevent(kqueue_fd, NULL, 0, events, 10, NULL);
        if (event_count == -1)
        {
            std::cerr << "Error in kevent! Error: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < event_count; ++i)
        {
            int event_fd = events[i].ident;
            
            if (event_fd == server_fd)
            {
                // New connection on the server socket
                handleNewConnection();
            }
            else
            {
                // Data available from a client
                handleClientData(event_fd);
            }
        }
    }
}

void Server::handleNewConnection()
{
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        std::cerr << "Failed to accept new client! Error: " << strerror(errno) << std::endl;
        return;
    }
    
    clients.push_back(client_fd);
    std::cout << "New client connected: " << client_fd << std::endl;

    // Register the client socket for monitoring incoming data
    registerEvent(client_fd, EVFILT_READ, EV_ADD);
}

void Server::handleClientData(int client_fd)
{
    char buffer[1024];
    int bytes_read = read(client_fd, buffer, sizeof(buffer));
    
    if (bytes_read <= 0)
    {
        // Client disconnected or error
        close(client_fd);
        clients.erase(std::remove(clients.begin(), clients.end(), client_fd), clients.end());
        std::cout << "Client disconnected: " << client_fd << std::endl;
    }
    else
    {
        buffer[bytes_read] = '\0';  // Null-terminate the buffer to safely print it
        std::cout << "Received from client " << client_fd << ": " << buffer << std::endl;
        // You can handle the data further or respond to the client here
    }
}

int main()
{
    Server server(8080);
    server.start();
    return 0;
}
