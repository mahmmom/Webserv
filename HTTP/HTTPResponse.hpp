#ifndef HTTPRESPONSE_HPP
# define HTTPRESPONSE_HPP

#include <string>
#include <map>

class HTTPResponse
{
	private:
		std::string 						version;
		std::string							statusCode;
		std::string							reasonPhrase;
		std::map<std::string, std::string>	headers;
		std::string							body;

	public:
		std::string	generateResponse();

		void	setVersion(const std::string& version);
		void	setStatusCode(const std::string& statusCode);
		void	setReasonPhrase(const std::string& reasonPhrase);
		void	setHeaders(const std::string& headerName, const std::string& headerValue);
		void	setBody(const std::string& body);

		const std::string& getReasonPhrase();
};

#endif
