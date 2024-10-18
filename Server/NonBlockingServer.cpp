
#include "NonBlockingServer.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h> //
#include <arpa/inet.h> //
#include <fcntl.h> //
#include <unistd.h>
#include <errno.h>
#include "Errors.hpp"
#include "../HTTPRequest/HTTPRequest.hpp"

int NonBlockingServer::count = 0;

/*
    GENERAL INFO:
        
        Function: NonBlockingServer::handleClientWrite();

            We have to deregister that EV_FILTWRITE event as soon as we send because if 
            you read the manual, it says: "Takes a descriptor as the identifier, and 
            returns whenever it is possible to write to the descriptor." That means that
            the EV_FILTWRITE will constantly trigger an event even if we are not writing, 
            the event is triggered merely by the socket being "writeable". So for most 
            cases, that simly means that if the client socket buffer is not full, then 
            it is deemed writable and just in that state, it will constantly trigger an 
            detectable event. So instead, we use it as a temporary "flag", only activate 
            it when I want to write something, if the socket is writeable then great, 
            detect the event so I can call handleClientWrite() and then as soon as the 
            data is sent, deunregister this event.
*/
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

	if ((kq = kqueue()) == -1) {
		perror("Kqueue: ");
		exit(EXIT_FAILURE);
	}

    eventManager = new KqueueManager();
    eventManager->registerEvent(server_socket, READ);
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

    eventManager->registerEvent(client_socket, READ);
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

            HTTPRequest Request(buffer);
            eventManager->registerEvent(client_socket, WRITE);
        }
    }
}

void NonBlockingServer::handleClientWrite(int client_socket)
{
    std::map<int, Client>::iterator it = clients.find(client_socket);
    if (it != clients.end())
    {
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

        eventManager->deregisterEvent(client_socket, WRITE);
    }
}

void NonBlockingServer::removeDisconnectedClients()
{
    for (size_t i = 0; i < to_remove.size(); ++i)
    {
        int socket = to_remove[i];

        eventManager->deregisterEvent(socket, READ);

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
        int nev = eventManager->eventListener();
        for (int i = 0; i < nev; i++) {

            EventBlock eventBlock = eventManager->getEvent(i);
            if (eventBlock.isRead && (eventBlock.fd == server_socket)) {
                acceptNewClient();
            }
            else if (eventBlock.isRead && (eventBlock.fd != server_socket)) {
                handleClientRead(eventBlock.fd);
            }
            else if (eventBlock.isWrite) {
                handleClientWrite(eventBlock.fd);
            }
        }
        removeDisconnectedClients();
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
