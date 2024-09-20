#include "Webserv.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/event.h>

class WebservException : public std::exception {
public:
    WebservException(const std::string& message) : message(message) {}
    const char* what() const noexcept override { return message.c_str(); }
private:
    std::string message;
};

Webserv::Webserv(const std::string& config_file)
{
    parseConfigFile(config_file);

    kq = kqueue();
    if (kq == -1)
    {
        throw WebservException("Failed to create kqueue");
    }

    for (int server_socket : server_sockets)
    {
        struct kevent event;
        EV_SET(&event, server_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(kq, &event, 1, NULL, 0, NULL) == -1)
        {
            throw WebservException("Failed to add server socket to kqueue");
        }
    }
}

void Webserv::setNonBlocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

void Webserv::acceptNewClient(int server_socket)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);

    if (client_socket >= 0)
    {
        setNonBlocking(client_socket);
        clients.insert(std::make_pair(client_socket, Client(client_socket)));

        struct kevent event;
        EV_SET(&event, client_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(kq, &event, 1, NULL, 0, NULL) == -1)
        {
            throw WebservException("Failed to add client socket to kqueue");
        }

        EV_SET(&event, client_socket, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
        if (kevent(kq, &event, 1, NULL, 0, NULL) == -1)
        {
            throw WebservException("Failed to add client socket to kqueue");
        }

        std::cout << "New client connected: " << client_socket << std::endl;
    }
}

void Webserv::handleClientRead(int client_socket)
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
        }
    }
    else if (bytes_read == 0 || (bytes_read == -1 && errno != EWOULDBLOCK))
    {
        removeClient(client_socket);
    }
}

void Webserv::handleClientWrite(int client_socket)
{
    std::map<int, Client>::iterator it = clients.find(client_socket);
    if (it != clients.end())
    {
        std::string& buffer = it->second.getBuffer();
        int bytes_sent = send(client_socket, buffer.c_str(), buffer.length(), 0);

        if (bytes_sent > 0)
        {
            buffer.erase(0, bytes_sent);
        }
        else if (bytes_sent == -1 && errno != EWOULDBLOCK)
        {
            removeClient(client_socket);
        }
    }
}

void Webserv::removeClient(int client_socket)
{
    std::cout << "Client disconnected: " << client_socket << std::endl;
    close(client_socket);
    clients.erase(client_socket);

    struct kevent event;
    EV_SET(&event, client_socket, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    kevent(kq, &event, 1, NULL, 0, NULL);
    EV_SET(&event, client_socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    kevent(kq, &event, 1, NULL, 0, NULL);
}

void Webserv::run()
{
    struct kevent events[MAX_EVENTS];
    while (true)
    {
        int n = kevent(kq, NULL, 0, events, MAX_EVENTS, NULL);
        if (n < 0)
        {
            throw WebservException("Kevent failed");
        }

        for (int i = 0; i < n; i++)
        {
            if (std::find(server_sockets.begin(), server_sockets.end(), events[i].ident) != server_sockets.end())
            {
                acceptNewClient(events[i].ident);
            }
            else
            {
                if (events[i].filter == EVFILT_READ)
                {
                    handleClientRead(events[i].ident);
                }
                else if (events[i].filter == EVFILT_WRITE)
                {
                    handleClientWrite(events[i].ident);
                }
            }
        }
    }
}

void Webserv::parseConfigFile(const std::string& config_file)
{
    // TODO: Implement configuration file parsing
    // For now, we'll just create a single server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        throw WebservException("Failed to create server socket");
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw WebservException("Failed to set socket options");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);  // Default port, should be configurable

    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        throw WebservException("Failed to bind server socket");
    }

    if (listen(server_socket, 5) < 0)
    {
        throw WebservException("Failed to listen on server socket");
    }

    setNonBlocking(server_socket);
    server_sockets.push_back(server_socket);
}

Webserv::~Webserv()
{
    for (int server_socket : server_sockets)
    {
        close(server_socket);
    }
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        close(it->first);
    }
    close(kq);
}