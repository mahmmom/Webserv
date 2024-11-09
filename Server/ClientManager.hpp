
#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include "Server.hpp"

class ClientManager
{
	private:

		int													fd;
		std::string											clientAddress;
		int													requestNumber;
		std::time_t											lastRequestTime;

		HTTPRequest											request;
		std::string											requestHeaders;
		std::string											requestBody;
		std::ofstream										requestBodyFile;
		size_t												requestBodySize;
		std::string											requestBodyFilePath;
		bool												areHeaderComplete;
		bool												isBodyComplete;
		
		std::string	intToString(int number);
		size_t		stringToSizeT(const std::string str);
	public:

		ClientManager(int fd, const std::string &clientIpAddr);
		~ClientManager();

		void	resetClientManager();
		void	updateLastRequestTime();
		void	incrementRequestsNumber();

		void 	processIncomingData(Server &server, const char *buffer, size_t bytesRead);
		void	processHeaders(Server &server, const char *buffer, size_t bytesRead);
		void	processBody(Server &server, const char *buffer, size_t bytesRead);
		void	parseHeaders(Server &server);
		void	initializeBodyStorage(Server &server);

		void	handleGetRequest(Server &server);
		void	handleHeadRequest(Server &server);
		void	handleDeleteRequest(Server &server);
		void	handlePostRequest(Server &server);

		int					getSocket() const;
		const std::string&	getClientAddress() const;
		int					getRequestCount() const;
		const std::string&	getPostRequestFileName();
		const HTTPRequest&	getRequest();
		bool				isTimedout(size_t keepaliveTimeout) const;
};

#endif