#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <string>
#include <map>
#include <iostream>
#include <sstream>

enum ResponseType {
	CompactResponse,
	ChunkedResponse
};

class HTTPResponse
{
	private:
		ResponseType 						type;
		std::string 						version;
		std::string							statusCode;
		std::string							reasonPhrase;
		std::map<std::string, std::string>	headers;
		std::string							body;
		std::string							fullResponse;
		std::string							filePath;
		long long							fileSize;

		std::string sizeTToString(size_t value);
	public:
		HTTPResponse();
		std::string&	generateResponse();
		void			buildDefaultErrorResponse(std::string statusCode, std::string reasonPhrase);

		void	setFilePath(const std::string& filePath);
		void	setFileSize(const long long& fileSize);
		void	setVersion(const std::string& version);
		void	setStatusCode(const std::string& statusCode);
		void	setReasonPhrase(const std::string& reasonPhrase);
		void	setHeaders(const std::string& headerName, const std::string& headerValue);
		void	setType(const ResponseType type);
		void	setBody(const std::string& body);

		const long long& 	getHeaders();
		const std::string&	getFilePath();
		const long long& 	getFileSize();
		const ResponseType& getType();
		const std::string& 	getReasonPhrase();
};

#endif
