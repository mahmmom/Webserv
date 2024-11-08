
#include "ClientManager.hpp"

ClientManager::ClientManager(int fd, const std::string &clientIpAddr)
	: fd(fd), clientAddress(clientIpAddr), requestNumber(0), areHeaderComplete(false), isBodyComplete(false)
{
	this->lastRequestTime = std::time(0);
}

ClientManager::~ClientManager()
{
	if (requestBodyFile.is_open())
		requestBodyFile.close();
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
	if (requestHeaders.size() > MAX_HEADER_SIZE)
	{
		server.handleExcessHeaders(fd);
		return;
	}
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

	this->request = HTTPRequest(requestHeaders,fd);

	if (request.getURI().size() > MAX_URI_SIZE)
	{
		server.handleExcessURI(fd);
		return;
	}

	if (request.getMethod() == "GET")
	{
		Logger::log(Logger::INFO, "Received a GET request for " + request.getURI() + 
		"from client with IP " + clientAddress + ", with fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		handleGetRequest(server);
	}
	else if (request.getMethod() == "HEAD")
	{
		Logger::log(Logger::INFO, "Received a HEAD request for " + request.getURI() + 
		"from client with IP " + clientAddress + ", with fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		// server.processHeadRequest(fd, request);
	}
	else if (request.getMethod() == "POST")
	{
		Logger::log(Logger::INFO, "Received a POST request for " + request.getURI() + 
		"from client with IP " + clientAddress + ", with fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		handlePostRequest(server);
	}
	else if (request.getMethod() == "DELETE")
	{
		Logger::log(Logger::INFO, "Received a DELETE request for " + request.getURI() + 
		"from client with IP " + clientAddress + ", with fd: " + Logger::intToString(fd),
			"ClientManager::parseHeaders");
		// server.processDeleteRequest(fd, request);
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
		// server.handleInvalidGetRequest(fd);
		Logger::log(Logger::WARN, "Client with socket FD: " + Logger::intToString(fd)
                + " made a GET request that includes a body", 
                "Server::handleInvalidGetRequest");
		request.setStatus(400);
	}
	// else
	server.processGetRequest(fd, request);
}

void	ClientManager::handlePostRequest(Server &server)
{
	// if (request.getStatus() != 200)
	// {
	// 	Logger::log(Logger::WARN, "Invalid POST request status for client with socket fd " + Logger::intToString(fd), "ClientManager::handlePostRequest");

	// 	std::map<int, std::string> reasonPhraseMap;
	// 	reasonPhraseMap[400] = "Bad Request";
	// 	reasonPhraseMap[411] = "Length Required";
	// 	reasonPhraseMap[501] = "Not Implemented";
	// 	reasonPhraseMap[505] = "HTTP Version Not Supported";
	
	// 	server.handleInvalidRequest(fd, intToString(request.getStatus()), reasonPhraseMap[request.getStatus()]);
	// 	return ;
	// }
	if (request.getHeader("transfer-encoding") == "chunked")
	{
		Logger::log(Logger::WARN, "Chunked Transfer-Encoding not supported for client with socket fd " + Logger::intToString(fd), "ClientManager::handlePostRequest");
		server.handleInvalidRequest(fd, "411", "Length Required");
		return ;
	}

	BaseSettings*		settings = &(server.getServerSettings());
	LocationSettings* 	locationSettings = server.getServerSettings().findLocation(request.getURI());

	if (locationSettings)
		settings = locationSettings;

	requestBodySize = stringToSizeT(request.getHeader("content-length"));
	if (requestBodySize > settings->getClientMaxBodySize())
	{
		Logger::log(Logger::WARN, "Body size of POST request exceeds client max body size for client with socket fd " + Logger::intToString(fd), "ClientManager::handlePostRequest");
		server.handleInvalidRequest(fd, "413", "Request Entity Too Large");
		request.setStatus(413);
		return ;
	}
	initializeBodyStorage(server);
}

void	ClientManager::initializeBodyStorage(Server &server)
{
	std::string filename = "post_body_" + Logger::intToString(std::chrono::system_clock::now().time_since_epoch().count()) + "_" + Logger::intToString(fd) + ".tmp";
	requestBodyFilePath = TEMP_FILE_DIRECTORY + filename;

	requestBodyFile.open(requestBodyFilePath.c_str(), std::ios::out | std::ios::binary);
	if (!requestBodyFile.is_open())
	{
		Logger::log(Logger::ERROR, "Failed to open temporary file for storing POST body for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		server.handleInvalidRequest(fd, "500", "Internal Server Error");
		return;
	}

	Logger::log(Logger::DEBUG, "Temporary file for POST body created: " + requestBodyFilePath, "ClientManager::initializeBodyStorage");
	
	if (requestBody.size() == requestBodySize)
	{
		Logger::log(Logger::DEBUG, "POST request body is complete from the first read for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		requestBodyFile << requestBody;
		requestBodyFile.close();
		isBodyComplete = true;
		server.processPostRequest(fd, request);
	}
	else if (requestBody.size() > requestBodySize)
	{
		Logger::log(Logger::WARN, "POST request body exceeds the declared content length for client with socket fd " + Logger::intToString(fd), "ClientManager::initializeBodyStorage");
		requestBodyFile.close();
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

	size_t remainingBodySize = requestBodySize - requestBodyFile.tellp();
	if (bytesRead > remainingBodySize)
	{
		Logger::log(Logger::WARN, "POST request body exceeds declared content length for client with socket fd " + Logger::intToString(fd), "ClientManager::processBody");
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
