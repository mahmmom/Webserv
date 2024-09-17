#include "Server.hpp"

Server::Server(const std::string& configFile)
{
    // Read configuration file
    std::ifstream file(configFile);
    if (!file.is_open())
    {
        std::cerr << "Error: Unable to open configuration file: " << configFile << std::endl;
        exit(EXIT_FAILURE);
    }
}

Server::~Server()
{
    close(kqueue_fd);
    for (int server_socket : server_sockets)
    {
        close(server_socket);
    }
}

void Server::run()
{
    setupSockets();
    setupKqueue();

    while (true)
    {
        struct kevent events[10];
        int nevents = kevent(kqueue_fd, NULL, 0, events, 10, NULL);
        if (nevents < 0)
        {
            std::cerr << "Error: kevent failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nevents; i++)
        {
            if (events[i].ident == server_sockets[0])
            {
                acceptConnection(server_sockets[0]);
            }
            else
            {
                handleClient(events[i].ident);
            }
        }
    }
}

void Server::setupSockets()
{
    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        std::cerr << "Error: Unable to create server socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set server socket to non-blocking
    int flags = fcntl(server_socket, F_GETFL, 0);
    if (flags < 0)
    {
        std::cerr << "Error: Unable to get server socket flags: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        std::cerr << "Error: Unable to set server socket to non-blocking: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Bind server socket
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Error: Unable to bind server socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Listen on server socket
    if (listen(server_socket, 10) < 0)
    {
        std::cerr << "Error: Unable to listen on server socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    server_sockets.push_back(server_socket);
}

void Server::setupKqueue()
{
    kqueue_fd = kqueue();
    if (kqueue_fd < 0)
    {
        std::cerr << "Error: Unable to create kqueue: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    struct kevent event;
    EV_SET(&event, server_sockets[0], EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kqueue_fd, &event, 1, NULL, 0, NULL) < 0)
    {
        std::cerr << "Error: Unable to add server socket to kqueue: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Server::acceptConnection(int server_socket)
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_socket < 0)
    {
        std::cerr << "Error: Unable to accept client connection: " << strerror(errno) << std::endl;
        return;
    }

    // Set client socket to non-blocking
    int flags = fcntl(client_socket, F_GETFL, 0);
    if (flags < 0)
    {
        std::cerr << "Error: Unable to get client socket flags: " << strerror(errno) << std::endl;
        close(client_socket);
        return;
    }
    if (fcntl(client_socket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        std::cerr << "Error: Unable to set client socket to non-blocking: " << strerror(errno) << std::endl;
        close(client_socket);
        return;
    }

    struct kevent event;
    EV_SET(&event, client_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kqueue_fd, &event, 1, NULL, 0, NULL) < 0)
    {
        std::cerr << "Error: Unable to add client socket to kqueue: " << strerror(errno) << std::endl;
        close(client_socket);
        return;
    }
}

void Server::handleClient(int client_socket)
{
    // Read client request
    char buffer[1024];
    int bytes_read = read(client_socket, buffer, sizeof(buffer));
    if (bytes_read < 0)
    {
        std::cerr << "Error: Unable to read client request: " << strerror(errno) << std::endl;
        close(client_socket);
        return;
    }

    // Handle client request
    sendResponse(client_socket);
}

void Server::sendResponse(int client_socket)
{
    const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello, World";
    int bytes_written = write(client_socket, response, strlen(response));
    if (bytes_written < 0)
    {
        std::cerr << "Error: Unable to send response to client: " << strerror(errno) << std::endl;
        close(client_socket);
        return;
    }

    close(client_socket);
}
