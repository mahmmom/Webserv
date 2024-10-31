
#include "ResponseGenerator.hpp"

ResponseGenerator::ResponseGenerator(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes) : 
	serverSettings(serverSettings), mimeTypes(mimeTypes)
{
	reasonPhraseMap[200] = "OK";
	reasonPhraseMap[301] = "Moved Permanently";
	reasonPhraseMap[302] = "Moved Temporarily";
	reasonPhraseMap[303] = "See Other";
	reasonPhraseMap[307] = "Temporary Redirect";
	reasonPhraseMap[308] = "Permanent Redirect";
	reasonPhraseMap[403] = "Forbidden";
	reasonPhraseMap[404] = "Not Found";
	reasonPhraseMap[500] = "Internal Server Error";
}

/*

*/
HTTPResponse ResponseGenerator::handleReturnDirective(HTTPRequest& request, BaseSettings* settings)
{
	(void) request;
	HTTPResponse response;

	const ReturnDirective returnDirective = settings->getReturnDirective();
	if (returnDirective.getTextOrURL() != "empty")
	{
		int statusCode = returnDirective.getStatusCode();
		std::cout << "This is the statusCode " << statusCode << std::endl;

		response.setVersion("HTTP/1.1");
		response.setHeaders("Server", "Ranchero");
		response.setHeaders("Connection", "keep-alive");
		if (statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 307 ||
				statusCode == 308 || statusCode == -1)
		{
			if (statusCode == -1)
				statusCode = 302;
			response.setStatusCode(intToString(statusCode));
			response.setReasonPhrase(reasonPhraseMap[statusCode]);
			response.setHeaders("Location", returnDirective.getTextOrURL());
			response.setHeaders("Content-type", "text/html");
			std::string body = 	"<html>"
								"<head><title>" + intToString(statusCode) + " " + reasonPhraseMap[statusCode] + "</title></head>"
								"<body>"
								"<center><h1>" + intToString(statusCode) + " " + reasonPhraseMap[statusCode] + "</h1></center>"
								"<hr><center>Ranchero</center>"
								"</body>"
								"</html>";
			response.setBody(body);
			response.setHeaders("Content-Length", intToString(body.length()));
		}
		else {
			response.setStatusCode(intToString(statusCode));
			response.setReasonPhrase(reasonPhraseMap[statusCode]);
			response.setHeaders("Content-type", "text/plain");
			std::string body = returnDirective.getTextOrURL();
			response.setBody(body);
			response.setHeaders("Content-Length", intToString(body.length()));
		}
	}
	return response;
}

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

long long ResponseGenerator::getFileSize(std::string& filePath) 
{
    struct stat pathStat;
    if (stat(filePath.c_str(), &pathStat) != 0) {
        std::cerr << "Failed to get file information.\n";
        return -1;
    }
    return pathStat.st_size;  // File size in bytes
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
		
		As for the too many redirects scenario (for example if in the return 
		directive you return a page to the same location block like this: 
			location /index {
				limit_except GET;
				return 302 /index/index.html;
			}
		), you don't have to handle that, the browser does so automatically. So 
		the server must actually server all the requests being made. What happens 
		instead is that browsers have a built-in limit for how many times they 
		will follow redirects. This limit is generally around 20 to 50 redirects. 
		If this limit is exceeded, the itself browser will stop making requests 
		and display the error message which says: "This page isn’t working 
		localhost redirected you too many times."
*/
HTTPResponse ResponseGenerator::handleSubRequest(HTTPRequest& request, const std::string& path)
{
	std::cout << "Subpath is " << path << std::endl;
	request.setURI(path);
	return (handleRequest(request));
}

/*
	NOTES
	
		Note 1:	This is the way Nginx does it, when it does a 302 redirect, it just puts 
				the relative path, unlike a 301 where the absolute path is given with an 
				"http://" at the beginning. With 302's and relative pahts, the browser 
				knows how to deal with that automatically. It will append it to the last 
				URI. So, if you have an error_page 404 404.html directive in a location 
				called /alt, if you request localhost/alt/nonexistent, you would be 
				redirected to localhost/alt/404.html by the browser. But if you did, 
				localhost/alt/nonexistent1/nonexisten2, then it would trigger a too many 
				redirections error BUT don't worry, because that's what happens with Nginx 
				too. The reason is because the 302 would redirect to localhost/alt/nonexistent1/404.html, 
				which also does not exist, and that would trigger a continous trail of errors 
				from our server. So the best way to avoid this problem is for the person 
				configuring the Nginx server to ALWAYS use absolute paths in the error_page 
				directive to avoid this problem (even though Nginx does not explicitly 
				mandate this requirement)
*/
HTTPResponse ResponseGenerator::serveErrorPage(HTTPRequest& request, int statusCode, BaseSettings* settings)
{
	HTTPResponse response;
	std::map<int, std::string> errorPages = settings->getErrorPages();
	std::string path = errorPages[statusCode];

	std::string testPath; // Must check if the error_page listed in the error_page directive actually exists
	if (path[0] == '/')
		testPath = settings->getRoot() + path;
	else {
		if (LocationSettings* derivedPtr = dynamic_cast<LocationSettings*>(settings))
			testPath = settings->getRoot() + derivedPtr->getPath() + "/" + path;
		else
			testPath = settings->getRoot() + "/" + path;
	}
	std::ifstream file((testPath).c_str());
	if (!file.is_open()) { // If the error_page does not exist, return a standard 404 (or 403 in rare cases where the file does not have the right permissions)
		int statusCode = 0;
		Logger::log(Logger::ERROR, "Failed to open file: " + request.getURI(), "ResponseGenerator::serveSmallFile");
		if (errno == EACCES)
			statusCode = 403;
		else
			statusCode = 404;
		response.setVersion("HTTP/1.1");
		response.setStatusCode(intToString(statusCode));
		response.setHeaders("Server", "Ranchero");
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
		response.setHeaders("Connection", "keep-alive");
		return (response);
	}

	if (path[0] == '/')
		return (handleSubRequest(request, path));
	response.setVersion("HTTP/1.1");
	response.setStatusCode(intToString(302));
	response.setReasonPhrase(reasonPhraseMap[statusCode]);
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Connection", "keep-alive");
	response.setHeaders("Location", path); // Note 1
	std::string body = 	"<html>"
						"<head><title>302 Found</title></head>"
						"<body>"
						"<center><h1>302 Found</h1></center>"
						"<hr><center>Ranchero</center>"
						"</body>"
						"</html>";
	response.setBody(body);
	response.setHeaders("Content-Length", intToString(body.length()));
	return (response);
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
HTTPResponse ResponseGenerator::serveError(HTTPRequest& request, int statusCode, BaseSettings* settings)
{
	(void) settings;
	(void) request;
	if (settings->getErrorPages().find(statusCode) != settings->getErrorPages().end())
		return (serveErrorPage(request, statusCode, settings));

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
		return (serveError(request, 403, settings));

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
	if (request.getURI()[0] == '/')
		path = settings->getRoot() + request.getURI();
	else
		path = settings->getRoot() + "/" + request.getURI();

	std::vector<std::string>::const_iterator it;
	for (it = settings->getIndex().begin(); it != settings->getIndex().end(); it++) {
		std::string indexPath = path + (*it);
		if (it == settings->getIndex().end() - 1 && ((*it)[0] == '/'))
			return (handleSubRequest(request, (*it).substr(1)));
		if (isFile(indexPath))
			return (serveFile(request, settings, indexPath));
		if (indexPath[indexPath.size() - 1] == '/')
		{
			if (isDirectory(indexPath.substr(0, indexPath.size() - 1)))
				return (handleSubRequest(request, "/" + (*it)));
		}
		if (isDirectory(indexPath))
			return (redirector(request, "/" + (*it) + "/")); // redirect to that path + "/"
	}
	return (handleAutoIndex(request, settings));
}

HTTPResponse ResponseGenerator::serveSmallFile(HTTPRequest& request, BaseSettings* settings, std::string& path)
{
	HTTPResponse response;

	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");

	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Content-Type", mimeTypes.getMimeType(path));
	response.setHeaders("Connection", "keep-alive");

	std::ifstream file(path.c_str());
	if (!file.is_open()) {
		Logger::log(Logger::ERROR, "Failed to open file: " + request.getURI(), "ResponseGenerator::serveSmallFile");
		if (errno == EACCES)
			return (serveError(request, 403, settings));
		return (serveError(request, 500, settings));
	}

	std::stringstream bodyBuffer;
	bodyBuffer << file.rdbuf();
	std::string body = bodyBuffer.str();
	response.setBody(body);
	response.setHeaders("Content-Length", intToString(body.size()));

	file.close();

	return (response);
}

HTTPResponse ResponseGenerator::serveChunkedResponse(HTTPRequest& request, BaseSettings* settings, std::string& filePath, long long& fileSize)
{
	HTTPResponse response;

	response.setFilePath(filePath);
	response.setFileSize(fileSize);
	response.setType(ChunkedResponse);
	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");

	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Transfer-Encoding", "chunked");
	response.setHeaders("Content-Type", mimeTypes.getMimeType(filePath));
	response.setHeaders("Connection", "keep-alive");

	std::ifstream file(filePath.c_str());
	if (!file.is_open()) {
		Logger::log(Logger::ERROR, "Failed to open file: " + request.getURI(), "ResponseGenerator::serveSmallFile");
		if (errno == EACCES)
			return (serveError(request, 403, settings));
		return (serveError(request, 500, settings));
	}
	return (response);
}

HTTPResponse ResponseGenerator::serveFile(HTTPRequest& request, BaseSettings* settings, std::string& path)
{
	long long fileSize;

	fileSize = getFileSize(path);

	if (fileSize == -1) {
		Logger::log(Logger::ERROR, "Failed to determine file size", "ResponseGenerator::serveFile");
		return (serveError(request, 500, settings));
	}
	if (fileSize <= COMPACT_RESPONSE_LIMIT)
		return (serveSmallFile(request, settings, path));
	return (serveChunkedResponse(request, settings, path, fileSize));
}

HTTPResponse ResponseGenerator::serveRequest(HTTPRequest& request, BaseSettings* settings)
{
	std::string	path;

	if (request.getURI()[0] == '/')
		path = settings->getRoot() + request.getURI();
	else
		path = settings->getRoot() + "/" + request.getURI();

	std::cout << "Path test is " << path << std::endl;
	if ((path[path.size() - 1] == '/') || isDirectory(path))
		return (handleDirectory(request, settings));
	if (isFile(path))
		return (serveFile(request, settings, path));
	return (serveError(request, 404, settings));
}

HTTPResponse ResponseGenerator::handleGetRequest(HTTPRequest& request)
{
	if (serverSettings.getReturnDirective().getEnabled())
		return (handleReturnDirective(request, &serverSettings));

	BaseSettings*		settings = &serverSettings;
	LocationSettings* 	locationSettings = serverSettings.findLocation(request.getURI());

	if (locationSettings) {
		settings = locationSettings;
		if (!locationSettings->isMethodAllowed(request.getMethod()))
			return (serveError(request, 403, settings));
	}
	if (settings->getReturnDirective().getEnabled())
		return (handleReturnDirective(request, settings));
	return (serveRequest(request, settings));
}

HTTPResponse ResponseGenerator::handleRequest(HTTPRequest& request)
{
	if (request.getMethod() == "GET") {
		std::string message = "Recieve a GET request from "  ;
		Logger::log(Logger::INFO, "Recieved a GET request from " 
			+ Logger::intToString(request.clientSocketFD) + " for "
			+ request.getURI(), "ResponseGenerator::handleRequest");
		return (handleGetRequest(request));
	}
	// else if (request.getMethod() == "HEAD ")
	// 	;
	// else if (request.getMethod() == "POST")
	// 	;
	// else if (request.getMethod() == "DELETE")
	// 	;

	HTTPResponse deleteLater;
	return deleteLater;
}
