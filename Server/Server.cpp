
#include "Server.hpp"

Server::Server(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes, EventManager* eventManager) :
	 serverSettings(serverSettings), mimeTypes(mimeTypes), eventManager(eventManager)
{
	memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    std::string ipAddress = serverSettings.getIP();
    inet_pton(AF_INET, ipAddress.c_str(), &(serverAddr.sin_addr));
    // serverAddr.sin_addr.s_addr = INADDR_ANY; // Binds to all interfaces (0.0.0.0) meaning the socket will listen on all network interfaces of the machine.
    serverAddr.sin_port = htons(serverSettings.getPort());
    serverPort = serverSettings.getPort();
    char serverStrAddress[INET_ADDRSTRLEN];
    inet_ntop(serverAddr.sin_family, &(serverAddr.sin_addr),
        serverStrAddress, sizeof(serverStrAddress));
    serverInterface = std::string(serverStrAddress);
}

void Server::setSocketOptions()
{
    if (serverSocket == -1)
        return ;

	int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        Logger::log(Logger::ERROR, "Failed to set socket options: " + std::string(strerror(errno)), "Server::setSocketOptions");
        serverSocket = -1;
    }
}

void Server::setupServerSocket()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Logger::log(Logger::ERROR, "Failed to create server socket: " + std::string(strerror(errno)), "Server::setupServerSocket");
        serverSocket = -1;
    }
    else {
        std::stringstream ss;
        ss << serverSocket;
        Logger::log(Logger::INFO, "Server socket with fd " + ss.str() + " created succesfuly", "Server::setupServerSocket");
    }
}

void Server::bindAndListenServerSocket()
{
    if (serverSocket == -1)
        return ;

	if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        Logger::log(Logger::ERROR, "Failed to bind server socket: " + std::string(strerror(errno)), "Server::bindAndListenServerSocket");
        serverSocket = -1;
        return ;
    }
	if (listen(serverSocket, 5) < 0) {
        Logger::log(Logger::ERROR, "Failed to listen on socket: " + std::string(strerror(errno)), "Server::bindAndListenServerSocket");
        serverSocket = -1;
    }
}

void Server::setNonBlocking(int& sockFD)
{
    if (serverSocket == -1)
        return ;
	int flags = fcntl(sockFD, F_GETFL, 0);
    if (flags < 0) {
        Logger::log(Logger::ERROR, "Failed to set socket to nonblocking: " + std::string(strerror(errno)), "Server::setNonBlocking");
        serverSocket = -1;
    }
    if (fcntl(sockFD, F_SETFL, flags | O_NONBLOCK) < 0) {
        Logger::log(Logger::ERROR, "Failed to set socket to nonblocking:  " + std::string(strerror(errno)), "Server::setNonBlocking");
        serverSocket = -1;
    }
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

int& Server::getServerPort()
{
    return (serverPort);
}

std::string& Server::getServerInterface()
{
    return (serverInterface);
}


void Server::acceptNewClient()
{
    struct sockaddr_in  clientAddr;
    socklen_t           clientLen = sizeof(clientAddr);
    char				clientStrAddress[INET_ADDRSTRLEN]; // consider adding to client class

    memset(&clientAddr, 0, sizeof(clientAddr));
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSocket == -1) {
        Logger::log(Logger::ERROR, "Could not accept new connection: " + std::string(strerror(errno)), "Server::acceptNewClient");
        return ;
    }
    Logger::log(Logger::INFO, "Accepted new connection on socket fd: " + Logger::intToString(clientSocket), "Server::acceptNewConnection");
	int flags = fcntl(clientSocket, F_GETFL, 0);
    if (flags < 0) {
        Logger::log(Logger::ERROR, "Failed to set socket to nonblocking: " + std::string(strerror(errno)), "Server::acceptNewConnection");
        close(clientSocket);
        return ;
    }
    if (fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK) < 0) {
        Logger::log(Logger::ERROR, "Failed to set socket to nonblocking:  " + std::string(strerror(errno)), "Server::acceptNewConnection");
        close(clientSocket);
        return ;
    }
    clients.insert(std::make_pair(clientSocket, Client(clientSocket)));
    inet_ntop(clientAddr.sin_family, &(clientAddr.sin_addr),
        clientStrAddress, sizeof(clientStrAddress));

    eventManager->registerEvent(clientSocket, READ);
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
                try to dereference this invalid iterator in while (it != clients.end()), which leads to 
                undefined behavior. But to fix this issue, we have to capture the next iterator through 
                the current iterator before erasing it. After the iterator has been shifted, we can now 
                safely erase the current iterator.
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

        if (it->second.isTimedout(timeoutValue)) {
            Logger::log(Logger::INFO, "Client with fd " + Logger::intToString(it->second.getSocket()) + 
                " has timed out, proceeding to disconnect", "Server::checkTimeouts");
            eventManager->deregisterEvent(it->second.getSocket(), READ);
            close(it->second.getSocket());
            std::map<int, Client>::iterator toErase = it; // Note 1
            it++;
            clients.erase(toErase);  // Erase using the saved iterator
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

        Logger::log(Logger::INFO, "Client with fd " + Logger::intToString(socket) + 
                " has disconnected", "Server::removeDisconnectedClients");
    }
    toRemove.clear();
}
