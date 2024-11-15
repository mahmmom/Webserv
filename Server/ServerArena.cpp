
#include "ServerArena.hpp"

int running = 1;

ServerArena::ServerArena(std::vector<ServerSettings>& serverSettings, MimeTypesSettings& mimeTypes,
								EventManager* eventManager) : eventManager(eventManager)
{
	initializeServers(serverSettings, mimeTypes);
}

void ServerArena::pseudoDestructor()
{
	for (size_t i = 0; i < servers.size(); i++) {
		delete (servers[i]);
	}
	servers.clear();
}

/*
	NOTES

		Note 1:	One scenario in which this might be executed is if you try to launch multiple servers on 
				the same port, what happens is that binding the socket fails. Thus, a value of -1 is returned 
				by the Server::getServerSocket() and that server is deleted. So only the first instance of a 
				server bound to that port would run. 
*/
void ServerArena::initializeServers(std::vector<ServerSettings>& serverSettings, MimeTypesSettings& mimeTypes)
{
	for (size_t i = 0; i < serverSettings.size(); i++) {
		Server *server = new Server(serverSettings[i], mimeTypes, eventManager);
		server->launch();
		if (server->getServerSocket() == -1) { // Note 1
			Logger::log(Logger::ERROR, "Failed to create server", "ServerArena::initializeServers");
			delete server;
			continue ;
		}
		eventManager->registerEvent(server->getServerSocket(), READ);
		servers.push_back(server);
	}
	displayServersRegistry();
}

void ServerArena::displayServersRegistry()
{
	std::cout << "\n🖥 Server Registry:\n│\n";
	for (size_t i = 0; i < servers.size(); i++)
	{
		std::stringstream ss;
        ss << servers[i]->getServerPort();
		if (i < servers.size() - 1)
			std::cout << "├── \033[0;36m🔵 Listening on Port \033[1;33m" << ss.str() << "\033[0;36m through interface "
					<< servers[i]->getServerInterface() << " 🗄️\033[0m\n│\n";
		else
			std::cout << "└── \033[0;36m🔵 Listening on Port \033[1;33m" << ss.str() << "\033[0;36m through interface "
					<< servers[i]->getServerInterface() << " 🗄️\033[0m\n";
	}
}

/*
	NOTES:

		Note 1:
			The purpose of this boolean is two-fold. First, it helps relate the read Event to the right 
			server where that client is stored if we have multiple servers set-up in our Nginx 
			configuration file. Second, it separates cgi read events from all the other non-cgi read 
			events on a server.

		Note 2:
			Why do we need this check? Well the reason behind it mainly happens when a client requests 
			a file that we will be sending back in chunked encoding. When a client requests such a file, 
			we respond with a "Transfer-Encoding: chunked" header field. The behavior of Google Chrome when 
			that happens is that it disconnects, opens a new connection designed for chunked encoding and then 
			continues to recieve the data on that new connection. But when that happens, it is actually the WRITE 
			event which triggers the EOF. So when we proceed to disconnect that client via the close function in 
			Server::removeDisconnectedClients (see Note 2 of that function for more details), the READ event is 
			already in the queue (close only deletes future occurences of that event) and so, since Google Chrome 
			disconnected, that previously registered READ event is still there, and it also wants to go away, but 
			it wants to announce that it is going away by trigger its own EOF because Google Chrome has disconnected. 
			Extra: The way the READ event announces that is by an triggering an instance of its own (with the EV_EOF 
			flag) and making recv() return 0. Yeah, that's how it works.
			So anyways, in that case, even though we cleared the clients map and closed the connection in 
			Server::removeDisconnectedClients, the event is still registered in the queue and so, and when it announces 
			that it wants to leave, it would trigger the warning found in Server::removeDisconnectedClients where we 
			attempt to we log a warning stating that we have already disconneccted this client before. So that's the 
			point of this if statement, if we have already erased the client, then just let these trailer, extra, 
			unecessary events flush out on their own, we don't want to process them.

			So in a nutshell, if the WRITE event triggered an EOF, we will disconnect the client but when the READ event 
			follows suite and wants to trigger its own EOF, we will stop it before that happens and just ignore that event 
			because we have already disconnected the client, we do not want to attempt to disconnect them again. So, do 
			not process that event, just let it flush out.
*/
void ServerArena::manageReadEvent(EventBlock& eventBlock)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		bool isClientFD = servers[i]->getClients().count(eventBlock.fd) > 0; // Note 1
		bool isCgiFD = servers[i]->getCgiMap().count(eventBlock.fd) > 0; // Note 1

		if (isClientFD && eventBlock.isEOF && (eventBlock.fd != servers[i]->getServerSocket())) {
			if (!servers[i]->checkClientInServer(eventBlock.fd)) // Note 2
			{
				Logger::log(Logger::DEBUG, "Client with socket fd: " + Logger::intToString(eventBlock.fd) + 
					" has triggered EV_EOF " + servers[i]->getServerInterface(), "ServerArena::manageReadEvent");
				servers[i]->removeDisconnectedClients(eventBlock.fd);
				break ;
			}
		}
		else if (eventBlock.isRead && (eventBlock.fd == servers[i]->getServerSocket())) {
			servers[i]->acceptNewClient();
			break ;
		}
		else if (isClientFD && eventBlock.isRead && (eventBlock.fd != servers[i]->getServerSocket())) {
			if (!servers[i]->checkClientInServer(eventBlock.fd))
			{
				Logger::log(Logger::DEBUG, "Client with socket fd: " + Logger::intToString(eventBlock.fd) + 
					" has sent data to server " + servers[i]->getServerInterface(), "ServerArena::manageReadEvent");
				servers[i]->handleClientRead(eventBlock.fd);
				break ;
			}
		}
		else if (isCgiFD /* && eventBlock.isRead && (eventBlock.fd != servers[i]->getServerSocket())*/) {
			Logger::log(Logger::DEBUG, "processing a cgi read event", "ServerArena::manageReadEvent");
			servers[i]->handleCgiOutput(eventBlock.fd);
			break ;
		}
	}
}

/*
	NOTES:

		Note 1:	In here, we are checking if the write eventBlock.fd belongs to that 
				particular server but through the responses map (which maps an fd to 
				a server) instead of checking the clients map. That is because the 
				error handling responses defined in the server class, like 
				server::handleInvalidRequest or server::handleExcessHeaders actually 
				remove that client through the removedBadClients function but keep 
				the fd unclosed and send one final response to that client by setting 
				closeConnection to true in the responseManager constructor. So if we 
				try to map a read event to the right server by correlating the 
				eventBlock.fd to the clients map, it would SEGFAULT in those cases 
				were a server error-handling function is triggered, because that 
				client has been erased from the clients map by the time the write 
				event is managed.
*/
void ServerArena::manageWriteEvent(EventBlock& eventBlock)
{
	for (size_t i = 0; i < servers.size(); i++) {
		if (eventBlock.isWrite) {
			bool forCurrentServer = servers[i]->getResponses().count(eventBlock.fd) > 0;

			if (forCurrentServer && eventBlock.isWrite) {
				servers[i]->handleClientWrite(eventBlock.fd);
				break ;
			}
		}
	}
}

void ServerArena::manageTimeouts()
{
	for (size_t i = 0; i < servers.size(); i++) {
		servers[i]->checkTimeouts();
	}
	lastTimeoutCheck = std::time(0);
}

void ServerArena::run()
{
	lastTimeoutCheck = std::time(0);

    while (running)
    {
		if (!running)
			break ;
	
		manageTimeouts();

		Logger::log(Logger::DEBUG, "⌛ Waiting for events ⌛", "ServerArena::run");

        int nev = eventManager->eventListener();

        for (int i = 0; i < nev; i++) {
            EventBlock eventBlock = eventManager->getEvent(i);
			// THIS MIGHT HAVE TO BE SPLIT INTO TWO FOR LOOPS UPON CGI IMPLEMENTATION
			// WHERE ONE LOOP HANDLES ALL THE READ EVENTS FIRST AND ANOTHER HANDLES 
			// ONE THEN HANDLES ALL THE WRITE EVENTS
            if (eventBlock.isRead || eventBlock.isEOF) {
                manageReadEvent(eventBlock);
            }
            else if (eventBlock.isWrite) {
                manageWriteEvent(eventBlock);
            }
        }
   }

	Logger::log(Logger::DEBUG, "🤠 Ranchero has been shut down.", "ServerArena::run");
	std::cout << "\n🤠\033[1;35m Ranchero has been shut down.\033[0m" << std::endl;
	pseudoDestructor();
}
