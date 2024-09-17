#include "NonBlockingServer.hpp"
#include <iostream>
#include <exception>

int main()
{
    try
    {
        NonBlockingServer server(8080);
        std::cout << "Server started on port 8080" << std::endl;
        server.run();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}