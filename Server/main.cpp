// #include "NonBlockingServer.hpp"
#include <iostream>
#include <exception>
#include "../Parser/ConfigParser.hpp"
#include "../Config/BaseSettings.hpp"
#include "../Parser/LoadSettings.hpp"
#include "ServerArena.hpp"

int main(int argc, char *argv[])
{
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
    try
    {
        ConfigParser configParser(configFile);
        configParser.parse();

        LoadSettings loadSettings(configParser.getConfigTreeRoot());
        loadSettings.loadServers(serverSettings);

        eventHandler = new EventHandler();
        // NonBlockingServer server(9000);
        // std::cout << "Server started on port 9000" << std::endl;
        // server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    ServerArena serverArena(serverSettings, eventHandler);
    serverArena.run();

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
