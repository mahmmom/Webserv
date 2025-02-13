
#ifndef CLIENTMANAGER_HPP
#define CLIENTMANAGER_HPP

#include "Server.hpp"
#include "../HTTP/RequestManager.hpp"

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

		// std::ofstream										requestBodyFile;
		std::string 										requestBuffer;

		size_t												requestBodySize;
		// std::string											requestBodyFilePath;

		bool												areHeaderComplete;
		bool												isBodyComplete;
		
		bool 												isChunkedTransfer;
    	RequestManager* 									requestManager; 

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
		void	initializeBodyStorage(Server &server, BaseSettings* settings);

		void	handleGetRequest(Server &server);
		void	handleHeadRequest(Server &server);
		void	handleDeleteRequest(Server &server);
		void	handlePostRequest(Server &server);

		int						getSocket() const;
		const std::string&		getClientAddress() const;
		int						getRequestCount() const;
		const HTTPRequest&		getRequest();
		const RequestManager*	getRequestManager();

		bool				isTimedout(size_t keepaliveTimeout) const;
};

#endif