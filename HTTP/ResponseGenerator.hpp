
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
#include <algorithm>
#include <dirent.h> // For directory operations
#include <sys/stat.h> // For file status (to differentiate files from directories)
#include <cstring>

#define COMPACT_RESPONSE_LIMIT 1048576 // 1 MB
// #define COMPACT_RESPONSE_LIMIT 2097152 // 2 MB
#define MAX_RANGE_SIZE 1048576 // 1 MB

enum SettingsIndex {
	SERVER,
	LOCATION
};

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
		
		int 						isSafeToDelete(const std::string &path);
		
		long long					stringToLongLong(const std::string& string);
		std::string 				longLongToString(long long value);
		std::string					postBody;

	public:
		ResponseGenerator(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes, const std::string& postBody = "");

		HTTPResponse	handleReturnDirective(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse	serveDirectoryListing(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse 	handleSubRequest(HTTPRequest& request, const std::string& path, bool isErrorPage, BaseSettings** settingsFull);

		HTTPResponse	handleAutoIndex(HTTPRequest& request, BaseSettings** settingsFull);

		HTTPResponse 	serveErrorPage(HTTPRequest& request, int statusCode, BaseSettings* settings, BaseSettings** settingsFull);
		HTTPResponse	serveError(HTTPRequest& request, int statusCode, BaseSettings** settingsFull);

		HTTPResponse	redirector(HTTPRequest& request, const std::string& URL);

		HTTPResponse 	handleDirectory(HTTPRequest& request, BaseSettings** settingsFull);
		HTTPResponse	serveChunkedResponse(HTTPRequest& request, BaseSettings** settingsFull, std::string& filePath, long long& fileSize);
		HTTPResponse 	serveSmallFile(HTTPRequest& request, BaseSettings** settingsFull, std::string& path);
		HTTPResponse 	serveFile(HTTPRequest& request, BaseSettings** settingsFull, std::string& path);
		bool			parseRangedResponse(std::string& rangeHeader, long long& startByte, long long& endByte, long long fileSize);
		HTTPResponse	serveRangedResponse(HTTPRequest& request, BaseSettings** settingsFull, std::string& path, long long fileSize);
		HTTPResponse	serveRequest(HTTPRequest& request, BaseSettings** settingsFull);

		HTTPResponse	handleDeleteRequest(HTTPRequest& request);
		HTTPResponse	handlePostRequest(HTTPRequest& request);
		HTTPResponse	handleHeadRequest(HTTPRequest& request, BaseSettings* fallbackLocation);
		HTTPResponse	handleGetRequest(HTTPRequest& request, BaseSettings* fallbackLocation);
		HTTPResponse	handleRequest(HTTPRequest& request, BaseSettings* fallbackLocation);
};

#endif