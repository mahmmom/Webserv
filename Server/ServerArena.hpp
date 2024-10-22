
#ifndef SERVERARENA_HPP
# define SERVERARENA_HPP

#include <vector>
#include "../Config/ServerSettings.hpp"
#include "Server.hpp"
#include "../Config/MimeTypesParser.hpp"


#if defined(__APPLE__) || defined(__FreeBSD__)
    #include "../Events/KqueueManager.hpp"
    typedef KqueueManager EventHandler;
#elif defined(__linux__)
    #include "../Events/EpollManager.hpp"
    typedef EpollManager EventHandler;
#endif

class ServerArena
{
	private:
		std::vector<Server* > 		servers;
		EventManager* 				eventManager;
	public:
		ServerArena(std::vector<ServerSettings>& serverSettings, EventManager* eventManager, MimeTypesParser& mimeTypesParser);
		void initializeServers(std::vector<ServerSettings>& serverSettings, MimeTypesParser& mimeTypesParser);
		void run();
		void manageReadEvent(EventBlock& eventBlock);
		void manageWriteEvent(EventBlock& eventBlock);
};

#endif