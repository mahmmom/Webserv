#include <iostream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>

class Server
{
    private:
        int server_fd;
        std::vector<pollfd> poll_fds;

        void setNonBlocking(int fd)
        {
            int flags = fcntl(fd, F_GETFL, 0);
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }

    public:
        Server(int port)
        {
            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd < 0)
            {
                std::cerr << "Error creating socket" << std::endl;
                exit(1);
            }
            int opt = 1;
            if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
            {
                std::cerr << "Error setting socket options" << std::endl;
                exit(1);
            }
            sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY;
            server_addr.sin_port = htons(port);

            if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            {
                std::cerr << "Error binding socket" << std::endl;
                exit(1);
            }

            if (listen(server_fd, 10) < 0)
            {
                std::cerr << "Error listening on socket" << std::endl;
                exit(1);
            }

            setNonBlocking(server_fd);

            pollfd server_pollfd = {server_fd, POLLIN, 0};
            poll_fds.push_back(server_pollfd);
        }

        void run()
        {
            while (true)
            {
                int activity = poll(poll_fds.data(), poll_fds.size(), -1);

                if (activity < 0)
                {
                    std::cerr << "poll error" << std::endl;
                    break;
                }

                for (size_t i = 0; i < poll_fds.size(); ++i)
                {
                    if (poll_fds[i].revents & POLLIN)
                    {
                        if (poll_fds[i].fd == server_fd)
                        {
                            // Handle new connection
                            int client_fd = accept(server_fd, NULL, NULL);
                            if (client_fd >= 0)
                            {
                                setNonBlocking(client_fd);
                                pollfd client_pollfd = {client_fd, POLLIN, 0};
                                poll_fds.push_back(client_pollfd);
                                std::cout << "New client connected: " << client_fd << std::endl;
                            }
                        }
                        else
                        {
                            // Handle client read
                            char buffer[1024] = {0};
                            int valread = read(poll_fds[i].fd, buffer, 1024);
                            if (valread <= 0)
                            {
                                // Client disconnected or error
                                close(poll_fds[i].fd);
                                poll_fds.erase(poll_fds.begin() + i);
                                std::cout << "Client disconnected" << std::endl;
                            }
                            else
                            {
                                std::cout << "Received: " << buffer;
                                // Echo back to client
                                send(poll_fds[i].fd, buffer, strlen(buffer), 0);
                            }
                        }
                    }
                }
            }
        }

        ~Server()
        {
            close(server_fd);
        }
};

int main()
{
    try
    {
        Server server(8080);
        std::cout << "Server started on port 8080" << std::endl;
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}