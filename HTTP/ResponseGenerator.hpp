
#ifndef RESPONSEGENERATOR_HPP
# define RESPONSEGENERATOR_HPP

#include "../Config/ServerSettings.hpp"
#include "../Config/LocationSettings.hpp"
#include "HTTPResponse.hpp"
#include "HTTPRequest.hpp"
#include <sys/stat.h>
#include <sstream>

class ResponseGenerator
{
	private:
		bool			isFile(const std::string& requestURI);
		bool			isDirectory(const std::string& requestURI);
		ServerSettings 	serverSettings;
		std::string		intToString(const int intValue);
	public:
		ResponseGenerator(ServerSettings serverSettings);

		void			handleReturnDirective();

		HTTPResponse	serveError(int statusCode, BaseSettings* settings);

		HTTPResponse	redirector(HTTPRequest& request, const std::string& URL);

		HTTPResponse 	handleDirectory(HTTPRequest& request, BaseSettings* settings);
		HTTPResponse 	serveFile(HTTPRequest& request, std::string& path);
		HTTPResponse	serveRequest(HTTPRequest& request, BaseSettings* settings);

		HTTPResponse	handleDeleteRequest(HTTPRequest& request);
		HTTPResponse	handlePostRequest(HTTPRequest& request);
		HTTPResponse	handleHeadRequest(HTTPRequest& request);
		HTTPResponse	handleGetRequest(HTTPRequest& request);
		HTTPResponse	handleRequest(HTTPRequest& request);
};

#endif