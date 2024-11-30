
#ifndef SERVER_HPP
# define SERVER_HPP

#include "../Logger/Logger.hpp"
#include "../Settings/ServerSettings.hpp"
#include "../Parser/MimeTypesSettings.hpp"
#include <map>
// #include "Client.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Errors.hpp"
#include <fcntl.h>
#include <unistd.h> // For close()
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <signal.h>

#include "../Events/EventManager.hpp"
#include "../HTTP/HTTPRequest.hpp"
#include "../HTTP/ResponseGenerator.hpp"
#include "../HTTP/ResponseManager.hpp"
#include "../CGI/CGIManager.hpp"

#define BUFFER_SIZE 65536 // 64 KB
// #define BUFFER_SIZE 8192 // 8 KB as per https://www.ibm.com/docs/en/was-nd/9.0.5?topic=environment-tuning-tcpip-buffer-sizes
// #define BUFFER_SIZE 8192 // 8 KB as per https://www.ibm.com/docs/en/was-nd/9.0.5?topic=environment-tuning-tcpip-buffer-sizes

#define MAX_HEADER_SIZE 8192 // 8 KB
#define MAX_URI_SIZE 2048 // 2 KB
#define MAX_CGI_OUTPUT_SIZE 115343360 // 110 MB
// #define MAX_CGI_OUTPUT_SIZE 2097152 // 2 MB
#define CGI_TIMEOUT 20
// #define CGI_TIMEOUT 20
#define CGI_LOAD_LIMIT 21
// #define CGI_LOAD_LIMIT 3

#define TEMP_FILE_DIRECTORY "Server/uploads/"

class ClientManager;
class Server
{
	private:
		int									serverSocket;
		int									serverPort;
		std::string							serverInterface;
		ServerSettings						serverSettings;
		MimeTypesSettings					mimeTypes;
		EventManager*						eventManager;
		struct sockaddr_in 					serverAddr;
		std::map<int, ResponseManager* > 	responses; // change to responseState* later
		std::map<int, ClientManager* >		clients;
		std::map<int, CGIManager* >			cgi;

	public:
		Server(ServerSettings& serverSettings, MimeTypesSettings& mimeTypes, EventManager* eventManager);
		~Server();

		bool checkClientInServer(int clientSocketFD);
		void setSocketOptions();
		void setupServerSocket();
		void bindAndListenServerSocket();
		void setNonBlocking(int sockFD);

		void launch();

		void checkTimeouts();
		void checkCgiTimeouts();

		int& getServerSocket();
		int& getServerPort();
		std::string& getServerInterface();

		void acceptNewClient();

		void handleCgiOutput(int cgiReadFD);

		void handleClientRead(int clientSocketFD);
		void handleClientWrite(int clientSocketFD);

		void processGetRequest(int clientSocketFD, HTTPRequest& request);
		void processHeadRequest(int clientSocketFD, HTTPRequest& request);
		void processPostRequest(int clientSocketFD, HTTPRequest& request);
		void processDeleteRequest(int clientSocketFD, HTTPRequest& request);

		void sendChunkedBody(int clientSocketFD, ResponseManager* responseManager);
		void sendChunkedHeaders(int clientSocketFD, ResponseManager* responseManager);
		void sendChunkedResponse(int clientSocketFD, ResponseManager* responseManager);
		void sendCompactFile(int clientSocketFD, ResponseManager* responseManager);

		void handleExcessHeaders(int clientSocketFD);
		void handleExcessURI(int clientSocketFD);
		void handleInvalidRequest(int clientSocketFD, std::string statusCode, std::string reasonPhrase);
		void handleInvalidGetRequest(int clientSocketFD);

		void removeBadClients(int clientSocketFD);
		void removeDisconnectedClients(int clientSocketFD);

		ServerSettings&						getServerSettings();
		std::map<int, ClientManager* >&		getClients();
		std::map<int, ResponseManager* >&	getResponses();
		std::map<int, CGIManager* >&		getCgiMap();
};

#endif