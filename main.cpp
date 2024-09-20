#include "Webserv.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " [configuration file]" << std::endl;
        return 1;
    }

    try
    {
        Webserv server(argv[1]);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}