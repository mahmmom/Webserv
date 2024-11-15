
#include <iostream>
#include <exception>
#include "../Parser/ConfigParser.hpp"
#include "../Parser/MimeTypesSettings.hpp"
#include "../Settings/BaseSettings.hpp"
#include "../Parser/LoadSettings.hpp"
#include "ServerArena.hpp"
#include "../Logger/Logger.hpp"
#include <signal.h>

/*
    NOTES

        Note 1: Ignoring SIGPIPE is important when we are dealing with chunked responses. With
                Google Chrome, I am not sure about other clients, a sequence of evenets actually 
                happens when a large file is requested. First the client requests a resource, it 
                doesn't yet know that it is a large file that would be sent back in chunked format. 

                So, our server lets it know that in ResponseGenerator::serveChunkedResponse() by 
                adding the "Transfer-Encoding: Chunked" header. Next, we start sending it the 
                response immediately but what Chrome does next is interesting. What we just sent 
                it manages to save somehow, but it also right away disconnects and starts a new 
                connection. According to ChatGPT, this is a different type of connection that is 
                more suitable for chunked-encoded data. It's the client's response to "fuse" the 
                data from the previous connection with the new connection, not our responsibility, 
                but here's the whole point of explaining this. When the client disconnects, we are 
                still sending data; sending chunked data is like a rocket barrage, it keeps firing 
                off data till the whole file is sent. So sometimes, when we send the data and the 
                client disocnnects, the send() function might trigger a SIGPIPE, which if not ignored 
                just shuts down the server abruptly without any warnings or issues! As such, this was 
                pretty hard to figure out. So instead, we just ignore the SIGPIPE and if it gets 
                triggered, then we just issue a warning in Server::sendChunkedBody that the client 
                could simply be re-establishing the connection.
*/

void    signalHandler(int SIGNUM)
{
    if (SIGNUM == SIGINT ||  SIGNUM == SIGTERM)
        running = 0;
    else if (SIGNUM == SIGCHLD)
	{
		int status;
		pid_t pid;
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0) { }
	}
}

/*
    NOTES

        Note 1: 

                SA_RESTART
                    When set, system calls that are interrupted by the signal will be automatically restarted, 
                    instead of returning with an error (EINTR). This is useful when you want to ensure that 
                    system calls (like read(), write(), etc.) are not interrupted by signals, and they resume 
                    automatically if the signal handler is finished.
                    
                    Example:
                    Without SA_RESTART: If a system call like read() is interrupted by a signal, it would return 
                    with an error (EINTR), and you would need to explicitly handle it.

                    With SA_RESTART: If read() is interrupted by a signal, the system call is restarted, so your 
                    program doesn't have to handle the interruption manually.

                SA_NOCLDSTOP
                    This flag prevents the delivery of SIGCHLD signals when child processes stop (i.e., when 
                    they call pause() or exit). Without SA_NOCLDSTOP, SIGCHLD would be delivered for both child 
                    process termination and stopping. But with this flag, only termination of the child process 
                    (exit) will trigger the signal. This is useful to avoid unnecessary signals from child processes 
                    when they're just stopped (for example, if they hit SIGSTOP or SIGTSTP).
*/

void    configureSignalHanlding()
{
    signal(SIGPIPE, SIG_IGN); // Note 1

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // Note 1

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}

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
    MimeTypesSettings mimeTypesSettings("etc/mime.types");
    EventManager*   eventHandler;
    try
    {
        ConfigParser configParser(configFile);
        configParser.parse();

        LoadSettings loadSettings(configParser.getConfigTreeRoot());
        loadSettings.loadServers(serverSettings);

        mimeTypesSettings.parse();

        eventHandler = new EventHandler();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    configureSignalHanlding();

	Logger::init(Logger::DEBUG, "Logs/webserve.log");
    ServerArena serverArena(serverSettings, mimeTypesSettings, eventHandler);
    Logger::log(Logger::DEBUG, "🏟️ Welcome to the Ranchero Grand Server Arena! 🏟️", "main");
    serverArena.run();

	Logger::cleanup();
	delete eventHandler;

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
