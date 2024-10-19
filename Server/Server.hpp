
#ifndef SERVER_HPP
# define SERVER_HPP

#include "../Config/ServerSettings.hpp"
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
#include "../HTTPRequest/HTTPRequest.hpp"

#define BUFFER_SIZE 8192 // 8 KB as per https://www.ibm.com/docs/en/was-nd/9.0.5?topic=environment-tuning-tcpip-buffer-sizes

class Server
{
	private:
		int						serverSocket;
		ServerSettings			serverSettings;
		EventManager*			eventManager;
		std::map<int, Client>	clients;
		std::vector<int>		toRemove;
		struct sockaddr_in 		serverAddr;
	public:
		Server(ServerSettings& serverSettings, EventManager* eventManager);
		void setSocketOptions();
		void setupServerSocket();
		void bindAndListenServerSocket();
		void setNonBlocking(int& sockFD);

		void launch();

		int& getServerSocket();

		void acceptNewClient();
		void handleClientRead(int clientSocketFD);
		void handleClientWrite(int clientSocketFD);
		void removeDisconnectedClients();
};

#endif