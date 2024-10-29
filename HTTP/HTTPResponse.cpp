
#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse() : type(CompactResponse) {}

std::string&	HTTPResponse::generateResponse()
{
	fullResponse = version + " " + statusCode + " " + reasonPhrase + "\r\n";
	std::map<std::string, std::string>::iterator it;
	for (it = headers.begin(); it != headers.end(); it++) {
		fullResponse += (it->first) + ":";
		fullResponse += " " + (it->second) + "\r\n";
	}
	fullResponse += "\r\n";
	if (!body.empty())
		fullResponse += body;
	
	return (fullResponse);
}

void HTTPResponse::setFilePath(const std::string& filePath)
{
	this->filePath = filePath;
}

void HTTPResponse::setFileSize(const long long& fileSize)
{
	this->fileSize = fileSize;
}

void	HTTPResponse::setType(const ResponseType type)
{
	this->type = type;
}

void	HTTPResponse::setVersion(const std::string& version)
{
	this->version = version;
}

void	HTTPResponse::setStatusCode(const std::string& statusCode)
{
	this->statusCode = statusCode;
}

void	HTTPResponse::setReasonPhrase(const std::string& reasonPhrase)
{
	this->reasonPhrase = reasonPhrase;
}

void	HTTPResponse::setHeaders(const std::string& headerName, const std::string& headerValue)
{
	this->headers[headerName] = headerValue;
}

void	HTTPResponse::setBody(const std::string& body)
{
	this->body = body;
}

const std::string& HTTPResponse::getFilePath()
{
	return (filePath);
}

const long long& HTTPResponse::getFileSize()
{
	return (fileSize);
}

const ResponseType& HTTPResponse::getType()
{
	return (type);
}

const std::string& HTTPResponse::getReasonPhrase()
{
	return (reasonPhrase);
}
