
#include "HTTPResponse.hpp"

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
