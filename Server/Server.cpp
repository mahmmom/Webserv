
#include "Server.hpp"

Server::Server(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes, EventManager* eventManager) :
	 serverSettings(serverSettings), mimeTypes(mimeTypes), eventManager(eventManager)
{
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Binds to all interfaces (0.0.0.0) meaning the socket will listen on all network interfaces of the machine.
    serverAddr.sin_port = htons(serverSettings.getPort());

    std::cout << "Just as a double cheek" << std::endl;

    char serverStrAddress[INET_ADDRSTRLEN];
    inet_ntop(serverAddr.sin_family, &(serverAddr.sin_addr),
        serverStrAddress, sizeof(serverStrAddress));
    std::cout << "Server address is " << serverStrAddress << std::endl;
    std::cout << "Server port is listening on " << ntohs(serverAddr.sin_port) << std::endl;
}

void Server::setSocketOptions()
{
	int opt = 1;

    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw SetsockoptException();
    }
}

void Server::setupServerSocket()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
}

void Server::bindAndListenServerSocket()
{
	if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        throw BindException();
    }
	if (listen(serverSocket, 5) < 0)
    {
        throw ListenException();
    }
}

void Server::setNonBlocking(int& sockFD)
{
	int flags = fcntl(sockFD, F_GETFL, 0);
    fcntl(sockFD, F_SETFL, flags | O_NONBLOCK);
}

void Server::launch()
{
	setupServerSocket();
	setSocketOptions();
	setNonBlocking(serverSocket);
	bindAndListenServerSocket();
}

int& Server::getServerSocket()
{
	return (serverSocket);
}

void Server::acceptNewClient()
{
    struct sockaddr_in  clientAddr;
    socklen_t           clientLen = sizeof(clientAddr);
    char				clientStrAddress[INET_ADDRSTRLEN]; // consider adding to client class

    memset(&clientAddr, 0, sizeof(clientAddr));
    int client_socket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (client_socket == -1) {
        throw(AcceptException());
    }

    setNonBlocking(client_socket);
    clients.insert(std::make_pair(client_socket, Client(client_socket)));
    inet_ntop(clientAddr.sin_family, &(clientAddr.sin_addr),
        clientStrAddress, sizeof(clientStrAddress));

    std::cout << "New client connected from: " << clientStrAddress
            << " on fd (" << client_socket << ")" << std::endl;

    eventManager->registerEvent(client_socket, READ);
}

void Server::handleGetRequest(int& clientSocketFD, HTTPRequest& request)
{
    ResponseGenerator responseGenerator(serverSettings, mimeTypes);

    HTTPResponse response = responseGenerator.handleRequest(request);
    responses[clientSocketFD] =  response;
    eventManager->registerEvent(clientSocketFD, WRITE);
}

void Server::handleClientRead(int& clientSocketFD)
{
	char buffer[BUFFER_SIZE] = {0};
    int bytes_read = recv(clientSocketFD, buffer, sizeof(buffer), 0);

    if (bytes_read < 0 && errno != EWOULDBLOCK) {
        perror ("Recv"); // probably should remove this
        std::cerr << "Will proceed to disconnect client (" 
            << clientSocketFD << ")" << std::endl;
        toRemove.push_back(clientSocketFD);
    }
    else if (bytes_read < 0 && errno == EWOULDBLOCK) {
        perror("*Recv");
    }
    else if (bytes_read == 0) {
        toRemove.push_back(clientSocketFD);
    }
    else
    {
        std::map<int, Client>::iterator it = clients.find(clientSocketFD);
        if (it != clients.end())
        {
            buffer[bytes_read] = '\0';
            it->second.getBuffer().append(buffer, bytes_read);
            std::cout << "Recieved from "
					<< "(" << it->second.getSocket() << ")"
					<< " the following -> " << it->second.getBuffer()
					<< " (bytes recieved: " << bytes_read << ")"
					<< std::endl << std::endl;

            HTTPRequest request(buffer);

            (it)->second.request = request;
            (it)->second.lastRequestTime = std::time(0);

            // LocationSettings* location = serverSettings.findLocation(Request.getURI());
            // if (!location)
            //     std::cerr << "Sorry mate, no location was found for this uri" << std::endl;
            // else
            //     std::cout << "Good boy with good logic" << std::endl;

            if (request.getMethod() == "GET")
                handleGetRequest(clientSocketFD, request);
        }
    }
}

void Server::handleClientWrite(int& clientSocketFD)
{
    std::map<int, Client>::iterator it = clients.find(clientSocketFD);
    if (it != clients.end())
    {
        std::string response = responses[clientSocketFD].generateResponse();
        // std::cout << "\n===RESPONSE===\n";
        // std::cout << response << std::endl;
        int bytes_sent = send(clientSocketFD, response.c_str(), response.length(), 0);
        if (bytes_sent == -1 && errno != EWOULDBLOCK) {
            perror ("Send"); // probably should remove this
            std::cerr << "Will proceed to disconnect client (" 
                << clientSocketFD << ")" << std::endl;
            toRemove.push_back(clientSocketFD);
        } 
        else if ((bytes_sent == -1 && errno == EWOULDBLOCK)) {
            perror("*Send");
        }
        else {
            std::cout << "successfuly sent" << std::endl;
            responses.erase(clientSocketFD);
        }
        eventManager->deregisterEvent(clientSocketFD, WRITE);
    }
}

/*
    NOTES:

        Note 1: Iterator Invalidation: When you call clients.erase(it);, the iterator it becomes invalid. 
                In the next iteration of the loop, you would experience a SEGFAULT because you would then 
                try to dereference this invalid iterator with it++, which leads to undefined behavior. 
                But to fix this issue, the erase function is designed to return the next valid iterator 
                and that is why we manually say it = clients.erase(it). 
*/
void    Server::checkTimeouts()
{
    std::map<int, Client>::iterator it = clients.begin();

    while (it != clients.end()) {
        BaseSettings*		settings = &serverSettings;
        LocationSettings* 	locationSettings = serverSettings.findLocation((it)->second.request.getURI());
        if (locationSettings)
            settings = locationSettings;

        size_t timeoutValue = settings->getKeepaliveTimeout();

        std::cout << "WTF " << settings->getRoot() << std::endl;

        if (it->second.isTimedout(timeoutValue)) {
            std::cout << "Client " << it->second.getSocket() << " has timed out, proceeding to disconnect" << std::endl;
            eventManager->deregisterEvent(it->second.getSocket(), READ);
            close(it->second.getSocket());
            it = clients.erase(it); // Note 1
        }
        else
            it++;
    }
}

void Server::removeDisconnectedClients()
{
	for (size_t i = 0; i < toRemove.size(); ++i)
    {
        int socket = toRemove[i];

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
    toRemove.clear();
}
