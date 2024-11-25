
#include "ClientManager.hpp"

ClientManager::ClientManager(int fd, const std::string &clientIpAddr)
	: fd(fd), clientAddress(clientIpAddr), requestNumber(0), areHeaderComplete(false), isBodyComplete(false),
		isChunkedTransfer(false), requestManager(NULL)
{
	this->lastRequestTime = std::time(0);
}

ClientManager::~ClientManager()
{
	if (requestBodyFile.is_open())
		requestBodyFile.close();
	delete (requestManager);
}

void	ClientManager::resetClientManager()
{
	requestHeaders.clear();
	requestBody.clear();
	if (requestBodyFile.is_open())
		requestBodyFile.close();
	if (requestBodyFilePath.size())
		remove(requestBodyFilePath.c_str());
	requestBodySize = 0;
	requestBodyFilePath.clear();
	areHeaderComplete = false;
	isBodyComplete = false;

	isChunkedTransfer = false;
	delete requestManager;
    requestManager = NULL;
}

void	ClientManager::updateLastRequestTime()
{
	lastRequestTime = std::time(0);
}

void	ClientManager::incrementRequestsNumber()
{
	requestNumber++;
}

void	ClientManager::processIncomingData(Server &server, const char *buffer, size_t bytesRead)
{
	if (!areHeaderComplete)
		processHeaders(server, buffer, bytesRead);
	else
		processBody(server, buffer, bytesRead);
}

void	ClientManager::processHeaders(Server &server, const char *buffer, size_t bytesRead)
{
	requestHeaders.append(buffer, bytesRead);
	if (requestHeaders.find("\r\n\r\n") != std::string::npos)
	{
		areHeaderComplete = true;
		Logger::log(Logger::DEBUG, "Headers completed for fd " + Logger::intToString(fd), "ClientManager::processHeaders");
		parseHeaders(server);
	}
	else
		Logger::log(Logger::DEBUG, "Received partial headers for fd " + Logger::intToString(fd), "ClientManager::processHeaders");
}

void	ClientManager::parseHeaders(Server &server)
{
	size_t endOfHeaders = requestHeaders.find("\r\n\r\n") + 4;
	requestBody = requestHeaders.substr(endOfHeaders);
	requestHeaders.resize(endOfHeaders);

	if (requestHeaders.size() > MAX_HEADER_SIZE)
	{
		server.handleExcessHeaders(fd);
		return;
	}

	request = HTTPRequest(requestHeaders, fd);

	// request.setBody(requestBody);
	// request.accurateDebugger();

	if (request.getURI().size() > MAX_URI_SIZE)
	{
		server.handleExcessURI(fd);
		return;
	}

	if (request.getMethod() == "GET")
	{
		Logger::log(Logger::INFO, "Received a GET request for " + request.getURI() + 
		" from client with IP " + clientAddress + ", on fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		handleGetRequest(server);
	}
	else if (request.getMethod() == "HEAD")
	{
		Logger::log(Logger::INFO, "Received a HEAD request for " + request.getURI() + 
		" from client with IP " + clientAddress + ", on fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		handleHeadRequest(server);
	}
	else if (request.getMethod() == "DELETE")
	{
		Logger::log(Logger::INFO, "Received a DELETE request for " + request.getURI() + 
		" from client with IP " + clientAddress + ", on fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		handleDeleteRequest(server);
	}
	else if (request.getMethod() == "POST")
	{
		Logger::log(Logger::INFO, "Received a POST request for " + request.getURI() + 
		" from client with IP " + clientAddress + ", on fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		handlePostRequest(server);
	}
	else
	{
		Logger::log(Logger::WARN, "Unsupported HTTP method for fd " + Logger::intToString(fd), "ClientManager::parseHeaders");
		server.handleInvalidRequest(fd, "501", "Not Implemented");
	}
}

void	ClientManager::handleGetRequest(Server &server)
{
	if (!requestBody.empty() || !request.getHeader("content-length").empty() || request.getHeader("transfer-encoding") == "chunked") {
		Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(fd)
                + " made a GET request that includes a body and/or contains body-related"
				+ " headers", 
                "Server::handleGetRequest");
		request.setStatus(400);
	}
	server.processGetRequest(fd, request);
}

void	ClientManager::handleHeadRequest(Server &server)
{
	if (!requestBody.empty() || !request.getHeader("content-length").empty() || request.getHeader("transfer-encoding") == "chunked") {
		Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(fd)
                + " made a HEAD request that includes a body and/or contains body-related"
				+ " headers", 
                "Server::handleHeadRequest");
		request.setStatus(400);
	}
	server.processGetRequest(fd, request);
}

void	ClientManager::handleDeleteRequest(Server &server)
{
	if (!requestBody.empty() || !request.getHeader("content-length").empty() || request.getHeader("transfer-encoding") == "chunked") {
		Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(fd)
                + " made a DELETE request that includes a body and/or contains body-related"
				+ " headers", 
                "Server::handleDeleteRequest");
		request.setStatus(400);
	}
	server.processDeleteRequest(fd, request);
}

void	ClientManager::handlePostRequest(Server &server)
{
	// if (request.getHeader("transfer-encoding") == "chunked")
	// {
	// 	Logger::log(Logger::WARN, "Chunked Transfer-Encoding not supported for client with socket fd " + Logger::intToString(fd), "ClientManager::handlePostRequest");
	// 	server.handleInvalidRequest(fd, "405", "Method Not Allowed");
	// 	return ;
	// }

	// // For the tester
	// if (requestBody.empty()) {
	// 	Logger::log(Logger::WARN, "POST request with no body " + Logger::intToString(fd), "ClientManager::handlePostRequest");
	// 	server.handleInvalidRequest(fd, "405", "Method Not Allowed");
	// 	return ;
	// }

	BaseSettings*		settings = &(server.getServerSettings());
	LocationSettings* 	locationSettings = server.getServerSettings().findLocation(request.getURI());

	if (locationSettings)
		settings = locationSettings;

    if (request.getHeader("transfer-encoding") == "chunked") {
        isChunkedTransfer = true;
        initializeBodyStorage(server);
        return;
    }

	requestBodySize = stringToSizeT(request.getHeader("content-length"));
	if (requestBodySize > settings->getClientMaxBodySize())
	{
		Logger::log(Logger::WARN, "Body size of POST request exceeds client max body size for client with socket fd " + Logger::intToString(fd), "ClientManager::handlePostRequest");
		server.handleInvalidRequest(fd, "413", "Request Entity Too Large");
		return ;
	}
	initializeBodyStorage(server);
}

void	ClientManager::initializeBodyStorage(Server &server)
{
	std::string filename;
	{
		std::ostringstream oss;
		oss << "post_body_" << Logger::intToString(static_cast<int>(time(0))) << "_" << Logger::intToString(fd) << ".tmp";
		filename = oss.str();
	}
	requestBodyFilePath = TEMP_FILE_DIRECTORY + filename;

	requestBodyFile.open(requestBodyFilePath.c_str(), std::ios::out | std::ios::binary);
	if (!requestBodyFile.is_open())
	{
		Logger::log(Logger::ERROR, "Failed to open temporary file for storing POST body for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		server.handleInvalidRequest(fd, "500", "Internal Server Error");
		return (void());
	}

	Logger::log(Logger::DEBUG, "Temporary file for POST body created: " + requestBodyFilePath, "ClientManager::initializeBodyStorage");

	if (isChunkedTransfer) {
		delete requestManager; // Delete any exisitng Manager (though chances are resetClientState already handled that)
		requestManager = new RequestManager(&requestBodyFile);
		
		/*
			if (!requestBody.empty()) {
				if (!requestManager->processChunkedData(requestBody)) {
					Logger::log(Logger::ERROR, "Failed to process initial chunked data", 
						"ClientManager::initializeBodyStorage");
					server.handleInvalidRequest(fd, "400", "Bad Request");
					return (void());
				}
				if (requestManager->isRequestComplete()) {
					requestBodyFile.close();
					isBodyComplete = true;
					server.processPostRequest(fd, request);
				}
			}
		*/

		// Handle the case where we already have the complete request
        if (!requestBody.empty()) {
            if (!requestManager->processChunkedData(requestBody)) {
                Logger::log(Logger::ERROR, "Failed to process initial chunked data", 
                    "ClientManager::initializeBodyStorage");
                server.handleInvalidRequest(fd, "400", "Bad Request");
                return (void());
            }
        }
        
        // Check if it's a complete empty chunked request (0\r\n\r\n)
        if (requestBody.find("0\r\n\r\n") != std::string::npos || 
            	requestManager->isRequestComplete()) {
            Logger::log(Logger::DEBUG, "Received complete empty chunked request", 
                "ClientManager::initializeBodyStorage");
            requestBodyFile.close();
            isBodyComplete = true;

			// request.setBody(requestManager->getOutputBuffer());
			// request.accurateDebugger();

            server.processPostRequest(fd, request);
            return (void());
        }
        return (void());
    }

	if (requestBody.size() == requestBodySize)
	{
		Logger::log(Logger::DEBUG, "POST request body is complete from the first read for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		
		requestBodyFile << requestBody;
		request.setBody(requestBody);

		requestBodyFile.close();
		isBodyComplete = true;
		server.processPostRequest(fd, request);
	}
	else if (requestBody.size() > requestBodySize)
	{
		Logger::log(Logger::WARN, "POST request body exceeds the declared content length for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		requestBodyFile.close();
		remove(requestBodyFilePath.c_str());
		server.handleInvalidRequest(fd, "400", "Bad Request");
	}
	else
	{
		Logger::log(Logger::DEBUG, "POST request body is incomplete from the first read for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		requestBodyFile << requestBody;
	}
}

void	ClientManager::processBody(Server &server, const char *buffer, size_t bytesRead)
{
	Logger::log(Logger::DEBUG, "Processing body of POST request for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");

    if (isChunkedTransfer && requestManager != NULL) {
        std::string chunk(buffer, bytesRead);
        if (!requestManager->processChunkedData(chunk)) {
            Logger::log(Logger::ERROR, "Failed to process chunked data", 
                "ClientManager::processBody");
            server.handleInvalidRequest(fd, "400", "Bad Request");
            return (void());
        }
        
        if (requestManager->isRequestComplete()) {
			Logger::log(Logger::DEBUG, "POST chunked request body is complete for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");

			request.setBody(requestManager->getOutputBuffer());

			requestBodyFile.close();
            isBodyComplete = true;
            server.processPostRequest(fd, request);
        }
        return (void());
    }

	size_t remainingBodySize = requestBodySize - requestBodyFile.tellp();
	if (bytesRead > remainingBodySize)
	{
		requestBodyFile.write(buffer, remainingBodySize);
		requestBodyFile.close();
		isBodyComplete = true;
		server.processPostRequest(fd, request);
		// server.processPostRequest(fd, request, true);
		// server.removeBadClients(fd);
	}
	else if (bytesRead == remainingBodySize)
	{
		Logger::log(Logger::DEBUG, "POST request body is complete for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");

		std::cout << " the boj is" << requestManager->getOutputBuffer() << std::endl;
		request.setBody(requestManager->getOutputBuffer());

		requestBodyFile.write(buffer, bytesRead);
		requestBodyFile.close();
		isBodyComplete = true;
		server.processPostRequest(fd, request);
	}
	else
	{
		Logger::log(Logger::DEBUG, "Appending to POST request body for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");
		requestBodyFile.write(buffer, bytesRead);
	}
}

int		ClientManager::getSocket() const
{
	return fd;
}

const std::string& ClientManager::getClientAddress() const
{
	return clientAddress;
}

int		ClientManager::getRequestCount() const
{
	return requestNumber;
}

const std::string& ClientManager::getPostRequestFileName()
{
	return (requestBodyFilePath);
}

const HTTPRequest&	ClientManager::getRequest()
{
	return (request);
}

const RequestManager* ClientManager::getRequestManager()
{
	return (requestManager);
}

bool	ClientManager::isTimedout(size_t keepaliveTimeout) const
{
	std::time_t now = std::time(0);
    std::time_t elapsedTime = now - lastRequestTime;

    bool timedOut = elapsedTime > static_cast<std::time_t>(keepaliveTimeout);
    return (timedOut);
}

std::string ClientManager::intToString(int number) 
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

size_t ClientManager::stringToSizeT(const std::string str)
{
    std::stringstream ss(str);
    size_t result;
    ss >> result;
    return result;
}
