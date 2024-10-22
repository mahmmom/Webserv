
#include "HTTPResponse.hpp"

std::string	HTTPResponse::generateResponse()
{
	std::string	response;

	response = version + " " + statusCode + " " + reasonPhrase + "\r\n";
	std::map<std::string, std::string>::iterator it;
	for (it = headers.begin(); it != headers.end(); it++) {
		response += (it->first) + ":";
		response += " " + (it->second) + "\r\n";
	}
	response += "\r\n";
	if (!body.empty())
		response += body;
	
	return (response);
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

const std::string& HTTPResponse::getReasonPhrase()
{
	return (reasonPhrase);
}
