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
#include "../HTTPRequest/HTTPRequest.hpp"

int NonBlockingServer::count = 0;

NonBlockingServer::NonBlockingServer(int port)
{
    #ifdef __APPLE__
        eventlist.resize(MAX_EVENTS);
    #elif __linux__
        events.resize(MAX_EVENTS);
    #endif

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw SocketException();
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw SetsockoptException();
    }

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw BindException();
    }

    if (listen(server_socket, BACKLOG) < 0) {
        throw ListenException();
    }

    setNonBlocking(server_socket);

    #ifdef __APPLE__
        kq = kqueue();
        if (kq == -1) {
            perror("kqueue");
            exit(EXIT_FAILURE);
        }
        struct kevent change_event;
        EV_SET(&change_event, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        changelist.push_back(change_event);
        kevent(kq, &changelist[0], changelist.size(), NULL, 0, NULL);
    #elif __linux__
        epoll_fd = epoll_create(MAX_EVENTS);
        if (epoll_fd == -1) {
            perror("epoll_create");
            exit(EXIT_FAILURE);
        }
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = server_socket;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
            perror("epoll_ctl");
            exit(EXIT_FAILURE);
        }
    #endif
}

void NonBlockingServer::setNonBlocking(int socket) {
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

void NonBlockingServer::acceptNewClient() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char client_str_address[INET_ADDRSTRLEN];

    memset(&client_addr, 0, sizeof(client_addr));
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket == -1) {
        throw AcceptException();
    }
    setNonBlocking(client_socket);
    clients.insert(std::make_pair(client_socket, Client(client_socket)));
    inet_ntop(client_addr.sin_family, &(client_addr.sin_addr), client_str_address, sizeof(client_str_address));

    std::cout << "New client connected from: " << client_str_address 
              << " on fd (" << client_socket << ")" << std::endl;

    #ifdef __APPLE__
        struct kevent change_event;
        EV_SET(&change_event, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        changelist.push_back(change_event);
        kevent(kq, &changelist[0], changelist.size(), NULL, 0, NULL);
    #elif __linux__
        struct epoll_event event;
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = client_socket;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
            perror("epoll_ctl: client_socket");
            exit(EXIT_FAILURE);
        }
    #endif
}

void NonBlockingServer::handleClientRead(int client_socket) {
    char buffer[MAXDATASIZE];
    memset(buffer, 0, MAXDATASIZE);
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_read < 0 && errno != EWOULDBLOCK) {
        std::cerr << "Will proceed to disconnect client (" << client_socket << ")" << std::endl;
        to_remove.push_back(client_socket);
    }
    else if (bytes_read == 0) {
        to_remove.push_back(client_socket);
    }
    else if (bytes_read > 0) {
        std::map<int, Client>::iterator it = clients.find(client_socket);
        if (it != clients.end()) {
            buffer[bytes_read] = '\0';
            it->second.getBuffer().append(buffer, bytes_read);
            std::cout << "Received from (" << it->second.getSocket() << ") -> " 
                      << it->second.getBuffer() << " (bytes received: " << bytes_read << ")" << std::endl;

            HTTPRequest Request(buffer);

            #ifdef __APPLE__
                struct kevent change_event;
                EV_SET(&change_event, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                changelist.push_back(change_event);
            #elif __linux__
                struct epoll_event event;
                event.events = EPOLLOUT | EPOLLET;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_socket, &event) == -1) {
                    perror("epoll_ctl: client_socket");
                    exit(EXIT_FAILURE);
                }
            #endif
        }
    }
}

void NonBlockingServer::handleClientWrite(int client_socket) {
    std::map<int, Client>::iterator it = clients.find(client_socket);
    if (it != clients.end()) {
        std::string buffer_head = "Relay from server: ";
        std::string& buffer = it->second.getBuffer();
        int bytes_sent = send(client_socket, (buffer_head + buffer).c_str(), (buffer_head + buffer).length(), 0);
        if (bytes_sent == -1 && errno != EWOULDBLOCK) {
            std::cerr << "Will proceed to disconnect client (" << client_socket << ")" << std::endl;          
            to_remove.push_back(client_socket);
        } 
        else if (bytes_sent > 0) {
            std::cout << "Successfully sent" << std::endl;
            buffer.erase(0, bytes_sent - buffer_head.length());
        }

        #ifdef __APPLE__
            struct kevent change_event;
            EV_SET(&change_event, client_socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
            changelist.push_back(change_event);
        #elif __linux__
            struct epoll_event event;
            event.events = EPOLLIN | EPOLLET;
            event.data.fd = client_socket;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_socket, &event) == -1) {
                perror("epoll_ctl: client_socket");
                exit(EXIT_FAILURE);
            }
        #endif
    }
}

void NonBlockingServer::removeDisconnectedClients() {
    for (std::vector<int>::iterator it = to_remove.begin(); it != to_remove.end(); ++it) {
        close(*it);
        clients.erase(*it);
        std::cout << "Client disconnected: " << *it << std::endl;
    }
    to_remove.clear();
}

void NonBlockingServer::run() {
    while (true) {
        #ifdef __APPLE__
            int nev = kevent(kq, &changelist[0], changelist.size(), &eventlist[0], eventlist.size(), NULL);
            changelist.clear();
            for (int i = 0; i < nev; i++) {
                if (eventlist[i].ident == (uintptr_t)server_socket) {
                    acceptNewClient();
                }
                else if (eventlist[i].filter == EVFILT_READ) {
                    handleClientRead(eventlist[i].ident);
                }
                else if (eventlist[i].filter == EVFILT_WRITE) {
                    handleClientWrite(eventlist[i].ident);
                }
            }
        #elif __linux__
            int nfds = epoll_wait(epoll_fd, &events[0], MAX_EVENTS, -1);
            for (int n = 0; n < nfds; ++n) {
                if (events[n].data.fd == server_socket) {
                    acceptNewClient();
                } else if (events[n].events & EPOLLIN) {
                    handleClientRead(events[n].data.fd);
                } else if (events[n].events & EPOLLOUT) {
                    handleClientWrite(events[n].data.fd);
                }
            }
        #endif
        removeDisconnectedClients();
    }
}

NonBlockingServer::~NonBlockingServer() {
    close(server_socket);
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        close(it->first);
    }
    #ifdef __APPLE__
        close(kq);
    #elif __linux__
        close(epoll_fd);
    #endif
}