#include "NonBlockingServer.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "Errors.hpp"

NonBlockingServer::NonBlockingServer(int port)
{
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        throw SocketException();
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    opt = 1;

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw SetsockoptException();
    }

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        throw BindException();
    }

    if (listen(server_socket, 5) < 0)
    {
        throw ListenException();
    }

    setNonBlocking(server_socket);

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&except_fds);
    FD_SET(server_socket, &read_fds);
}

void NonBlockingServer::setNonBlocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

void NonBlockingServer::acceptNewClient()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

    if (client_socket >= 0)
    {
        setNonBlocking(client_socket);
        clients.insert(std::make_pair(client_socket, Client(client_socket)));
        std::cout << "New client connected: " << client_socket << std::endl;
    }
}

void NonBlockingServer::handleClientRead(int client_socket)
{
    char buffer[1024] = {0};
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_read > 0)
    {
        std::map<int, Client>::iterator it = clients.find(client_socket);
        if (it != clients.end())
        {
            it->second.getBuffer().append(buffer, bytes_read);
            std::cout << "Received from client " << client_socket << ": " << it->second.getBuffer();
            handleClientWrite(client_socket);
        }
    }
    else if (bytes_read == 0 || (bytes_read == -1 && errno != EWOULDBLOCK))
    {
        to_remove.push_back(client_socket);
        FD_CLR(client_socket, &read_fds);
    }
}

void NonBlockingServer::handleClientWrite(int client_socket)
{
    std::map<int, Client>::iterator it = clients.find(client_socket);
    if (it != clients.end())
    {
        std::string& buffer = it->second.getBuffer();
        int bytes_sent = send(client_socket, buffer.c_str(), buffer.length(), 0);

        if (bytes_sent > 0)
        {
            buffer.erase(0, bytes_sent);
            if (buffer.empty())
            {
                FD_CLR(client_socket, &write_fds);
            }
        }
        else if (bytes_sent == -1 && errno != EWOULDBLOCK)
        {
            to_remove.push_back(client_socket);
        }
    }
}

void NonBlockingServer::removeDisconnectedClients()
{
    for (size_t i = 0; i < to_remove.size(); ++i)
    {
        int socket = to_remove[i];
        std::cout << "Client disconnected: " << socket << std::endl;
        close(socket);
        clients.erase(socket);
        FD_CLR(socket, &read_fds);
        FD_CLR(socket, &write_fds);
        FD_CLR(socket, &except_fds);
    }
    to_remove.clear();
}

void NonBlockingServer::run()
{
    while (true)
    {
        fd_set temp_read_fds = read_fds;
        fd_set temp_write_fds = write_fds;
        fd_set temp_except_fds = except_fds;

        int max_fd = server_socket;
        std::map<int, Client>::iterator it;
        for (it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->first > max_fd)
            {
                max_fd = it->first;
            }
        }

        if (select(max_fd + 1, &temp_read_fds, &temp_write_fds, &temp_except_fds, NULL) < 0)
        {
            throw SelectException();
        }

        if (FD_ISSET(server_socket, &temp_read_fds))
        {
            std::cout << "Satisfied accept read condition" << std::endl;
            acceptNewClient();
        }

        for (it = clients.begin(); it != clients.end(); ++it)
        {
            int client_socket = it->first;
            if (FD_ISSET(client_socket, &temp_read_fds))
            {
                std::cout << "Satisfied read condition" << std::endl;
                handleClientRead(client_socket);
            }
            if (FD_ISSET(client_socket, &temp_write_fds))
            {
                std::cout << "Satisfied write condition" << std::endl;
                handleClientWrite(client_socket);
            }
            if (FD_ISSET(client_socket, &temp_except_fds))
            {
                std::cout << "Satisfied except condition" << std::endl;
                to_remove.push_back(client_socket);
            }
        }
        removeDisconnectedClients();
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        for (it = clients.begin(); it != clients.end(); ++it)
        {
            FD_SET(it->first, &read_fds);
        }
    }
}

NonBlockingServer::~NonBlockingServer()
{
    close(server_socket);
    std::map<int, Client>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it)
    {
        close(it->first);
    }
}
