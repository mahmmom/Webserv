#ifndef HTTPREQUEST_HPP
# define HTTPREQUEST_HPP

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>

class HTTPRequest
{
	private:
		int									status;
		std::string							uri;
		std::string							method;
		std::map<std::string, std::string>	headers;

		void	tokenizeHeaderFields(const std::string& fullRequest);
		bool	buildHeaderMap(std::vector<std::string>& headerFields);
		bool	processRequestLine(const std::string& requestLine);

		HTTPRequest();
		HTTPRequest& operator=(const HTTPRequest& other);
		HTTPRequest(const HTTPRequest& other);
	public:
		HTTPRequest(const std::string& full_request);
		~HTTPRequest();
};

#endif