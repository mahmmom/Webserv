
#include "Server.hpp"
#include "ClientManager.hpp"

size_t simulatedSend(int socket, const void* buffer, size_t length, int flags)
{

    size_t bytesToSend;
    // Simulate sending only a portion of the data
    if (length == 1)
        bytesToSend = length;
    else    
        bytesToSend = length / 2; // Simulate sending half of the requested data
    
    // Ensure we do not send more than available
    if (bytesToSend > length) {
        bytesToSend = length;
    }

    // Actually send the data over the socket
    ssize_t result = send(socket, buffer, bytesToSend, flags);

    if (result < 0) {
        std::cerr << "Error sending data: " << strerror(errno) << std::endl;
        return -1; // Return -1 on send error
    }

    std::cout << "Sent " << result << " bytes."; // Log how many bytes were sent
    return result; // Return the actual number of bytes sent
}

/*
=====================================================================================
                                SERVER SETUP
=====================================================================================
*/
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

Server::~Server()
{
    std::map<int, ResponseManager* >::iterator responsesIt;
    for (responsesIt = responses.begin(); responsesIt != responses.end(); responsesIt++) {
        delete responsesIt->second;
    }
    responses.clear();

    std::map<int, ClientManager* >::iterator clientsIt;
    for (clientsIt = clients.begin(); clientsIt != clients.end(); clientsIt++) {
        delete clientsIt->second;
        close (clientsIt->first);
    }
    clients.clear();

    std::map<int, CGIManager* >::iterator cgiIt;
    for (cgiIt = cgi.begin(); cgiIt != cgi.end(); cgiIt++) {
        delete cgiIt->second;
    }
    cgi.clear();

    if (serverSocket != -1) {
       close(serverSocket);
    }
}

bool Server::checkClientInServer(int clientSocketFD)
{
    if (clients.count(clientSocketFD) > 0)
        return (false);
    return (true);
}

void Server::setSocketOptions()
{
    if (serverSocket == -1) {
        return ;
    }

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
    if (serverSocket == -1) {
        return ;
    }

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

void Server::setNonBlocking(int sockFD)
{
    if (serverSocket == -1) {
        return ;
    }
	int flags = fcntl(sockFD, F_GETFL, 0);
    if (flags < 0) {
        Logger::log(Logger::ERROR, "Failed to obtain server socket flags: " + std::string(strerror(errno)), "Server::setNonBlocking");
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

    inet_ntop(clientAddr.sin_family, &(clientAddr.sin_addr), clientStrAddress, sizeof(clientStrAddress));
    clients[clientSocket] = new ClientManager(clientSocket, clientStrAddress);
    eventManager->registerEvent(clientSocket, READ);
}

/*
=====================================================================================
                                  METHOD PROCESSORS
=====================================================================================
*/
/*
    NOTES

        Note 1: Everytime we write to a client, that means that we have recieved their request completely and 
                thus, in order to allow for new requests from that client without actually removing them from 
                our clients map, we simply reset the ClientManager class attributes via the method 
                ClientManager::resetClientState(). By doing so, the data for the current request will not get 
                jumbled up with data for the upcoming request made by the client.
*/
void Server::processGetRequest(int clientSocketFD, HTTPRequest& request)
{
    if (serverSettings.getCgiDirective().isEnabled() && CGIManager::isValidCGI(request, serverSettings)) {
        // std::cout << "Size is " << cgi.size() << std::endl;
        if (cgi.size() > CGI_LOAD_LIMIT) {
			Logger::log(Logger::WARN, "Server is overloaded at the moment and cannot process the cgi request", "Server::processGetRequest");
			handleInvalidRequest(clientSocketFD, "503", "Service Unavailable");
			return (void());
		}
        CGIManager* cgiManager = new CGIManager(request, serverSettings, eventManager, 
                                                    clientSocketFD);
        if (!(cgiManager->getErrorDetected()))
            cgi[cgiManager->getCgiFD()] = cgiManager;
        else {
            delete cgiManager;
            handleInvalidRequest(clientSocketFD, "500", "Internal Server Error");
        }
    }
    else {
        ResponseGenerator responseGenerator(serverSettings, mimeTypes);

        HTTPResponse response = responseGenerator.handleRequest(request);

        ResponseManager* responseManager = NULL;
        if (response.getType() == CompactResponse)
            responseManager = new ResponseManager(response.generateResponse(), false);
        else
            responseManager = new ResponseManager(response.generateResponse(), response.getFilePath(), response.getFileSize());
        if (clients.count(clientSocketFD) > 0) // Note 1
            clients[clientSocketFD]->resetClientManager();

        responses[clientSocketFD] =  responseManager;
        eventManager->registerEvent(clientSocketFD, WRITE);
    }
}

void Server::processPostRequest(int clientSocketFD, HTTPRequest& request)
{
    if (serverSettings.getCgiDirective().isEnabled() && CGIManager::isValidCGI(request, serverSettings)) {
        if (cgi.size() > CGI_LOAD_LIMIT) {
			Logger::log(Logger::WARN, "Server is overloaded at the moment and cannot process the cgi request", "Server::processGetRequest");
			handleInvalidRequest(clientSocketFD, "503", "Service Unavailable");
			return (void());
		}
        CGIManager* cgiManager = new CGIManager(request, serverSettings, eventManager, 
                                                    clientSocketFD, clients[clientSocketFD]->getPostRequestFileName());
        if (!(cgiManager->getErrorDetected()))
            cgi[cgiManager->getCgiFD()] = cgiManager;
        else {
            delete cgiManager;
            handleInvalidRequest(clientSocketFD, "500", "Internal Server Error");
        }
    }
    else {
        ResponseGenerator responseGenerator(serverSettings, mimeTypes);

        HTTPResponse response = responseGenerator.handleRequest(request);

        ResponseManager* responseManager = NULL;
        responseManager = new ResponseManager(response.generateResponse(), false);
        if (clients.count(clientSocketFD) > 0)
            clients[clientSocketFD]->resetClientManager();
        responses[clientSocketFD] =  responseManager;
        eventManager->registerEvent(clientSocketFD, WRITE);
    }
}

void Server::processHeadRequest(int clientSocketFD, HTTPRequest& request)
{
    ResponseGenerator responseGenerator(serverSettings, mimeTypes);

    HTTPResponse response = responseGenerator.handleRequest(request);

    ResponseManager* responseManager = NULL;
    responseManager = new ResponseManager(response.generateResponse(), false);
    if (clients.count(clientSocketFD) > 0)
        clients[clientSocketFD]->resetClientManager();
    responses[clientSocketFD] =  responseManager;
    eventManager->registerEvent(clientSocketFD, WRITE);
}

void Server::processDeleteRequest(int clientSocketFD, HTTPRequest& request)
{
    ResponseGenerator responseGenerator(serverSettings, mimeTypes);

    HTTPResponse response = responseGenerator.handleRequest(request);

    ResponseManager* responseManager = NULL;
    responseManager = new ResponseManager(response.generateResponse(), false);
    if (clients.count(clientSocketFD) > 0)
        clients[clientSocketFD]->resetClientManager();
    responses[clientSocketFD] =  responseManager;
    eventManager->registerEvent(clientSocketFD, WRITE);
}

/*
=====================================================================================
                                OPERATION HANDLERS
=====================================================================================
*/
void Server::handleClientRead(int clientSocketFD)
{
	char buffer[BUFFER_SIZE + 1] = {0};
    int bytesRead = recv(clientSocketFD, buffer, sizeof(buffer), 0);

    if (bytesRead < 0) {
        Logger::log(Logger::ERROR, "Failed to recieve data from client with socket fd: "
            + Logger::intToString(clientSocketFD), "Server::handleClientRead");
        removeBadClients(clientSocketFD);
        close(clientSocketFD);
    }
    else if (bytesRead == 0) {
        removeDisconnectedClients(clientSocketFD);
    }
    else {
        std::map<int, ClientManager* >::iterator it = clients.find(clientSocketFD);
        if (it != clients.end())
        {

            buffer[bytesRead] = '\0';

            // std::cout << "============================================================" << std::endl;
            // std::cout << "Recieved from "
			// 		<< "(" << it->second->getSocket() << ")"
			// 		<< " the following -> " << buffer
			// 		<< " (bytes recieved: " << bytesRead << ")"
			// 		<< std::endl;
            // std::cout << "============================================================" << std::endl;

            it->second->updateLastRequestTime();
            // it->second->incrementRequestsNumber();
            it->second->processIncomingData(*this, buffer, bytesRead);

        }
    }
}

void Server::handleClientWrite(int clientSocketFD)
{
    std::map<int, ResponseManager* >::iterator it = responses.find(clientSocketFD);
    if (it != responses.end()) {
        ResponseManager* responseManager = responses[clientSocketFD];
        if (responseManager->getType() == CompactResponse)
            sendCompactFile(clientSocketFD, responseManager);
        else
            sendChunkedResponse(clientSocketFD, responseManager);
    }
    else {
        Logger::log(Logger::WARN, "Rogue WRITE event not attributed to any response has been detected,"
            " on client with socket FD: " + Logger::intToString(clientSocketFD) + " proceeding to"
            " deregister it","Server::handleClientWrite");
        eventManager->deregisterEvent(clientSocketFD, WRITE);
    }
}

/*
=====================================================================================
                                CGI HANDLER
=====================================================================================
*/
void Server::handleCgiOutput(int cgiReadFD)
{
    Logger::log(Logger::DEBUG, "Handling CGI output from pipe with fd " + Logger::intToString(cgiReadFD), "Server::handleCgiOutput");

    char   buffer[BUFFER_SIZE + 1];
    ssize_t bytesRead = read(cgiReadFD, buffer, BUFFER_SIZE);
    CGIManager* cgiManager = cgi[cgiReadFD];

    if (bytesRead < 0) {
        kill(cgiManager->getChildPid(), SIGKILL);
        eventManager->deregisterEvent(cgiReadFD, READ);
        handleInvalidRequest(cgiManager->getCgiClientSocketFD(), "500", "Internal Server Error");
        cgi.erase(cgiReadFD);
        delete cgiManager;
    }
    else if (bytesRead == 0) {
        if (clients.count(cgiManager->getCgiClientSocketFD()) == 0)
        {
            Logger::log(Logger::ERROR, "CGI request was operating on socket fd " 
                + Logger::intToString(cgiManager->getCgiClientSocketFD()) + " but no "
                "client was found with such an fd", "Server::handleCgiOutput");
			eventManager->deregisterEvent(cgiReadFD, READ);
			cgi.erase(cgiReadFD);
			delete cgiManager;
			return;
        }
        ResponseManager *responseManager = new ResponseManager(cgiManager->generateCgiResponse(), false);
        if (clients.count(cgiManager->getCgiClientSocketFD()) > 0)
			clients[cgiManager->getCgiClientSocketFD()]->resetClientManager();
        responses[cgiManager->getCgiClientSocketFD()] = responseManager;
		eventManager->registerEvent(cgiManager->getCgiClientSocketFD(), WRITE);
		eventManager->deregisterEvent(cgiReadFD, READ);
		cgi.erase(cgiReadFD);
		delete cgiManager;
    }
    else {
        buffer[bytesRead] = '\0';
        std::string cgiResponseSnippet(buffer, bytesRead);
        cgiManager->appendCgiResponse(cgiResponseSnippet);
        if (cgiManager->getCgiResponse().length() >= MAX_CGI_OUTPUT_SIZE)
		{
			kill(cgiManager->getChildPid(), SIGKILL);
			eventManager->deregisterEvent(cgiReadFD, READ);
			handleInvalidRequest(cgiManager->getCgiClientSocketFD(), "500", "Internal Server Error");
			cgi.erase(cgiReadFD);
			delete cgiManager;
		}
    }
}

/*
=====================================================================================
                                    SENDERS
=====================================================================================
*/
/*
    NOTES

        Note 1: When testing on Linux, opening multiple tabs and requesting chunked resources 
                can actually cause the buffer to get full. All of the tabs will be on the same 
                fd, they are all the same client. It's not like on MacOS where each tab is a 
                separate client. Thus, if all are on the same socket and all are requesting 
                a large file sent in chunks, the buffer can easily get full. In that case, I 
                don't want to disconnect the client, instead, let him wait till the buffer is 
                full and request again. If you're worried about what if the client disconnected 
                (not that the buffer is full), then who cares what happens to him haha? Chances 
                are he'll trigger an EOF masked READ event which will gracefully terminate him on 
                the next run of the eventListener in ServerArena; or he will just time out if for 
                God knows whatever reason they did not trigger an EOF (like they terminated the session 
                in a non-graceful manner). 
*/
void Server::sendChunkedHeaders(int clientSocketFD, ResponseManager* responseManager)
{
    std::string headers = responseManager->getHeaders();
    const char* response = headers.c_str() + responseManager->getBytesSent();
    size_t      len = headers.length() - responseManager->getBytesSent();
    int      bytesSent = send(clientSocketFD, response, len, 0);

    if (bytesSent < 0)
    {
        Logger::log(Logger::ERROR, "Failed to send headers for a chunked response to client with "
            "socket fd " + Logger::intToString(clientSocketFD), "Server::sendChunkedHeaders");
        eventManager->deregisterEvent(clientSocketFD, WRITE);
        responses.erase(clientSocketFD);
        delete (responseManager);
        // removeBadClients(clientSocketFD);    // Note 1
        // close(clientSocketFD);
        return ;
    }

    responseManager->updateBytesSent(bytesSent);
    if (static_cast<size_t>(responseManager->getBytesSent()) >= responseManager->getHeaders().size())
    {
        responseManager->resetBytesSent();
        responseManager->setHeadersFullySent();
        Logger::log(Logger::DEBUG, "Headers for a chunked response have been fully sent to client with "
            "socket fd " + Logger::intToString(clientSocketFD), "Server::sendChunkedHeaders");
    }
    else
        Logger::log(Logger::DEBUG, "Headers for a chunked response have been partially sent to client with "
            "socket fd " + Logger::intToString(clientSocketFD), "Server::sendChunkedHeaders");
}

void Server::sendChunkedBody(int clientSocketFD, ResponseManager* responseManager)
{
    std::string chunk = responseManager->obtainChunk();

    const char* response = chunk.c_str() + responseManager->getBytesSent();
    size_t      len = chunk.length() - responseManager->getBytesSent();
    int         bytesSent = send(clientSocketFD, response, len, 0);

    if (bytesSent < 0)
    {
        Logger::log(Logger::WARN, "Failed to send a chunked response to client with "
            "socket fd, the client could be re-establishing a new connection more appropriate "
            "for chunked-encoding" + Logger::intToString(clientSocketFD), "Server::sendChunkedBody");
        eventManager->deregisterEvent(clientSocketFD, WRITE);
        responses.erase(clientSocketFD);
        delete (responseManager);
        // removeBadClients(clientSocketFD);
        // close(clientSocketFD);
        return ;
    }

    responseManager->updateBytesSent(bytesSent);
    if (static_cast<size_t>(responseManager->getBytesSent()) >= chunk.length())
    {
        responseManager->resetBytesSent();
        Logger::log(Logger::DEBUG, "A chunked response has been fully sent to client with socket fd " + 
            Logger::intToString(clientSocketFD), "Server::sendChunkedBody");
    }
    else
        Logger::log(Logger::DEBUG, "A chunked response has been partially sent to client with socket fd " + 
            Logger::intToString(clientSocketFD), "Server::sendChunkedBody");
    if (responseManager->isFinished())
    {
        std::string closingChunk = "0\r\n\r\n";
        int         bytesSent = send(clientSocketFD, closingChunk.c_str(), closingChunk.size(), 0);
        if (bytesSent < 0)
        {
            Logger::log(Logger::ERROR, "Failed to send the closing chunked response to client with "
                "socket fd " + Logger::intToString(clientSocketFD), "Server::sendChunkedBody");
            eventManager->deregisterEvent(clientSocketFD, WRITE);
            responses.erase(clientSocketFD);
            delete (responseManager);
            // removeBadClients(clientSocketFD);
            // close(clientSocketFD);
            return ;
        }
        Logger::log(Logger::DEBUG, "All chunked responses have been fully sent to client with socket fd " + 
            Logger::intToString(clientSocketFD), "Server::sendChunkedBody");
        eventManager->deregisterEvent(clientSocketFD, WRITE);
        responses.erase(clientSocketFD);
        delete (responseManager);
    }
}

void Server::sendChunkedResponse(int clientSocketFD, ResponseManager* responseManager)
{
    if (!responseManager->getHeadersFullySent())
        sendChunkedHeaders(clientSocketFD, responseManager);
    else
        sendChunkedBody(clientSocketFD, responseManager);
}

void    Server::sendCompactFile(int clientSocketFD, ResponseManager* responseManager)
{
    std::string compactResponse = responseManager->getCompactResponse();
    const char* response = compactResponse.c_str() + responseManager->getBytesSent();
    size_t      len = compactResponse.length() - responseManager->getBytesSent();
    int         bytesSent = send(clientSocketFD, response, len, 0);
    // size_t      bytesSent = simulatedSend(clientSocketFD, response, len, 0);

    if (bytesSent < 0)
    {
        Logger::log(Logger::ERROR, "Failed to send compact response to client with socket fd " + 
            Logger::intToString(clientSocketFD), "Server::sendCompactFile");
        eventManager->deregisterEvent(clientSocketFD, WRITE);
        responses.erase(clientSocketFD);
        delete (responseManager);
        // removeBadClients(clientSocketFD);
        // close(clientSocketFD);
        return ;
    }

    responseManager->updateBytesSent(bytesSent);
    if (responseManager->isFinished())
    {
        Logger::log(Logger::DEBUG, "A compact response has been fully sent to client with socket fd " + 
            Logger::intToString(clientSocketFD), "Server::sendCompactFile");
        eventManager->deregisterEvent(clientSocketFD, WRITE);
        responses.erase(clientSocketFD);
        if (responseManager->getCloseConnection()) {
            Logger::log(Logger::INFO, "Disconnecting client with socket fd: " + Logger::intToString(clientSocketFD)
                + " upon sending a compact response", "Server::sendCompactFile");
            close(clientSocketFD);
        }
        delete (responseManager);
    }
    else
        Logger::log(Logger::DEBUG, "A compact response has been sent partially to client with socket fd " + 
            Logger::intToString(clientSocketFD) + " for resource " + clients[clientSocketFD]->getRequest().getURI(), 
            "Server::sendCompactFile");
}

/*
=====================================================================================
                                TIMEOUT CHECKERS
=====================================================================================

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
    std::map<int, ClientManager* >::iterator it = clients.begin();

    while (it != clients.end()) {
        BaseSettings*		settings = &serverSettings;
        LocationSettings* 	locationSettings = serverSettings.findLocation((it)->second->getRequest().getURI());

        if (locationSettings)
            settings = locationSettings;

        size_t timeoutValue = settings->getKeepaliveTimeout();

        if (it->second->isTimedout(timeoutValue)) {
            Logger::log(Logger::INFO, "Client with fd " + Logger::intToString(it->second->getSocket()) + 
                " has timed out, proceeding to disconnect", "Server::checkTimeouts");
            int clientSocketFD = it->first;
            eventManager->deregisterEvent(clientSocketFD, READ);
            std::map<int, ClientManager* >::iterator toErase = it; // Note 1
            it++;
            delete (toErase->second);
            clients.erase(toErase);  // Erase using the saved iterator
            close(clientSocketFD);
        }
        else
            it++;
    }
}

void Server::checkCgiTimeouts()
{
    std::map<int, CGIManager* >::iterator it = cgi.begin();

    while (it != cgi.end()) {

        int cgiPipeFD = it->first;
        if (it->second->isCgiTimedOut(CGI_TIMEOUT)) {
            Logger::log(Logger::INFO, "cgi process with fd " + Logger::intToString(cgiPipeFD) + 
                " has timed out, proceeding to disconnect", "Server::checkCgiTimeouts");
            kill(it->second->getChildPid(), SIGKILL);
            eventManager->deregisterEvent(cgiPipeFD, READ);
            handleInvalidRequest(it->second->getCgiClientSocketFD(), "504", "Gateway Timeout");
            std::map<int, CGIManager* >::iterator toErase = it;
            it++;
            delete toErase->second;
            cgi.erase(cgiPipeFD);
            close(cgiPipeFD);
        }
        else
            it++;
    }
}

/*
=====================================================================================
                                CLIENT REMOVAL
=====================================================================================
*/
void Server::removeBadClients(int clientSocketFD)
{  
    ClientManager* clientManager = clients[clientSocketFD];
    eventManager->deregisterEvent(clientSocketFD, READ);
    clients.erase(clientSocketFD);
    delete clientManager;

    Logger::log(Logger::INFO, "Client with fd " + Logger::intToString(clientSocketFD) + 
            " has exhibited abnormal activity and has been removed", "Server::removeBadClients");
}

/*
    NOTES:

        Note 1:
            This is just a remnant of the old code, but I will keep it because it's safer to do so. 
            Nevertheless, in ServerArena::manageReadEvent, we already check if the socket has already 
            been erased, so this if statement is likely not going to be triggered, at least I haven't 
            seen it, and can't think of a case where it could be.

        Note 2:
            .erase() doesn't need an iterator here because we are working with maps; for std::maps, we can erase an 
            element by its key because no duplicate keys are allowed. But for a vector, we can have duplicate values 
            and thus, the erase method (its version of the override) requires an iterator.

        Note 3:
            If a client disconnects while in the midst of recieving chunked responses (so they basically close the 
            tab or something while listening to an mp3 file), then the ResponseManager object that was allocated 
            to oversee that process never had the chance to get deleted the normal way, in the send related member 
            functions where if (responseManager->isFinished()) condition is satisified. Hence, to not leak in the 
            aforementioned scenario, we have to delete it here as well, in case there was a "hanging" responseManager 
            object.

        Note 4:
            Ok so EV_DELETE used in the deregistering of an event is a bit weird because of how 
            the manual explains it. But I guarantee you that my explanation is in fact how it behaves, 
            I've spent hours and hours trying to experiment till I figured it out. So the manual says 
            the following:

            EV_DELETE   Removes the event from the kqueue. Events which are attached to file 
                        descriptors are automatically deleted on the last close of the descriptor.

            What this means is that EV_DELETE will remove the event from the kqueue but when it does so, 
            that means it will no longer detect that event in the future! However, any pending events that 
            were already there on the kqueue, will still be processed! The second part of what it is saying, 
            is that if you simply close the file descriptor associated with a client's socket, then there is 
            no need to actually use EV_DELETE, yes, that's what they mean by automatic! All events associated 
            with that file descriptor are automatically deleted by just closing the file descriptor. Again 
            though, this means future events are no longer detected but, anything that was already on the 
            kqueue will still be processed! 

            So yes, in fact, this line eventManager->deregisterEvent(clientSocketFD, READ); in the function 
            below is useless, but I am just going to keep it there because why not, it feels safer to have it 
            lol. Nevertheless, the deregisterEvent function itself is NOT useless, in case you assumed that. 
            We actually need that fine-tuning because under normal circumstances, when a client simply requests 
            a page, I actually only want the Write event to be listened for while I send the response, but when 
            it's sent, I want to delete it off the kqeueue but I still want the Read event open! Nevertheless, when 
            I disconnect a client, who cares, we have to close the socket's fd anyways and that automatically deletes 
            all events.
*/
void Server::removeDisconnectedClients(int clientSocketFD)
{
    if (clients.find(clientSocketFD) == clients.end()) {
        Logger::log(Logger::WARN, "Already handled this EOF", "Server::removeDisconnectedClients");
        return ;  // Note 1
    }
    ClientManager* ClientManager = clients[clientSocketFD];
    eventManager->deregisterEvent(clientSocketFD, READ);
    clients.erase(clientSocketFD);  // Note 2
    Logger::log(Logger::INFO, "Client with fd " + Logger::intToString(clientSocketFD) + 
            " has disconnected", "Server::removeDisconnectedClients");
    delete(ClientManager);

    std::map<int, ResponseManager* >::iterator it = responses.find(clientSocketFD); // Note 3
    if (it != responses.end()) {
        delete it->second;
        responses.erase(clientSocketFD);
    }

    close(clientSocketFD); // Note 4
}

/*
=====================================================================================
                                ERROR HANDLING
=====================================================================================

    ALL THE ERROR HANDLING FUNCTIONS BELOW DO NOT GET SENT TO ANY OF THE FOLLOWING:
        * processGetRequest
        * processPostRequest
        * processHeadRequest
        * processDeleteRequest
    
    and thus there is no call to this function:
        HTTPResponse response = responseGenerator.handleRequest(request);
    instead, they all go directly to:
        HTTPResponse response;
        response.buildDefaultErrorResponse("414", "URI Too Long");
*/
void Server::handleExcessHeaders(int clientSocketFD)
{
    Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(clientSocketFD)
                    + " sent a header that exceed the permissible limit -> " 
                    + Logger::intToString(MAX_HEADER_SIZE) + " bytes", 
                    "Server::handleExcessHeader");
    
    removeBadClients(clientSocketFD);

    HTTPResponse response;
    response.buildDefaultErrorResponse("431", "Request Headers Fields Too Large");
    ResponseManager* responseManager = new ResponseManager(response.generateResponse(), true);
    responses[clientSocketFD] =  responseManager;

    eventManager->registerEvent(clientSocketFD, WRITE);
}

void Server::handleExcessURI(int clientSocketFD)
{
    Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(clientSocketFD)
                    + " sent a URI that exceeded the permissible limit -> " 
                    + Logger::intToString(MAX_URI_SIZE) + " bytes", 
                    "Server::handleExcessURI");
    
    removeBadClients(clientSocketFD);

    HTTPResponse response;
    response.buildDefaultErrorResponse("414", "URI Too Long");
    ResponseManager* responseManager = new ResponseManager(response.generateResponse(), true);
    responses[clientSocketFD] =  responseManager;

    eventManager->registerEvent(clientSocketFD, WRITE);
}

void Server::handleInvalidGetRequest(int clientSocketFD)
{
    Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(clientSocketFD)
                + " made a GET request that includes a body", 
                "Server::handleInvalidGetRequest");

    removeBadClients(clientSocketFD);

    HTTPResponse response;
    response.buildDefaultErrorResponse("400", "Bad Request");
    ResponseManager* responseManager = new ResponseManager(response.generateResponse(), true);
    responses[clientSocketFD] =  responseManager;

    eventManager->registerEvent(clientSocketFD, WRITE);
}

/*
    NOTES:

        Note 1: This gets triggered when a client has requested a cgi script and the client timesout 
                while the script is still running, AND then, the script itself then times out (NOT 
                because the client timed-out, it's just a script that takes a long time). In that 
                case, since the client timedout, we wont be able to send them the 504 error_page.
*/
void Server::handleInvalidRequest(int clientSocketFD, std::string statusCode, std::string reasonPhrase)
{
    if (clients.find(clientSocketFD) == clients.end()) {
        Logger::log(Logger::WARN, "Client has already disconnected, unable to send error page", "Server::handleInvalidGetRequest");
        return ;  // Note 1
    }

    Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(clientSocketFD)
                + " made an invalid request", 
                "Server::handleInvalidRequest");

    removeBadClients(clientSocketFD);

    HTTPResponse response;
    response.buildDefaultErrorResponse(statusCode, reasonPhrase);
    ResponseManager* responseManager = new ResponseManager(response.generateResponse(), true);
    responses[clientSocketFD] =  responseManager;

    eventManager->registerEvent(clientSocketFD, WRITE);
}

/*
=====================================================================================
                                     GETTERS
=====================================================================================
*/
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

ServerSettings&	Server::getServerSettings()
{
    return (serverSettings);
}

std::map<int, ClientManager* >&	Server::getClients()
{
    return (clients);
}

std::map<int, ResponseManager* >& Server::getResponses()
{
    return (responses);
}

std::map<int, CGIManager* >& Server::getCgiMap()
{
    return (cgi);
}
