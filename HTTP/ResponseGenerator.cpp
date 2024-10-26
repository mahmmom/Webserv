
#include "ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes) : 
	serverSettings(serverSettings), mimeTypes(mimeTypes)
{
	reasonPhraseMap[301] = "Moved Permanently";
	reasonPhraseMap[302] = "Moved Temporarily";
	reasonPhraseMap[403] = "Forbidden";
	reasonPhraseMap[404] = "Not Found";
	reasonPhraseMap[500] = "Internal Server Error";
}

/*
	301, 302, 303, 307, and 308

	HTTP/1.1 302 Moved Temporarily
	Server: nginx/1.27.1
	Date: Thu, 24 Oct 2024 01:24:57 GMT
	Content-Type: text/html
	Content-Length: 145
	Connection: close
	Location: https://google.com

	<html>
	<head><title>302 Found</title></head>
	<body>
	<center><h1>302 Found</h1></center>
	<hr><center>nginx/1.27.1</center>
	</body>
	</html>
*/
// HTTPResponse ResponseGenerator::handleReturnDirective(HTTPRequest& request, BaseSettings* settings)
// {
// 	HTTPResponse response;

// 	const ReturnDirective returnDirective = settings->getReturnDirective();
// 	if (returnDirective.getTextOrURL() != "empty")
// 	{
// 		int statusCode = request.getStatus();
// 		if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307 ||
// 				statusCode == 308) {
// 			response.setStatusCode(intToString(statusCode));
// 			response.setHeaders("Content-type", "text/html"); // Should be changed to mime-type
// 		}
// 		else if (statusCode == -1) {
// 			response.setStatusCode("302");
// 			response.setReasonPhrase(reasonPhraseMap[302]);
// 			response.setHeaders("Server", "Ranchero");
// 			response.setHeaders("Content-Type", "text/html");
// 			response.setHeaders("Location", returnDirective.getTextOrURL());
// 			std::string body = 	"<html>"
// 								"<head><title>302 Found</title></head>"
// 								"<body>"
// 								"<center><h1>302 Found</h1></center>"
// 								"<hr><center>Ranchero</center>"
// 								"</body>"
// 								"</html>";
// 			response.setBody(body);
// 			response.setHeaders("Content-Length", intToString(body.length()));
// 		}
// 		else {
// 			response.setHeaders("Content-type", "plain/text");
// 		}
	
// 	}

// }

HTTPResponse ResponseGenerator::serveDirectoryListing(HTTPRequest& request, BaseSettings* settings) {
    HTTPResponse response;
    response.setVersion("HTTP/1.1");
    response.setStatusCode("200");
    response.setReasonPhrase("OK");
    response.setHeaders("Content-Type", "text/html");
    response.setHeaders("Server", "Ranchero");
    response.setHeaders("Connection", "keep-alive");

    std::string body;
    body = "<html>"
           "<head><title>Index of " + request.getURI() + "</title></head>"
           "<body>"
           "<h1>Index of " + request.getURI() + "</h1><hr><pre><a href=\"../\">../</a>\n";

    DIR* dir = opendir((settings->getRoot() + request.getURI()).c_str());
    if (dir == NULL) {
        // Handle error case
        response.setStatusCode("404");
        response.setReasonPhrase("Not Found");
        return response;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") 
            continue;

        std::string fullPath = settings->getRoot() + request.getURI() + name;
        if (isFile(fullPath)) {
            body += "<a href=\"" + name + "\">" + name + "</a>\n";
        } else {
            body += "<a href=\"" + name + "/\">" + name + "/</a>\n";
        }
    }

    body += "</pre><hr></body></html>";
    closedir(dir);

    // Add Content-Length header
    response.setHeaders("Content-Length", intToString(body.length()));
    response.setBody(body);
    
    return response;
}

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
HTTPResponse ResponseGenerator::handleSubRequest(HTTPRequest& request, const std::string& path)
{
	request.setURI(path);
	return (handleRequest(request));
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
	(void) settings;
	// if (settings->getErrorPages().find(statusCode) != settings->getErrorPages().end())
	// 	; // return serveErrorPage

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
	
	return (serveDirectoryListing(request, settings));
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
		std::string indexPath = path + (*it);
		if (it == settings->getIndex().end() - 1 && ((*it)[0] == '/'))
			return (handleSubRequest(request, (*it)));
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
	response.setHeaders("Content-Type", mimeTypes.getMimeType(path)); // MUST BE CHANGEd TO MIMETYPE! USE getMimeType
	response.setHeaders("Connection", "keep-alive");

	std::ifstream file(path.c_str());
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
	// if (serverSettings.getReturnDirective().getEnabled())
	// 	;
		// return (handleReturnDirective(request, &serverSettings));

	BaseSettings*		settings = &serverSettings;
	LocationSettings* 	locationSettings = serverSettings.findLocation(request.getURI());

	if (locationSettings) {
		settings = locationSettings;
		if (!locationSettings->isMethodAllowed(request.getMethod()))
			return (serveError(403, settings));
	}

	// if (settings->getReturnDirective().getEnabled())
	// 	;
		// return (handleReturnDirective(request, settings));
	return (serveRequest(request, settings));
}

HTTPResponse ResponseGenerator::handleRequest(HTTPRequest& request)
{
	std::cout << "kfjdkdsjfkldsjkfdjdfs\n";
	std::cout << "URI is " << request.getURI() << std::endl;
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
