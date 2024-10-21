
#include "ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator(ServerSettings serverSettings) : 
	serverSettings(serverSettings) {}

void	ResponseGenerator::handleReturnDirective() {}

std::string ResponseGenerator::intToString(const int intValue)
{
	std::stringstream ss;

	ss << intValue;
	if (ss.fail() || !ss.eof())
		return (""); // probably issue a warning via the logger
	return (ss.str());
}

bool ResponseGenerator::isFile(const std::string& requestURI)
{
	struct stat pathStat;

	stat(requestURI.c_str(), &pathStat);
	return (S_ISDIR(pathStat.st_mode));
}

bool ResponseGenerator::isDirectory(const std::string& requestURI)
{
	struct stat pathStat;

	stat(requestURI.c_str(), &pathStat);
	return (S_ISREG(pathStat.st_mode));
}

HTTPResponse ResponseGenerator::serveError(int statusCode, BaseSettings* settings)
{
	if (settings->getErrorPages().find(statusCode) != settings->getErrorPages().end())
		; // return serveErrorPage

	HTTPResponse response;

	response.setVersion("HTTP/1.1");
	response.setStatusCode(intToString(statusCode));
	response.setHeaders("Server", "Ranchero");
	// response.setReasonPhrase();
	response.setHeaders("Content-Type", "text/html");

	std::string message = intToString(statusCode) /* + reasonPhrase */;
	std::string body = "<html>"
						"<head><title>" + message +
						"</title></head>"
						"<body>"
						"<center><h1>" + message + "</h1></center>"
						"<hr><center>nginx/1.27.1</center>"
						"</body>"
						"</html>";
}

HTTPResponse ResponseGenerator::redirector(HTTPRequest& request, const std::string& URL)
{
	HTTPResponse 	response;
	std::string		completeURL;

	completeURL = "http://" + request.getHeader("host") + URL;
	response.setVersion("HTTP/1.1");
	response.setStatusCode("301");
	response.setReasonPhrase("Moved permanently");
	response.setHeaders("Content-Type", "text/html");
	response.setHeaders("Content-Length", "0");
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Location", completeURL);
	response.setHeaders("Connection", "keep-alive");

	return (response);
}

HTTPResponse ResponseGenerator::handleDirectory(HTTPRequest& request, BaseSettings* settings)
{
	if (request.getURI()[request.getURI().size() - 1] != '/')
		return (redirector(request, request.getURI() + "/")); // redirect to path + "/"

	std::string	path;
	path = serverSettings.getRoot() + request.getURI();
	std::vector<std::string>::iterator it;
	for (it = serverSettings.getIndex().begin(); it != serverSettings.getIndex().end(); it++) {
		std::string indexPath;
		if (it == serverSettings.getIndex().end() - 1 && ((*it)[0] == '/'))
			indexPath = serverSettings.getRoot() + (*it);
		else
			indexPath = path + (*it);
		if (isFile(indexPath))
			return (serveFile(request, indexPath));
		if ((indexPath[indexPath.size() - 1] == '/') || isDirectory(indexPath))
			return (redirector(request, (*it) + "/")); // redirect to that path + "/"
	}
	// return (handleAutoIndex());
}

HTTPResponse ResponseGenerator::serveFile(HTTPRequest& request, std::string& path)
{
	;
}

HTTPResponse ResponseGenerator::serveRequest(HTTPRequest& request, BaseSettings* settings)
{
	std::string	path;

	path = serverSettings.getRoot() + request.getURI();
	
	if ((path[path.size() - 1] == '/') || isDirectory(path))
		return (handleDirectory(request, settings));
	if (isFile(path))
		return (serveFile(request, path));
	// return server error
}

HTTPResponse ResponseGenerator::handleGetRequest(HTTPRequest& request)
{
	if (serverSettings.getReturnDirective().getEnabled())
		;

	BaseSettings*		settings = &serverSettings;
	LocationSettings* 	locationSettings = serverSettings.findLocation(request.getURI());

	if (locationSettings) {
		settings = locationSettings;
		if (!locationSettings->isMethodAllowed(request.getMethod()))
			; // return serve error
	}

	if (settings->getReturnDirective().getEnabled())
		;
	return (serveRequest(request, settings));
}

HTTPResponse ResponseGenerator::handleRequest(HTTPRequest& request)
{
	if (request.getMethod() == "GET")
		return (handleGetRequest(request));
	else if (request.getMethod() == "HEAD ")
		;
	else if (request.getMethod() == "POST")
		;
	else if (request.getMethod() == "DELETE")
		;

	HTTPResponse deleteLater;
	return deleteLater;
}
