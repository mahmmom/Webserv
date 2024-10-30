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

		bool	processHostField(std::string& hostValue);
		bool	processPostRequest();
		bool	checkDuplicates(std::string& headerFieldName);
		bool	buildHeaderMap(std::vector<std::string>& headerFields);
		void	tokenizeHeaderFields(const std::string& fullRequest);

		bool	processRequestLine(const std::string& requestLine);
		bool 	isRequestLineComplete(const std::string& requestLine);
		void	extractQueries(const std::string& querieString);
		bool	processURI(std::string& uriTok);
		bool	processVersion(std::string& versionTok);

		void setStatus(const int& status);

		void	debugger();

	public:
		int 								clientSocketFD;

		HTTPRequest();

		std::string			getHeader(const std::string& headerName);
		const std::string&	getMethod() const;
		const int& 			getStatus() const;
		const std::string& 	getURI() const;

		void 	setURI(const std::string& uri);

		HTTPRequest(const std::string& full_request, int clientSocketFD);
		~HTTPRequest();
};

#endif