
#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse() : type(CompactResponse) {}

std::string& HTTPResponse::generateResponse()
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

/*
	HTTP/1.1 404 Not Found
	Server: nginx/1.27.1
	Date: Thu, 31 Oct 2024 22:52:16 GMT
	Content-Type: text/html
	Content-Length: 153
	Connection: close

	<html>
	<head><title>404 Not Found</title></head>
	<body>
	<center><h1>404 Not Found</h1></center>
	<hr><center>nginx/1.27.1</center>
	</body>
	</html>
*/
void HTTPResponse::buildDefaultErrorResponse(std::string statusCode, std::string reasonPhrase)
{
	version = "HTTP/1.1";
	this->statusCode = statusCode;
	this->reasonPhrase = reasonPhrase;
	headers["Server"] = "Ranchero";
	headers["Connection"] = "close";
	headers["Content-Type"] = "text/html";

	std::string body =	"<html>"
						"<head><title>" + statusCode + " " + reasonPhrase + "</title></head>"
						"<body>"
						"<center><h1>404 Not Found</h1></center>"
						"<hr><center>nginx/1.27.1</center>"
						"</body>"
						"</html>";
	this->body = body;
	headers["Content-Length"] = body.length();
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
