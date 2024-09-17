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

    struct pollfd server_pollfd;
    server_pollfd.fd = server_socket;
    server_pollfd.events = POLLIN;
    poll_fds.push_back(server_pollfd);
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

        struct pollfd client_pollfd;
        client_pollfd.fd = client_socket;
        client_pollfd.events = POLLIN | POLLOUT;
        poll_fds.push_back(client_pollfd);

        std::cout << "New client connected: " << client_socket << std::endl;
    }
}

void NonBlockingServer::handleClientRead(int client_socket)
{
    char buffer[1024] = {0};
    int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read > 0)
    {
        std::map<int, Client>::iterator it = clients.find(client_socket);
        if (it != clients.end())
        {
            it->second.getBuffer().append(buffer, bytes_read);
            std::cout << "Received from client " << client_socket << ": " << it->second.getBuffer();
        }
    }
    else if (bytes_read == 0 || (bytes_read == -1 && errno != EWOULDBLOCK))
    {
        to_remove.push_back(client_socket);
    }
}

void NonBlockingServer::handleClientWrite(int client_socket)
{
    std::map<int, Client>::iterator it = clients.find(client_socket);
    if (it != clients.end())
    {
        std::string& buffer = it->second.getBuffer();
        if (buffer.empty())
            return ;
        int bytes_sent = send(client_socket, buffer.c_str(), buffer.length(), 0);

        if (bytes_sent > 0)
        {
            buffer.erase(0, bytes_sent);
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

        // Remove the client from poll_fds
        for (size_t j = 0; j < poll_fds.size(); ++j)
        {
            if (poll_fds[j].fd == socket)
            {
                poll_fds.erase(poll_fds.begin() + j);
                break;
            }
        }
    }
    to_remove.clear();
}

void NonBlockingServer::run()
{
    while (true)
    {
        int poll_count = poll(poll_fds.data(), poll_fds.size(), -1);
        if (poll_count < 0)
            throw PollException();

        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if (poll_fds[i].fd == server_socket)
                    acceptNewClient();
                else
                    handleClientRead(poll_fds[i].fd);
            }

            if (poll_fds[i].revents & POLLOUT)
                handleClientWrite(poll_fds[i].fd);

            if (poll_fds[i].revents & (POLLHUP | POLLERR))
                to_remove.push_back(poll_fds[i].fd);
        }
        removeDisconnectedClients();
    }
}

NonBlockingServer::~NonBlockingServer()
{
    close(server_socket);
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        close(it->first);
    }
}
