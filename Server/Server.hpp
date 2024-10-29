
#ifndef SERVER_HPP
# define SERVER_HPP

#include "../Logger/Logger.hpp"
#include "../Settings/ServerSettings.hpp"
#include "../Parser/MimeTypesSettings.hpp"
#include <map>
#include "Client.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Errors.hpp"
#include <fcntl.h>
#include <unistd.h> // For close()
#include <cstring>
#include <cerrno>
#include <cstdio>

#include "../Events/EventManager.hpp"
#include "../HTTP/HTTPRequest.hpp"
#include "../HTTP/ResponseGenerator.hpp"
#include "../HTTP/ResponseManager.hpp"

#define BUFFER_SIZE 8192 // 8 KB as per https://www.ibm.com/docs/en/was-nd/9.0.5?topic=environment-tuning-tcpip-buffer-sizes

class Server
{
	private:
		int									serverSocket;
		int									serverPort;
		std::string							serverInterface;
		ServerSettings						serverSettings;
		MimeTypesSettings					mimeTypes;
		EventManager*						eventManager;
		std::map<int, Client>				clients;
		std::vector<int>					toRemove;
		struct sockaddr_in 					serverAddr;
		std::map<int, ResponseManager* > 	responses; // change to responseState* later
	public:
		Server(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes, EventManager* eventManager);

		void setSocketOptions();
		void setupServerSocket();
		void bindAndListenServerSocket();
		void setNonBlocking(int& sockFD);

		void launch();

		void checkTimeouts();

		int& getServerSocket();
		int& getServerPort();
		std::string& getServerInterface();

		void acceptNewClient();


		void handleGetRequest(int& clientSocketFD, HTTPRequest& request);
		void handleClientRead(int& clientSocketFD);

		void sendChunkedBody(int& clientSocketFD, ResponseManager* responseManager);
		void sendChunkedHeaders(int& clientSocketFD, ResponseManager* responseManager);
		void sendChunkedResponse(int& clientSocketFD, ResponseManager* responseManager);
		void sendCompactFile(int& clientSocketFD, ResponseManager* responseManager);
		void handleClientWrite(int& clientSocketFD);

		void removeBadClients(int& clientSocketFD);
		void removeDisconnectedClients(int& clientSocketFD);
};

#endif