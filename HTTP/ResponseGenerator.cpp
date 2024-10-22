
#include "ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator(ServerSettings serverSettings) : serverSettings(serverSettings) 
{
	reasonPhraseMap[301] = "Moved Permanently";
	reasonPhraseMap[403] = "Forbidden";
	reasonPhraseMap[404] = "Not Found";
	reasonPhraseMap[500] = "Internal Server Error";
}

void	ResponseGenerator::handleReturnDirective() {}

std::string ResponseGenerator::intToString(const int intValue)
{
	std::stringstream ss;

	ss << intValue;
	if (ss.fail())
		return (""); // probably issue a warning via the logger
	return (ss.str());
}

bool ResponseGenerator::isFile(const std::string& requestURI)
{
	struct stat pathStat;

	stat(requestURI.c_str(), &pathStat);
	return (S_ISREG(pathStat.st_mode));
}

bool ResponseGenerator::isDirectory(const std::string& requestURI)
{
	struct stat pathStat;

	stat(requestURI.c_str(), &pathStat);
	return (S_ISDIR(pathStat.st_mode));
}

/*
	GENERAL
		This function is responsible for handling internal redirects from 
		Nginx (Ranchero) where a new request is initiated from within the 
		server itself (a subrequest), and not an external HTTP request 
		coming from the client. This mainly to handle fallback URI's. A 
		few notes about these types of requests:
			* The client never sees this internal process
			* No new network connection is created
			* Headers from the original request are preserved
			* The client only receives the final response
*/
void ResponseGenerator::handleSubrequest(HTTPRequest& request, std::string& path)
{
	request.setURI(path);
	handleRequest(request);
}

/*
	Note 1: My reasonPhraseMap does not cover every status code in HTTP. So, since 
			I am the one setting the errors I handle here manually, that should be 
			fine for the most part. However, it could be that a CGI script that I 
			run might return a status code I did not handle. In that case, I wouldn't 
			have a reasonPhrase prepped for it and so instead, I would just send the 
			header containg the statusCode in the top line of the response. No need to 
			send an HTML file containing the status code and the reason phrase as it 
			usually does.
*/
HTTPResponse ResponseGenerator::serveError(int statusCode, BaseSettings* settings)
{
	if (settings->getErrorPages().find(statusCode) != settings->getErrorPages().end())
		; // return serveErrorPage

	HTTPResponse response;

	response.setVersion("HTTP/1.1");

	response.setStatusCode(intToString(statusCode));
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Content-Type", "text/plain");
	if (reasonPhraseMap.find(statusCode) != reasonPhraseMap.end()) // Note 1
	{
		response.setReasonPhrase(reasonPhraseMap[statusCode]);
		response.setHeaders("Content-Type", "text/html");
		std::string message = intToString(statusCode) + " " + reasonPhraseMap[statusCode];
		std::string body = 	"<html>"
							"<head><title>" + message +
							"</title></head>"
							"<body>"
							"<center><h1>" + message + "</h1></center>"
							"<hr><center>Ranchero</center>"
							"</body>"
							"</html>";
		response.setBody(body);
		response.setHeaders("Content-Length", intToString(body.size()));
	}
	else
		response.setReasonPhrase("Uknown Status");
	response.setHeaders("Connection", "keep-alive"); // status codes in 500 range except 503 need to close the connection!

	return (response);
}

HTTPResponse ResponseGenerator::redirector(HTTPRequest& request, const std::string& URL)
{
	HTTPResponse 	response;
	std::string		completeURL;

	completeURL = "http://" + request.getHeader("host") + URL;
	response.setVersion("HTTP/1.1");
	response.setStatusCode("301");
	response.setReasonPhrase(reasonPhraseMap[301]);
	response.setHeaders("Content-Type", "text/html");
	response.setHeaders("Content-Length", "0");
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Location", completeURL);
	response.setHeaders("Connection", "keep-alive");

	return (response);
}

HTTPResponse ResponseGenerator::handleAutoIndex(HTTPRequest& request, BaseSettings* settings)
{
	if (settings->getAutoindex() == "off")
		return (serveError(403, settings));
	
	// return (serveDirectoryListing);
	
	(void) request;
	HTTPResponse deleteLater;
	return deleteLater;
}

/*
	Note 1: Since the index directive can take an absolute path in its last entry (that 
			means it could be anywhere from the root and not necessarily restricted to 
			the location block we are in (if we are in one)). As such, we might have to 
			check on the settings of that location block, because chances are, it would 
			have a different configuration.
*/
HTTPResponse ResponseGenerator::handleDirectory(HTTPRequest& request, BaseSettings* settings)
{
	if (request.getURI()[request.getURI().size() - 1] != '/')
		return (redirector(request, request.getURI() + "/")); // redirect to path + "/"

	std::string	path;
	path = settings->getRoot() + request.getURI();
	std::vector<std::string>::const_iterator it;
	for (it = settings->getIndex().begin(); it != settings->getIndex().end(); it++) {
		std::string indexPath;
		if (it == settings->getIndex().end() - 1 && ((*it)[0] == '/')) {
			indexPath = settings->getRoot() + (*it);
			// LocationSettings* locationSettings = serverSettings.findLocation(*it); // Note 1
	    	// if (locationSettings && !locationSettings->isMethodAllowed(request.getMethod()))
    	    // 	return serveError(403, settings);
		}
		else
			indexPath = path + (*it);

		// std::cout << "jfdhjdfshkjdsfhjdfkshdfjkshdf " << indexPath << std::endl;

		if (isFile(indexPath))
			return (serveFile(request, settings, indexPath));
		if ((indexPath[indexPath.size() - 1] == '/') || isDirectory(indexPath))
			return (redirector(request, (*it) + "/")); // redirect to that path + "/"
	}
	return (handleAutoIndex(request, settings));
}

HTTPResponse ResponseGenerator::serveFile(HTTPRequest& request, BaseSettings* settings, std::string& path)
{
	HTTPResponse response;

	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");

	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Content-Type", "text/html"); // MUST BE CHANGEd TO MIMETYPE! USE getMimeType
	response.setHeaders("Connection", "keep-alive");

	// std::cout << "within: jfdhjdfshkjdsfhjdfkshdfjkshdf " << path << std::endl;

	std::ifstream file(path);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << request.getURI() << std::endl;
		if (errno == EACCES)
			return (serveError(403, settings));
		return (serveError(500, settings));
	}

	std::stringstream bodyBuffer;
	bodyBuffer << file.rdbuf();
	std::string body = bodyBuffer.str();
	response.setBody(body);
	response.setHeaders("Content-Length", intToString(body.size()));

	file.close();

	return (response);
}

HTTPResponse ResponseGenerator::serveRequest(HTTPRequest& request, BaseSettings* settings)
{
	std::string	path;

	path = settings->getRoot() + request.getURI();

	if ((path[path.size() - 1] == '/') || isDirectory(path))
		return (handleDirectory(request, settings));
	if (isFile(path))
		return (serveFile(request, settings, path));
	return (serveError(404, settings));
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
			return (serveError(403, settings));
	}

	if (settings->getReturnDirective().getEnabled())
		;
	return (serveRequest(request, settings));
}

HTTPResponse ResponseGenerator::handleRequest(HTTPRequest& request)
{
	if (request.getMethod() == "GET")
		return (handleGetRequest(request));
	// else if (request.getMethod() == "HEAD ")
	// 	;
	// else if (request.getMethod() == "POST")
	// 	;
	// else if (request.getMethod() == "DELETE")
	// 	;

	HTTPResponse deleteLater;
	return deleteLater;
}
