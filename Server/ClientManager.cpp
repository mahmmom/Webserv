
#include "ClientManager.hpp"

ClientManager::ClientManager(int fd, const std::string &clientIpAddr)
	: fd(fd), clientAddress(clientIpAddr), requestNumber(0), areHeaderComplete(false), isBodyComplete(false),
		isChunkedTransfer(false), requestManager(NULL)
{
	this->lastRequestTime = std::time(0);
}

ClientManager::~ClientManager()
{
	delete (requestManager);
}

void	ClientManager::resetClientManager()
{
	requestHeaders.clear();
	requestBody.clear();
	requestBodySize = 0;
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

/*
	NOTES

		Note 1:	If we go off the way we designed the HTTPRequest class, then in the request line is missing a token, 
				meaning that it did not have all three of nethod, uri, & version, then we do not even proceed to 
				fill out the request line (we just return false and set status to 400). So in that case, if 
				someone did something like POST HTTP/1.1; that's only 2 tokens, the uri is missining. Hence, the 
				request method would indeed be empty cause it was never assigned something. That's why we are 
				checking that in the else clause. Of course, if someone did something like TRACE /example HTTP/1.1, 
				that would also be picked up by this clause but instead assigned a status code of 501.
*/
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
	else // Note 1
	{
		Logger::log(Logger::WARN, "An HTTP request containing issues within the request-line was "
						"made by client with IP " + clientAddress + ", on fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");

		if (request.getStatus() == 400)
			server.handleInvalidRequest(fd, "400", "Bad Request");
		else if (request.getStatus() == 505)
			server.handleInvalidRequest(fd, "505", "HTTP Version Not Supported");
		else
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

	BaseSettings*		settings = &(server.getServerSettings());
	LocationSettings* 	locationSettings = server.getServerSettings().findLocation(request.getURI());

	if (locationSettings)
		settings = locationSettings;

	if (request.getStatus() == 411 || request.getStatus() == 400)
	{
		if (request.getStatus() == 411)
			server.handleInvalidRequest(fd, "411", "Length Required");
		else
			server.handleInvalidRequest(fd, "400", "Bad Request");
		return ;
	}

    if (request.getHeader("transfer-encoding") == "chunked")
	{
        isChunkedTransfer = true;
        initializeBodyStorage(server, settings);
        return ;
    }

	requestBodySize = stringToSizeT(request.getHeader("content-length"));
	if (requestBodySize > settings->getClientMaxBodySize())
	{
		// std::cout << "size is " << request.getHeader("content-length") << std::endl;
		Logger::log(Logger::WARN, "Body size of POST request exceeds client max body size for client with socket fd " + Logger::intToString(fd), "ClientManager::handlePostRequest");
		server.handleInvalidRequest(fd, "413", "Request Entity Too Large");
		return ;
	}
	initializeBodyStorage(server, settings);
}

void	ClientManager::initializeBodyStorage(Server &server, BaseSettings* settings)
{
	if (isChunkedTransfer) {
		delete requestManager; // Delete any exisitng Manager (though chances are resetClientState already handled that)
		requestManager = new RequestManager(settings->getClientMaxBodySize());

		// Handle the case where we already have the complete request
        if (!requestBody.empty()) {
			if (!requestManager->processChunkedData(requestBody)) {
				if (requestManager->hasExceededMaxSize()) {
					Logger::log(Logger::WARN, "Body size of chunked POST request exceeds client max body size for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");
					server.handleInvalidRequest(fd, "413", "Request Entity Too Large");
				} 
				else {
					Logger::log(Logger::ERROR, "Failed to process initial chunked data", 
						"ClientManager::processBody");
					server.handleInvalidRequest(fd, "400", "Bad Request");
				}
                return (void());
            }
        }

        // Check if it's a complete empty chunked request (0\r\n\r\n)
        if (requestBody.find("0\r\n\r\n") != std::string::npos || 
            	requestManager->isRequestComplete()) {
            Logger::log(Logger::DEBUG, "Received complete empty chunked request", 
                "ClientManager::initializeBodyStorage");

            isBodyComplete = true;

			request.setBody(requestManager->getBuffer());
			// request.accurateDebugger();

            server.processPostRequest(fd, request);
            return (void());
        }
        return (void());
    }

	if (requestBody.size() == requestBodySize)
	{
		Logger::log(Logger::DEBUG, "POST request body is complete from the first read for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		
		requestBuffer = requestBody;
		request.setBody(requestBuffer);
		isBodyComplete = true;
		server.processPostRequest(fd, request);
	}
	else if (requestBody.size() > requestBodySize)
	{
		Logger::log(Logger::WARN, "POST request body exceeds the declared content length for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		server.handleInvalidRequest(fd, "400", "Bad Request");
	}
	else
	{
		Logger::log(Logger::DEBUG, "POST request body is incomplete from the first read for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		requestBuffer = requestBody;
	}
}

void	ClientManager::processBody(Server &server, const char *buffer, size_t bytesRead)
{
	Logger::log(Logger::DEBUG, "Processing body of POST request for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");

    if (isChunkedTransfer && requestManager != NULL) 
	{
        std::string chunk(buffer, bytesRead);
		
		if (!requestManager->processChunkedData(chunk)) {
			if (requestManager->hasExceededMaxSize()) {
				Logger::log(Logger::WARN, "Body size of chunked POST request exceeds client max body size for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");
				server.handleInvalidRequest(fd, "413", "Request Entity Too Large");
			} 
			else {
				Logger::log(Logger::ERROR, "Failed to process chunked data", 
					"ClientManager::processBody");
				server.handleInvalidRequest(fd, "400", "Bad Request");
			}
			return (void());
		}

        if (requestManager->isRequestComplete()) {
			Logger::log(Logger::DEBUG, "POST chunked request body is complete for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");
			request.setBody(requestManager->getBuffer());
            isBodyComplete = true;
            server.processPostRequest(fd, request);
        }
        return (void());
    }

	size_t remainingBodySize = requestBodySize - requestBuffer.size();
	if (bytesRead >= remainingBodySize)
	{
		Logger::log(Logger::DEBUG, "POST request body is complete for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");
		requestBuffer.append(buffer, remainingBodySize);
		request.setBody(requestBuffer);
		isBodyComplete = true;
		server.processPostRequest(fd, request);
	}
	else
	{
		Logger::log(Logger::DEBUG, "Appending to POST request body for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");
		requestBuffer.append(buffer, bytesRead);
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
