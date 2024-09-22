#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>

class HTTPRequest
{
	private:
		int									status;
		std::string							uri;
		std::string							method;
		std::string							version;
		std::vector<std::string>			queries;
		std::map<std::string, std::string>	headers;

		void	tokenizeHeaderFields(const std::string& fullRequest);
		bool	buildHeaderMap(std::vector<std::string>& headerFields);

		bool	processRequestLine(const std::string& requestLine);
		void	extractQueries(const std::string& querieString);
		bool	processURI(std::string& uriTok);
		bool	processVersion(std::string& versionTok);

		HTTPRequest();
		HTTPRequest& operator=(const HTTPRequest& other);
		HTTPRequest(const HTTPRequest& other);

		void	debugger();
	public:
		HTTPRequest(const std::string& full_request);
		~HTTPRequest();
};

#endif