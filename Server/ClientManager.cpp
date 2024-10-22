#include "ClientManager.hpp"

ClientManager::ClientManager(int fd, const std::string &clientIpAddr)
    : _fd(fd), _clientIpAddr(clientIpAddr), _requestCount(0), _areHeaderComplete(false), _isBodyComplete(false)
{
    // this->_lastRequestTime = std::chrono::steady_clock::now();
}

ClientManager::~ClientManager()
{
    if (_requestBodyFile.is_open())
        _requestBodyFile.close();
}

void ClientManager::resetClientManager()
{
    _requestHeaders.clear();
    _requestBody.clear();
    if (_requestBodyFile.is_open())
        _requestBodyFile.close();
    if (_requestBodyFilePath.size() > 0)
    {
        remove(_requestBodyFilePath.c_str());
    }
    _requestBodySize = 0;
    _requestBodyFilePath.clear();
    _areHeaderComplete = false;
    _isBodyComplete = false;
}

void ClientManager::updateLastRequestTime()
{
    // _lastRequestTime = std::chrono::steady_clock::now();
}

void ClientManager::incrementRequestCount()
{
    _requestCount++;
}

void ClientManager::processIncomingData(Server &server, const char *buffer, size_t bytesRead)
{
    if (!_areHeaderComplete)
        processHeaders(server, buffer, bytesRead);
    else
        processBody(server, buffer, bytesRead);
}

void ClientManager::processHeaders(Server &server, const char *buffer, size_t bytesRead)
{
    // Mousa

    // _requestHeaders.append(buffer, bytesRead);
    // if (_requestHeaders.size() > MAX_REQUEST_HEADERS_SIZE)
    // {
    //     server.handleHeaderSizeExceeded(_fd);
    //     return;
    // }
    // if (_requestHeaders.find("\r\n\r\n") != std::string::npos)
    // {
    //     _areHeaderComplete = true;
    //     parseHeaders(server);
    // }
}

void ClientManager::processBody(Server &server, const char *buffer, size_t bytesRead)
{
    // Mousa

    // if (_requestBodyFilePath.size() > 0)
    // {
    //     _requestBodyFile.write(buffer, bytesRead);
    //     _requestBodySize += bytesRead;
    //     if (_requestBodySize > MAX_REQUEST_BODY_SIZE)
    //     {
    //         server.handleBodySizeExceeded(_fd);
    //         return;
    //     }
    //     if (_requestBodySize == _request.getBodySize())
    //     {
    //         _isBodyComplete = true;
    //         _requestBodyFile.close();
    //         if (_request.getMethod() == "POST")
    //             handlePostRequest(server);
    //         else
    //             server.handleBadRequest(_fd);
    //     }
    // }
    // else
    // {
    //     _requestBody.append(buffer, bytesRead);
    //     if (_requestBody.size() > MAX_REQUEST_BODY_SIZE)
    //     {
    //         server.handleBodySizeExceeded(_fd);
    //         return;
    //     }
    //     if (_requestBody.size() == _request.getBodySize())
    //     {
    //         _isBodyComplete = true;
    //         if (_request.getMethod() == "POST")
    //             handlePostRequest(server);
    //         else
    //             server.handleBadRequest(_fd);
    //     }
    // }
}

void ClientManager::parseHeaders(Server &server)
{
    // std::istringstream iss(_requestHeaders);
    // std::string line;
    // std::getline(iss, line);
    // std::istringstream firstLine(line);
    // std::string method;
    // std::string uri;
    // std::string version;
    // firstLine >> method >> uri >> version;
    // if (method == "GET")
    // {
    //     _request.setMethod("GET");
    //     _request.setUri(uri);
    //     _request.setVersion(version);
    //     handleGetRequest(server);
    // }
    // else if (method == "HEAD")
    // {
    //     _request.setMethod("HEAD");
    //     _request.setUri(uri);
    //     _request.setVersion(version);
    //     handleHeadRequest(server);
    // }
    // else if (method == "POST")
    // {
    //     _request.setMethod("POST");
    //     _request.setUri(uri);
    //     _request.setVersion(version);
    //     handlePostRequest(server);
    // }
    // else
    // {
    //     server.handleBadRequest(_fd);
    // }
}

void ClientManager::initializeBodyStorage(Server &server)
{
    // _requestBodyFilePath = server.getRequestBodyFilePath();
    // _requestBodyFile.open(_requestBodyFilePath, std::ios::out | std::ios::binary);
    // if (!_requestBodyFile.is_open())
    // {
    //     server.handleInternalError(_fd);
    //     return;
    // }
}

void ClientManager::handleGetRequest(Server &server)
{
    std::ostringstream logStream;
    logStream << "Received a 'GET' request for '" << _request.getURI() << "' from IP '"
              << _clientIpAddr << "', processing on socket descriptor " << _fd;
    server.handleGetRequest(_fd, _request);
}

void ClientManager::handleHeadRequest(Server &server)
{
    // std::ostringstream logStream;
    // logStream << "Received a 'HEAD' request for '" << _request.getUri() << "' from IP '"
    //           << _clientIpAddr << "', processing on socket descriptor " << _fd;
    // server.handleHeadRequest(_fd, _request);
}

void ClientManager::handlePostRequest(Server &server)
{
    // if (_request.getStatus() != 200)
    // {
    //     server.handleBadRequest(_fd);
    // }
    // else
    // {
    //     std::ostringstream logStream;
    //     logStream << "Received a 'POST' request for '" << _request.getUri() << "' from IP '"
    //               << _clientIpAddr << "', processing on socket descriptor " << _fd;
    //     server.handlePostRequest(_fd, _request);
    // }
}

int ClientManager::getFd() const
{
    return _fd;
}

const std::string &ClientManager::getClientIpAddr() const
{
    return _clientIpAddr;
}

int ClientManager::getRequestCount() const
{
    return _requestCount;
}

const std::string &ClientManager::getPostRequestFileName()
{
    // return _request.getPostRequestFileName();
}

bool ClientManager::isTimedOut(size_t keepalive_timeout) const
{
    // std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
    // return std::chrono::duration_cast<std::chrono::seconds>(now - _lastRequestTime).count() > keepalive_timeout;
    return false;
}

