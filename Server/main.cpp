#include "NonBlockingServer.hpp"
#include <iostream>
#include <exception>
#include "ServerManager.hpp"

int main()
{
    try
    {
        // NonBlockingServer server(9000);
        // server.run();
        ServerManager manager(9000);
        std::cout << "Server started on port 9000" << std::endl;
        manager.run();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
