
#ifndef RESPONSEGENERATOR_HPP
# define RESPONSEGENERATOR_HPP

#include "../Settings/ServerSettings.hpp"
#include "../Settings/LocationSettings.hpp"
#include "../Parser/MimeTypesSettings.hpp"
#include "../Logger/Logger.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include <sstream>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <cerrno>
#include <dirent.h> // For directory operations
#include <sys/stat.h> // For file status (to differentiate files from directories)


#define COMPACT_RESPONSE_LIMIT 1048576 // 1 MB
// #define COMPACT_RESPONSE_LIMIT 2097152 // 2 MB

class ResponseGenerator
{
	private:
		bool						isFile(const std::string& requestURI);
		bool						isDirectory(const std::string& requestURI);
		long long 					getFileSize(std::string& filePath);
		ServerSettings 				serverSettings;
		MimeTypesSettings			mimeTypes;
		std::string					intToString(const int intValue);
		std::map<int, std::string>	reasonPhraseMap;
	public:
		ResponseGenerator(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes);

		HTTPResponse	handleReturnDirective(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse	serveDirectoryListing(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse 	handleSubRequest(HTTPRequest& request, const std::string& path);

		HTTPResponse	handleAutoIndex(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse 	serveErrorPage(HTTPRequest& request, int statusCode, BaseSettings* settings);
		HTTPResponse	serveError(HTTPRequest& request, int statusCode, BaseSettings* settings);

		HTTPResponse	redirector(HTTPRequest& request, const std::string& URL);

		HTTPResponse 	handleDirectory(HTTPRequest& request, BaseSettings* settings);
		HTTPResponse	serveChunkedResponse(HTTPRequest& request, BaseSettings* settings, std::string& filePath, long long& fileSize);
		HTTPResponse 	serveSmallFile(HTTPRequest& request, BaseSettings* settings, std::string& path);
		HTTPResponse 	serveFile(HTTPRequest& request, BaseSettings* settings, std::string& path);
		HTTPResponse	serveRequest(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse	handleDeleteRequest(HTTPRequest& request);
		HTTPResponse	handlePostRequest(HTTPRequest& request);
		HTTPResponse	handleHeadRequest(HTTPRequest& request);
		HTTPResponse	handleGetRequest(HTTPRequest& request);
		HTTPResponse	handleRequest(HTTPRequest& request);
};

#endif