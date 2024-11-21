
#include "ResponseGenerator.hpp"
#include "HTTPResponse.hpp"

ResponseGenerator::ResponseGenerator(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes) : 
	serverSettings(serverSettings), mimeTypes(mimeTypes)
{
	reasonPhraseMap[200] = "OK";
	reasonPhraseMap[301] = "Moved Permanently";
	reasonPhraseMap[302] = "Moved Temporarily";
	reasonPhraseMap[303] = "See Other";
	reasonPhraseMap[307] = "Temporary Redirect";
	reasonPhraseMap[308] = "Permanent Redirect";
	reasonPhraseMap[400] = "Bad Request";
	reasonPhraseMap[403] = "Forbidden";
	reasonPhraseMap[404] = "Not Found";
	reasonPhraseMap[411] = "Length Required";
	reasonPhraseMap[500] = "Internal Server Error";
	reasonPhraseMap[501] = "Not Implemented";
	reasonPhraseMap[505] = "HTTP Version Not Supported";
}

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

	std::string dirPath;
	if (request.getURI()[0] == '/')
		dirPath = settings->getRoot() + request.getURI();
	else
		dirPath = settings->getRoot() + "/" + request.getURI(); // There is always a slash at the beginning of a URI made by chrome but just in case we get a non-chrome request

    DIR* dir = opendir(dirPath.c_str());
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

        std::string fullPath = dirPath + name;
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

	if (stat(requestURI.c_str(), &pathStat) != 0)
		return false;
	return (S_ISREG(pathStat.st_mode));
}

bool ResponseGenerator::isDirectory(const std::string& requestURI)
{
	struct stat pathStat;

	if (stat(requestURI.c_str(), &pathStat) != 0)
		return false;
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
		
		These are usually triggered when there is an absolute path specified 
		in the Nginx config file, such as in the last entry of the index 
		directive or in the error_page directive. When being "transferred", 
		to an absolute path, and I am using the word transfered because typically, 
		Nginx does not "redirect" you to an absolute URI, instead it does so 
		internally, and this is being mimicked via the this function, 
		handleSubRequest().
		
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
		If this limit is exceeded, the browser itself will stop making requests 
		and display the error message which says: "This page isn’t working 
		localhost redirected you too many times."
*/
HTTPResponse ResponseGenerator::handleSubRequest(HTTPRequest& request, const std::string& path)
{
	// std::cout << "Subpath is " << path << std::endl;
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
	std::map<int, std::string> errorPagesLevel = settings->getErrorPagesLevel();
	std::string path = errorPages[statusCode];
	std::string level = errorPagesLevel[statusCode];

	std::string testPath; // Must check if the error_page listed in the error_page directive actually exists
	if (path[0] == '/') // Checking if the error_page directive entry was an absolute path 
		testPath = settings->getRoot() + path;
	else {
		LocationSettings* derivedPtr = dynamic_cast<LocationSettings*>(settings);
		if (derivedPtr != NULL && level == "location") {
			testPath = settings->getRoot() + derivedPtr->getPath() + "/" + path;
		}
		else
			testPath = settings->getRoot() + "/" + path;
	}

    // std::cout << "\033[31m" // Start red color
    //           << "This is test path -> " << testPath 
    //           << "\033[0m"  // Reset to default color
    //           << std::endl;

	std::ifstream file((testPath).c_str());
	if (!file.is_open()) { // If the error_page does not exist, return a standard 404 (or 403 in rare cases where the file does not have the right permissions)
		int statusCode = 0;
		Logger::log(Logger::ERROR, "Failed to open file: " + request.getURI(), "ResponseGenerator::serveErrorPage");
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
HTTPResponse ResponseGenerator::serveError(HTTPRequest& request, int statusCode, BaseSettings** settingsArray)
{
	if (settingsArray[LOCATION] == NULL) {
		if (settingsArray[SERVER]->getErrorPages().find(statusCode) != settingsArray[SERVER]->getErrorPages().end())
			return (serveErrorPage(request, statusCode, settingsArray[SERVER]));
	}
	else {
		const std::map<int, std::string> locationErrorPages = settingsArray[LOCATION]->getErrorPages();
		std::map<int, std::string>::const_iterator it = locationErrorPages.find(statusCode);

		if (it != locationErrorPages.end()) {
			std::string level = settingsArray[LOCATION]->getErrorPagesLevel().find(statusCode)->second;
			if (level == "location")
				return (serveErrorPage(request, statusCode, settingsArray[LOCATION]));
		}
		if (settingsArray[SERVER]->getErrorPages().find(statusCode) != settingsArray[SERVER]->getErrorPages().end()) {
			return (serveErrorPage(request, statusCode, settingsArray[SERVER]));
		}
	}

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
		response.setReasonPhrase("Unknown Status");
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

HTTPResponse ResponseGenerator::handleAutoIndex(HTTPRequest& request, BaseSettings** settingsFull)
{
	BaseSettings* settings;

	if (settingsFull[LOCATION])
		settings = settingsFull[LOCATION];
	else
		settings = settingsFull[SERVER];

	if (settings->getAutoindex() == "off")
		return (serveError(request, 403, settingsFull));

	return (serveDirectoryListing(request, settings));
}

/*
	Note 1: Since the index directive can take an absolute path in its last entry (that 
			means it could be anywhere from the root and not necessarily restricted to 
			the location block we are in (if we are in one)). As such, we might have to 
			check on the settings of that location block, because chances are, it would 
			have a different configuration.
*/
HTTPResponse ResponseGenerator::handleDirectory(HTTPRequest& request, BaseSettings** settingsFull)
{
	if (request.getURI()[request.getURI().size() - 1] != '/')
		return (redirector(request, request.getURI() + "/")); // redirect to path + "/"

	BaseSettings* settings;

	if (settingsFull[LOCATION])
		settings = settingsFull[LOCATION];
	else
		settings = settingsFull[SERVER];

	// After the if statement above, we now ensure that all URI's passed here end with a trailing slash!
	std::string	path;
	if (request.getURI()[0] == '/')
		path = settings->getRoot() + request.getURI();
	else
		path = settings->getRoot() + "/" + request.getURI(); // There is always a slash at the beginning of a URI made by chrome but just in case we get a non-chrome request

	if (!isDirectory(path)) {
		return (serveError(request, 404, settingsFull));
	}

	std::vector<std::string>::const_iterator it;
	for (it = settings->getIndex().begin(); it != settings->getIndex().end(); it++) {
		std::string indexPath;
		if ((*it)[0] == '/')
			indexPath = path + (*it).substr(1);
		else
			indexPath = path + (*it);

		// std::cout << "\033[31m" // Start red color
		// 		<< "This is index path -> " << indexPath 
		// 		<< "\033[0m"  // Reset to default color
		// 		<< std::endl;

		if (it == settings->getIndex().end() - 1 && ((*it)[0] == '/')) // if it's the last entry in the index directive and it's an absolute path
			return (handleSubRequest(request, (*it).substr(1)));
		if (isFile(indexPath)) // if the entry in the index directive is an actual valid file
			return (serveFile(request, settingsFull, indexPath));
		if (indexPath[indexPath.size() - 1] == '/') // if the entry in the index directive is a directory and has been entered with a trailing slash
		{
			if (isDirectory(indexPath.substr(0, indexPath.size() - 1))) // if the directory is actually valid
				return (handleSubRequest(request, "/" + (*it)));
		}
		if (isDirectory(indexPath)) // if the entry in the index directive is a valid directory but has NOT been entered with a trailing slash
			return (redirector(request, request.getURI() + (*it) + "/")); // redirect to that path + "/"
	}

	return (handleAutoIndex(request, settingsFull));
}

HTTPResponse ResponseGenerator::serveSmallFile(HTTPRequest& request, BaseSettings** settingsFull, std::string& path)
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
			return (serveError(request, 403, settingsFull));
		return (serveError(request, 500, settingsFull));
	}

	std::stringstream bodyBuffer;
	bodyBuffer << file.rdbuf();
	std::string body = bodyBuffer.str();
	response.setBody(body);
	response.setHeaders("Content-Length", intToString(body.size()));

	file.close();

	return (response);
}

HTTPResponse ResponseGenerator::serveChunkedResponse(HTTPRequest& request, BaseSettings** settingsFull, std::string& filePath, long long& fileSize)
{
	HTTPResponse response;

	response.setFilePath(filePath);
	response.setFileSize(fileSize);
	response.setType(ChunkedResponse);
	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");
	response.setHeaders("Accept-Ranges", "bytes");
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Transfer-Encoding", "chunked");
	response.setHeaders("Content-Type", mimeTypes.getMimeType(filePath));
	response.setHeaders("Connection", "keep-alive");

	std::ifstream file(filePath.c_str());
	if (!file.is_open()) {
		Logger::log(Logger::ERROR, "Failed to open file: " + request.getURI(), "ResponseGenerator::serveChunkedResponse");
		if (errno == EACCES)
			return (serveError(request, 403, settingsFull));
		return (serveError(request, 500, settingsFull));
	}
	return (response);
}

bool	ResponseGenerator::parseRangedResponse(std::string& rangeHeader, long long& startByte,
												long long& endByte, long long fileSize)
{
	if (rangeHeader.find("bytes=", 0) != 0)
		return (false);


	size_t		beginPos = rangeHeader.find("=") + 1;
	size_t		endPos = rangeHeader.find("-");
	std::string	startByteStr;
	std::string	endByteStr;

	if (beginPos == std::string::npos || endPos == std::string::npos)
		return (false);
	startByteStr = rangeHeader.substr(beginPos, endPos - beginPos);
	endByteStr = rangeHeader.substr(endPos + 1);

	if (startByteStr.find_first_not_of("0123456789") != std::string::npos)
		return (false);
	if (endByteStr.find_first_not_of("0123456789") != std::string::npos 
		&& !endByteStr.empty())
		return (false);
	
	startByte = stringToLongLong(startByteStr);
	if (startByte == -1 || startByte == -2)
		return (false);

	if (endByteStr.empty())
		endByte = std::min(startByte + MAX_RANGE_SIZE - 1, fileSize - 1);
	else {
		endByte = stringToLongLong(endByteStr);
		if (endByte == -1 || endByte == -2)
			return (false);
		endByte = std::min(endByte, startByte + MAX_RANGE_SIZE - 1);
		endByte = std::min(endByte, fileSize - 1);
	}

	if (startByte > endByte || endByte >= fileSize || startByte >= fileSize)
		return (false);

	return (true);
}

HTTPResponse ResponseGenerator::serveRangedResponse(HTTPRequest& request, BaseSettings** settingsFull, 
														std::string& path, long long fileSize)
{
	HTTPResponse 	response;
	std::string		rangeHeader = request.getHeader("range");
	long long 		startByte;
	long long		endByte;

	if (parseRangedResponse(rangeHeader, startByte, endByte, fileSize) == false) {
		Logger::log(Logger::ERROR, "Invalid range header", "ResponseGenerator::serveRangedResponse");
		return (serveError(request, 416, settingsFull));
	}

	std::ifstream fileStream;
	fileStream.open(path.c_str(), std::ifstream::binary);
	fileStream.seekg(startByte);
	int rangeLen = endByte - startByte + 1 + 1;
	char buffer[rangeLen];
	fileStream.read(buffer, rangeLen);
	size_t bytesRead = fileStream.gcount(); 
	std::string bufferStr(buffer, bytesRead);
	fileStream.close();

	response.setVersion("HTTP/1.1");
	response.setStatusCode("206");
	response.setReasonPhrase("Partial Content");
	response.setBody(bufferStr);
	response.setHeaders("Content-Length", intToString(bufferStr.length()));
	response.setHeaders("Content-Type", "text/plain");
	response.setHeaders("Content-Range", "bytes " + longLongToString(startByte) + "-" 
							+ longLongToString(endByte) + "/" + longLongToString(fileSize));
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Connection", "keep-alive");

	return (response);
}

HTTPResponse ResponseGenerator::serveFile(HTTPRequest& request, BaseSettings** settingsFull, std::string& path)
{
	long long fileSize;

	fileSize = getFileSize(path);
	if (fileSize == -1) {
		Logger::log(Logger::ERROR, "Failed to determine file size", "ResponseGenerator::serveFile");
		return (serveError(request, 500, settingsFull));
	}

	if (fileSize <= COMPACT_RESPONSE_LIMIT)
		return (serveSmallFile(request, settingsFull, path));

	if (!request.getHeader("range").empty())
		return (serveRangedResponse(request, settingsFull, path, fileSize));
	else
		return (serveChunkedResponse(request, settingsFull, path, fileSize));
}

HTTPResponse ResponseGenerator::serveRequest(HTTPRequest& request, BaseSettings** settingsFull)
{
	BaseSettings*	settings;
	std::string		path;

	if (settingsFull[LOCATION])
		settings = settingsFull[LOCATION];
	else
		settings = settingsFull[SERVER];

	if (request.getURI()[0] == '/')
		path = settings->getRoot() + request.getURI();
	else
		path = settings->getRoot() + "/" + request.getURI(); // There is always a slash at the beginning of a URI made by chrome but just in case we get a non-chrome request

	// std::cout << "Path test is " << path << std::endl;
	if ((path[path.size() - 1] == '/') || isDirectory(path))
		return (handleDirectory(request, settingsFull));
	if (isFile(path))
		return (serveFile(request, settingsFull, path));
	return (serveError(request, 404, settingsFull));
}

HTTPResponse ResponseGenerator::handleGetRequest(HTTPRequest& request)
{
	BaseSettings* 		settingsFull[2];
	
	settingsFull[SERVER] = &serverSettings;
	LocationSettings* 	locationSettings = serverSettings.findLocation(request.getURI());

	if (locationSettings)
		settingsFull[LOCATION] = locationSettings;
	else
		settingsFull[LOCATION] = NULL;

	if (request.getStatus() != 200)
		return (serveError(request, request.getStatus(), settingsFull));

	if (settingsFull[SERVER]->getReturnDirective().getEnabled())
		return (handleReturnDirective(request, &serverSettings));

	if (settingsFull[LOCATION]) {
		if (!locationSettings->isMethodAllowed(request.getMethod()))
			return (serveError(request, 405, settingsFull));
		if (settingsFull[LOCATION]->getReturnDirective().getEnabled())
			return (handleReturnDirective(request, settingsFull[LOCATION]));
	}

	return (serveRequest(request, settingsFull));
}

HTTPResponse	ResponseGenerator::handlePostRequest(HTTPRequest& request)
{
	BaseSettings* 		settingsFull[2];
	
	settingsFull[SERVER] = &serverSettings;
	LocationSettings* 	locationSettings = serverSettings.findLocation(request.getURI());

	if (locationSettings)
		settingsFull[LOCATION] = locationSettings;
	else
		settingsFull[LOCATION] = NULL;

	if (request.getStatus() != 200)
		return (serveError(request, request.getStatus(), settingsFull));

	if (settingsFull[SERVER]->getReturnDirective().getEnabled())
		return (handleReturnDirective(request, &serverSettings));

	if (settingsFull[LOCATION]) {
		if (!locationSettings->isMethodAllowed(request.getMethod()))
			return (serveError(request, 405, settingsFull));
		if (settingsFull[LOCATION]->getReturnDirective().getEnabled())
			return (handleReturnDirective(request, settingsFull[LOCATION]));
	}

	HTTPResponse response;

	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");
 	std::string body =	"<html>"
							"<head><title>Request Processed</title></head>"
							"<body>"
								"<h2>Request Successfully Processed</h2>"
								"<p>Your request has been processed by our server. Thank you!</p>"
							"</body>"
						"</html>";
	response.setBody(body);
	response.setHeaders("Content-Length", longLongToString(body.length()));
	response.setHeaders("Content-Type", "text/html");
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Connection", "keep-alive");

	return (response);
}

HTTPResponse ResponseGenerator::handleHeadRequest(HTTPRequest& request)
{
	HTTPResponse response = handleGetRequest(request);
	response.setType(CompactResponse);
	response.setBody("");

	return (response);
}

int ResponseGenerator::isSafeToDelete(const std::string &path)
{
    struct stat pathStat;
    
    // Attempt to retrieve file status
    if (stat(path.c_str(), &pathStat) != 0) {
        if (errno == ENOENT) {
            // File does not exist
            return 404; // Not Found
        }
        // Other errors (e.g., permission issues accessing the file)
        return 500; // Internal Server Error
    }

    // Check if it is a regular file
    if (!S_ISREG(pathStat.st_mode)) {
        // Target is not a regular file, like a directory
        return 403; // Forbidden (or 400 Bad Request if preferred)
    }
    
    // Check if the user has write permission on the file
    if ((pathStat.st_mode & S_IWUSR) == 0) {
        // No write permission on the file
        return 403; // Forbidden
    }

    // Check if the user has write and execute permissions on the directory
    std::string dirPath = path.substr(0, path.find_last_of("/"));
    struct stat dirStat;
    if (stat(dirPath.c_str(), &dirStat) != 0) {
        // Could not access directory
        return 500; // Internal Server Error
    }
    
    if ((dirStat.st_mode & S_IWUSR) == 0 || (dirStat.st_mode & S_IXUSR) == 0) {
        // No write/execute permission on the containing directory
        return 403; // Forbidden
    }
    
    return 200; // OK, safe to delete
}

HTTPResponse ResponseGenerator::handleDeleteRequest(HTTPRequest& request)
{
	BaseSettings* 		settingsFull[2];
	
	settingsFull[SERVER] = &serverSettings;
	LocationSettings* 	locationSettings = serverSettings.findLocation(request.getURI());

	if (locationSettings)
		settingsFull[LOCATION] = locationSettings;
	else
		settingsFull[LOCATION] = NULL;

	if (request.getStatus() != 200)
		return (serveError(request, request.getStatus(), settingsFull));

	if (settingsFull[SERVER]->getReturnDirective().getEnabled())
		return (handleReturnDirective(request, &serverSettings));

	if (settingsFull[LOCATION]) {
		if (!locationSettings->isMethodAllowed(request.getMethod()))
			return (serveError(request, 405, settingsFull));
		if (settingsFull[LOCATION]->getReturnDirective().getEnabled())
			return (handleReturnDirective(request, settingsFull[LOCATION]));
	}

    HTTPResponse response;
    std::string filePath;

    // Construct the file path based on the root directory and request URI
    if (request.getURI()[0] == '/')
        filePath = serverSettings.getRoot() + request.getURI();
    else
        filePath = serverSettings.getRoot() + "/" + request.getURI();

    // Use isSafeToDelete to check the file and get the status code
    int statusCode = isSafeToDelete(filePath);
    if (statusCode != 200) {
        // Return an error response based on the status code
        response.setVersion("HTTP/1.1");
        response.setStatusCode(longLongToString(statusCode));
        response.setReasonPhrase(reasonPhraseMap[statusCode]);
        response.setHeaders("Server", "Ranchero");
        response.setHeaders("Content-Length", "0"); // No body for error responses
        return response;
    }


    // Attempt to delete the file
    if (remove(filePath.c_str()) != 0) {
        // Unexpected error during deletion
        response.setVersion("HTTP/1.1");
        response.setStatusCode("500");
        response.setReasonPhrase("Internal Server Error");
        response.setHeaders("Server", "Ranchero");
        response.setHeaders("Content-Length", "0"); // No body
        return response;
    }

    // File deleted successfully, return 204 No Content
    response.setVersion("HTTP/1.1");
    response.setStatusCode("204");
    response.setReasonPhrase("No Content");
    response.setHeaders("Server", "Ranchero");
    response.setHeaders("Content-Length", "0"); // No body
    return response;
}

/*
	NOTES:

		Note 1:	This is just to shut the compiler up. We will never hit this point
				because at this point in the code, we have guaranteed that the 
				method in the requestURI has to be one of these 4. The functions 
				ClientManager::parseHeaders(Server &server) and the subcall to
				server.handleInvalidRequest(fd, "501", "Not Implemented"); have handled 
				this case already and this function HTTPResponse won't be called by the 
				Server class object in that scenario.
*/
HTTPResponse ResponseGenerator::handleRequest(HTTPRequest& request)
{
	if (request.getMethod() == "GET")
		return (handleGetRequest(request));
	else if (request.getMethod() == "HEAD")
		return (handleHeadRequest(request));
	else if (request.getMethod() == "POST")
		return (handlePostRequest(request));
	else if (request.getMethod() == "DELETE")
		return (handleDeleteRequest(request));
	
	HTTPResponse controlPathResponse;
	return (controlPathResponse); // Note 1
}

long long ResponseGenerator::stringToLongLong(const std::string& string)
{
	std::stringstream 	fileStream(string);
	long long			longVal;

	fileStream >> longVal;
    if (fileStream.fail()) {
        Logger::log(Logger::WARN,  "Invalid number format when attempting to convert bytes range header"
										"value from string to long long", 
			"ResponseGenerator::stringToLongLong");
		return (-1);
    }

    // Check for leftover characters
    if (!fileStream.eof()) {
        Logger::log(Logger::WARN,  "Extra characters present after number while converting from string format"
									"to long long in the bytes range header", 
			"ResponseGenerator::stringToLongLong");
		return (-2);
    }
	return (longVal);
}

std::string ResponseGenerator::longLongToString(long long value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
