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
		int									fallbackCounter;
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

		void	normalizeURI();

	public:
		int 								clientSocketFD;

		HTTPRequest();

		std::string							getHeader(const std::string& headerName);
		const std::string&					getMethod() const;
		const int& 							getStatus();
		const std::string& 					getURI() const;
		const std::vector<std::string>& 	getQueries() const;
		const std::string& 					getVersion() const;

		void	setStatus(const int& status);
		void 	setURI(const std::string& uri);

		void	incrementFallbackCounter();

		void	debugger();

		HTTPRequest(const std::string& full_request, int clientSocketFD);
		~HTTPRequest();
};

#endif