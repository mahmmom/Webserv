
#include "ServerArena.hpp"

ServerArena::ServerArena(std::vector<ServerSettings>& serverSettings, MimeTypesSettings& mimeTypes,
								EventManager* eventManager) : eventManager(eventManager)
{
	initializeServers(serverSettings, mimeTypes);
}

void ServerArena::initializeServers(std::vector<ServerSettings>& serverSettings, MimeTypesSettings& mimeTypes)
{
	for (size_t i = 0; i < serverSettings.size(); i++) {
		Server *server = new Server(serverSettings[i], mimeTypes, eventManager);
		server->launch();
		if (server->getServerSocket() == -1) {
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
		bool isClientFD = servers[i]->getClients().count(eventBlock.fd) > 0;

		if (isClientFD && eventBlock.isEOF && (eventBlock.fd != servers[i]->getServerSocket())) {
			if (!servers[i]->checkClientInServer(eventBlock.fd)) // Note 1
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
	}
}

// void ServerArena::manageReadEvent(EventBlock& eventBlock)
// {
// 	for (size_t i = 0; i < servers.size(); i++) {
// 		if (eventBlock.isEOF && (eventBlock.fd != servers[i]->getServerSocket())) {
// 				std::string type;
// 				if (eventBlock.isRead)
// 					type = "READ";
// 				else if (eventBlock.isWrite)
// 					type = "WRITE";

// 				Logger::log(Logger::DEBUG, "Client with socket fd: " + Logger::intToString(eventBlock.fd) + 
// 					" has triggered EV_EOF " + servers[i]->getServerInterface() +
// 					" for event " + type, "ServerArena::manageReadEvent");
// 				servers[i]->removeDisconnectedClients(eventBlock.fd);
// 				break ;
// 		}
// 		else if (eventBlock.isRead && (eventBlock.fd == servers[i]->getServerSocket())) {
// 			servers[i]->acceptNewClient();
// 			break ;
// 		}
// 		else if (eventBlock.isRead && (eventBlock.fd != servers[i]->getServerSocket())) {
// 			servers[i]->handleClientRead(eventBlock.fd);
// 			break ;
// 		}
// 	}
// }

void ServerArena::manageWriteEvent(EventBlock& eventBlock)
{
	for (size_t i = 0; i < servers.size(); i++) {
		if (eventBlock.isWrite) {
			servers[i]->handleClientWrite(eventBlock.fd);
			break ;
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
    while (true)
    {
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
				// std::cout << "djfksfjkldsjfkldsjfkldsjfkldsjfkds\n";
                manageWriteEvent(eventBlock);
            }
        }

		// for (int i = 0; i < nev; i++) {
        //     EventBlock eventBlock = eventManager->getEvent(i);
		// 	// THIS MIGHT HAVE TO BE SPLIT INTO TWO FOR LOOPS UPON CGI IMPLEMENTATION
		// 	// WHERE ONE LOOP HANDLES ALL THE READ EVENTS FIRST AND ANOTHER HANDLES 
		// 	// ONE THEN HANDLES ALL THE WRITE EVENTS
        //     if (eventBlock.isRead || eventBlock.isEOF) {
        //         manageReadEvent(eventBlock);
        //     }
        // }
		// for (int i = 0; i < nev; i++) {
        //     EventBlock eventBlock = eventManager->getEvent(i);
		// 	// THIS MIGHT HAVE TO BE SPLIT INTO TWO FOR LOOPS UPON CGI IMPLEMENTATION
		// 	// WHERE ONE LOOP HANDLES ALL THE READ EVENTS FIRST AND ANOTHER HANDLES 
		// 	// ONE THEN HANDLES ALL THE WRITE EVENTS
        //     if (eventBlock.isWrite) {
		// 		// std::cout << "djfksfjkldsjfkldsjfkldsjfkldsjfkds\n";
        //         manageWriteEvent(eventBlock);
        //     }
        // }
   }
}
