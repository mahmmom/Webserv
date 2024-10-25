
#ifndef RESPONSEGENERATOR_HPP
# define RESPONSEGENERATOR_HPP

#include "../Config/ServerSettings.hpp"
#include "../Config/LocationSettings.hpp"
#include "../Parser/MimeTypesSettings.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include <sstream>
#include <map>
#include <fstream>
#include <sys/stat.h>
#include <cerrno>
#include <dirent.h> // For directory operations
#include <sys/stat.h> // For file status (to differentiate files from directories)

class ResponseGenerator
{
	private:
		bool						isFile(const std::string& requestURI);
		bool						isDirectory(const std::string& requestURI);
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

		HTTPResponse	serveError(int statusCode, BaseSettings* settings);

		HTTPResponse	redirector(HTTPRequest& request, const std::string& URL);

		HTTPResponse 	handleDirectory(HTTPRequest& request, BaseSettings* settings);
		HTTPResponse 	serveFile(HTTPRequest& request, BaseSettings* settings, std::string& path);
		HTTPResponse	serveRequest(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse	handleDeleteRequest(HTTPRequest& request);
		HTTPResponse	handlePostRequest(HTTPRequest& request);
		HTTPResponse	handleHeadRequest(HTTPRequest& request);
		HTTPResponse	handleGetRequest(HTTPRequest& request);
		HTTPResponse	handleRequest(HTTPRequest& request);
};

#endif