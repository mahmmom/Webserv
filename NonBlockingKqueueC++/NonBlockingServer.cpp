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

int NonBlockingServer::count = 0;

NonBlockingServer::NonBlockingServer(int port) : eventlist(10)
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

	if ((kq = kqueue()) == -1) {
		perror("Kqueue: ");
		exit(EXIT_FAILURE);
	}
    struct kevent change_event;
	EV_SET(&change_event, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	changelist.push_back(change_event);
	kevent(kq, changelist.data(), changelist.size(), NULL, 0, 0);
}

void NonBlockingServer::setNonBlocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

void NonBlockingServer::acceptNewClient()
{
    struct sockaddr_in  client_addr;
    socklen_t           client_len = sizeof(client_addr);
    char				client_str_address[INET_ADDRSTRLEN]; // consider adding to client class

    memset(&client_addr, 0, sizeof(client_addr));
    int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket == -1) {
        throw(AcceptException());
    }
    setNonBlocking(client_socket);
    clients.insert(std::make_pair(client_socket, Client(client_socket)));
    inet_ntop(client_addr.sin_family, &(client_addr.sin_addr),
        client_str_address, sizeof(client_str_address));

    std::cout << "New client connected from: " << client_str_address 
            << " on fd (" << client_socket << ")" << std::endl;

    struct kevent change_event;
    EV_SET(&change_event, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    changelist.push_back(change_event);
    kevent(kq, changelist.data(), changelist.size(), NULL, 0, 0);
}

void NonBlockingServer::handleClientRead(int client_socket)
{
    char buffer[MAXDATASIZE] = {0};
    int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_read < 0 && errno != EWOULDBLOCK) {
        perror ("Recv"); // probably should remove this
        std::cerr << "Will proceed to disconnect client (" 
            << client_socket << ")" << std::endl;
        to_remove.push_back(client_socket);
    }
    else if (bytes_read < 0 && errno == EWOULDBLOCK) {
        perror("*Recv");
    }
    else if (bytes_read == 0) {
        to_remove.push_back(client_socket);
    }
    else
    {
        std::map<int, Client>::iterator it = clients.find(client_socket);
        if (it != clients.end())
        {
            buffer[bytes_read] = '\0';
            it->second.getBuffer().append(buffer, bytes_read);
            std::cout << "Recieved from "
					<< "(" << it->second.getSocket() << ")"
					<< " the following -> " << it->second.getBuffer()
					<< " (bytes recieved: " << bytes_read << ")"
					<< std::endl << std::endl;

            struct kevent change_event;
            EV_SET(&change_event, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
            templist.push_back(change_event);
        }
    }
}

void NonBlockingServer::handleClientWrite(int client_socket)
{
    std::map<int, Client>::iterator it = clients.find(client_socket);
    if (it != clients.end())
    {
        // if (count == 0)
        // {
        //     int pid = fork();
        //     if (pid == 0){
        //         signal(SIGPIPE, SIG_IGN);   // Signal Handling (SIGPIPE): When sending 
        //                                     // data to a closed socket, the system can 
        //                                     // send a SIGPIPE signal to the process. 
        //                                     // By default, this signal terminates the process.
        //                                     // In your case, if the client closes the connection 
        //                                     // while the server is still sending data, the child 
        //                                     // process may receive SIGPIPE, causing it to terminate 
        //                                     // silently without reaching your exit(0) or printing 
        //                                     // the perror messages.
        //         std::string response = "Hello from server XD";
        //         int bytes_sent;
        //         int i = 0;
        //         do {
        //             bytes_sent = send(client_socket, response.c_str(), response.length(), 0);
        //             std::cout << "[" << i << "] " << "Sent from server " << bytes_sent << " bytes to client_socket " << client_socket << std::endl;
        //             i++;
        //         } while (bytes_sent != -1);
        //         if (bytes_sent == -1 && errno != EWOULDBLOCK) {
        //             perror ("Send"); // probably should remove this
        //         } 
        //         else if ((bytes_sent == -1 && errno == EWOULDBLOCK)) {
        //             perror("*Send");
        //         }
        //         std::cout << "exit" << std::endl;
        //         exit(0);
        //     }
        //     std::string& buffer = it->second.getBuffer();
        //     buffer.erase(0, 20);
        //     count++;
        //     struct kevent change_event;
        //     EV_SET(&change_event, client_socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        //     templist.push_back(change_event);
        //     return ;
        // }
        std::string buffer_head = "Relay from server: ";
        std::string& buffer = it->second.getBuffer();
        int bytes_sent = send(client_socket, (buffer_head + buffer).c_str(), (buffer_head + buffer).length(), 0);
        if (bytes_sent == -1 && errno != EWOULDBLOCK) {
            perror ("Send"); // probably should remove this
            std::cerr << "Will proceed to disconnect client (" 
                << client_socket << ")" << std::endl;          
            to_remove.push_back(client_socket);
        } 
        else if ((bytes_sent == -1 && errno == EWOULDBLOCK)) {
            perror("*Send");
        }
        else {
            std::cout << "successfuly sent" << std::endl;
            buffer.erase(0, bytes_sent);
        }
        struct kevent change_event;
        EV_SET(&change_event, client_socket, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        templist.push_back(change_event);
    }
}

void NonBlockingServer::removeDisconnectedClients()
{
    for (size_t i = 0; i < to_remove.size(); ++i)
    {
        int socket = to_remove[i];
        close(socket);  // manual says EV_DELETE: Removes the event from the kqueue. 
                        // Events which are attached to file descriptors are automatically 
                        // deleted on the last close of the descriptor. So we can just close
                        // and any events attached to that the socket are automatically dequeued.
        
        clients.erase(socket);  // erase doesn't need an iterator here because we are working with
                                // maps; for std::maps, we can erase an element by its key because 
                                // no duplicate keys are allowed. But for a vector, we can have 
                                // duplicate values and thus, the erase method (its version of the
                                // override) requires an iterator.
        std::cout << "Client disconnected: " << socket << std::endl << std::endl;
    }
    to_remove.clear();
}

void NonBlockingServer::run()
{
    while (true)
    {
        // std::cout << "New horde" << std::endl;
        int nev = kevent(kq, NULL, 0, eventlist.data(), eventlist.size(), NULL);
        for (int i = 0; i < nev; i++) {
            if (eventlist[i].filter == EVFILT_READ && eventlist[i].ident == (uintptr_t) server_socket) {
                acceptNewClient();
            }
            else if (eventlist[i].filter == EVFILT_READ && eventlist[i].ident != (uintptr_t) server_socket) {
                handleClientRead(eventlist[i].ident);
            }
            else if (eventlist[i].filter == EVFILT_WRITE) {
                handleClientWrite(eventlist[i].ident);
            }
        }
        if (!templist.empty()) {
            kevent(kq, templist.data(), templist.size(), NULL, 0, 0);
            templist.clear();
        }
        if ((size_t) nev == eventlist.size()) {
            eventlist.resize(eventlist.size() * 2);  // Double the size if full
        }
        removeDisconnectedClients();
        // std::cout << std::endl;
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
