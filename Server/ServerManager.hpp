#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#ifdef __APPLE__
    #include "NonBlockingServer_MacOS.hpp"
#elif __linux__
    #include "NonBlockingServer_Linux.hpp"
#else
    #error "Unsupported platform"
#endif

class ServerManager {
private:
    #ifdef __APPLE__
        NonBlockingServer_MacOS* server;
    #elif __linux__
        NonBlockingServer_Linux* server;
    #endif

public:
    ServerManager(int port);
    void run();
    ~ServerManager();
};

#endif
