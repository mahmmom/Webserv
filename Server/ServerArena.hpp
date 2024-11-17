
#ifndef SERVERARENA_HPP
# define SERVERARENA_HPP

#include <vector>
#include <sstream>
#include "../Settings/ServerSettings.hpp"
#include "../Parser/MimeTypesSettings.hpp"
#include "Server.hpp"
#include "../Logger/Logger.hpp"
#include <ctime>

#if defined(__APPLE__) || defined(__FreeBSD__)
    #include "../Events/KqueueManager.hpp"
    typedef KqueueManager EventHandler;
#elif defined(__linux__)
    #include "../Events/EpollManager.hpp"
    typedef EpollManager EventHandler;
#endif

extern int running;

class ServerArena
{
	private:
		std::vector<Server* > 		servers;
		EventManager* 				eventManager;
		std::time_t					lastTimeoutCheck;
		std::time_t					lastCgiTimeoutCheck;
	public:
		ServerArena(std::vector<ServerSettings>& serverSettings, MimeTypesSettings& mimeTypes, EventManager* eventManager);
		void displayServersRegistry();
		void manageTimeouts();
		void run();
		void initializeServers(std::vector<ServerSettings>& serverSettings, MimeTypesSettings& mimeTypes);
		void manageReadEvent(EventBlock& eventBlock);
		void manageWriteEvent(EventBlock& eventBlock);
		void pseudoDestructor();
};

#endif