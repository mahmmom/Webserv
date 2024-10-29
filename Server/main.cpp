// #include "NonBlockingServer.hpp"
#include <iostream>
#include <exception>
#include "../Parser/ConfigParser.hpp"
#include "../Parser/MimeTypesSettings.hpp"
#include "../Settings/BaseSettings.hpp"
#include "../Parser/LoadSettings.hpp"
#include "ServerArena.hpp"
#include "../Logger/Logger.hpp"

int main(int argc, char *argv[])
{
    signal(SIGPIPE, SIG_IGN);

    std::string configFile;
    if (argc == 1)
        configFile = "etc/nginx.conf";
    else if (argc == 2)
        configFile = argv[1];
    else {
        std::cerr << "To launch server, either: " << std::endl 
                    << "1) ./webserv" << std::endl
                    << "2) ./webserv ./path/to/config/file" << std::endl;
        return (1);
    }

    std::vector<ServerSettings> serverSettings;
    EventManager*   eventHandler;
    MimeTypesSettings mimeTypes("etc/mime.types");
    try
    {
        ConfigParser configParser(configFile);
        configParser.parse();

        LoadSettings loadSettings(configParser.getConfigTreeRoot());
        loadSettings.loadServers(serverSettings);

        mimeTypes.parse();

        eventHandler = new EventHandler();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

	Logger::init(Logger::DEBUG, "Logs/webserve.log");
    ServerArena serverArena(serverSettings, mimeTypes, eventHandler);
    Logger::log(Logger::DEBUG, "🏟️ Welcome to the Ranchero Grand Server Arena! 🏟️", "main");
    serverArena.run();

	Logger::cleanup();
	// delete eventHandler;

    return (0);
}

// int main(int argc, char *argv[])
// {
//     std::string configFile;
//     if (argc == 1)
//         configFile = "etc/nginx.conf";
//     else if (argc == 2)
//         configFile = argv[1];
//     else {
//         std::cerr << "To launch server, either: " << std::endl 
//                     << "1) ./webserv" << std::endl
//                     << "2) ./webserv ./path/to/config/file" << std::endl;
//         return (1);
//     }

//     try
//     {
//         NonBlockingServer server(9000);
//         std::cout << "Server started on port 9000" << std::endl;
//         server.run();
//     }
//     catch (const std::exception& e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return 1;
//     }

//     return (0);
// }
