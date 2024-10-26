
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
		eventManager->registerEvent(server->getServerSocket(), READ);
		servers.push_back(server);
	}
}

void ServerArena::manageReadEvent(EventBlock& eventBlock)
{
	for (size_t i = 0; i < servers.size(); i++) {
		if (eventBlock.isRead && (eventBlock.fd == servers[i]->getServerSocket())) {
			servers[i]->acceptNewClient();
			break ;
		}
		else if (eventBlock.isRead && (eventBlock.fd != servers[i]->getServerSocket())) {
			servers[i]->handleClientRead(eventBlock.fd);
			break ;
		}
	}
}

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

        int nev = eventManager->eventListener();

        for (int i = 0; i < nev; i++) {
            EventBlock eventBlock = eventManager->getEvent(i);
			// THIS MIGHT HAVE TO BE SPLIT INTO TWO FOR LOOPS UPON CGI IMPLEMENTATION
			// WHERE ONE LOOP HANDLES ALL THE READ EVENTS FIRST AND ANOTHER HANDLES 
			// ONE THEN HANDLES ALL THE WRITE EVENTS
            if (eventBlock.isRead) {
                manageReadEvent(eventBlock);
            }
            else if (eventBlock.isWrite) {
				// std::cout << "djfksfjkldsjfkldsjfkldsjfkldsjfkds\n";
                manageWriteEvent(eventBlock);
            }
        }
   }
}
