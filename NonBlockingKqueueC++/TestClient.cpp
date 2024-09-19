#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <fcntl.h>
#define PORT 9000

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: programe_name host_ip" << std::endl;
		exit (1);
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	if (inet_pton(AF_INET, argv[1], &(address.sin_addr)) == 0) {
		perror("Inet_pton: ");
		exit(EXIT_FAILURE);
	}
	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	int	sock_fd;
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket: ");
		exit(EXIT_FAILURE);
	}

	if (connect(sock_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
		perror("Connect: ");
		exit(EXIT_FAILURE);
	}

    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

	char				server_str_address[INET_ADDRSTRLEN];
	inet_ntop(address.sin_family, &(address.sin_addr), server_str_address, sizeof(server_str_address));
	std::cout << "Client connected to server: " << server_str_address << std::endl;


    std::time_t start_time = std::time(nullptr); // Record the start time
    bool receiving_started = false; // Flag to check if receiving has started
	bool run_once = false;

	std::cout << "Enter your message: ";
	std::string	user_input;
	std::getline(std::cin, user_input);
	if (send(sock_fd, user_input.c_str(), user_input.size(), 0) == -1) {
			perror("Send: ");
	}

	while (true) {

		std::time_t current_time = std::time(nullptr);
        if (!receiving_started && difftime(current_time, start_time) >= 10) {
            receiving_started = true; // Start receiving after 10 seconds
        }
		if (receiving_started) {
			if (run_once == false) {
				// Do-while loop to empty the buffer of the socket
				std::cout << "Emptying process has commenced" << std::endl;
				char buffer[21];
				ssize_t recv_len;
				int		i = 0;
				do {
					recv_len = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
					if (recv_len > 0) {
						buffer[recv_len] = '\0';
					} else if (recv_len == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) {
								perror("WhackReceive: ");
								break;
						} else {
							perror("<>Receive: ");
							break;
						}
					}
					buffer[recv_len] = '\0';
					std::cout << i++ << ", " << recv_len << ", " << buffer << std::endl;
				} while (1); // Continue receiving while there's data
				std::cout << std::endl;
				run_once = true;
			}
			else {
				std::cout << "Enter your message: ";
				std::string	user_input;
				std::getline(std::cin, user_input);
				if (send(sock_fd, user_input.c_str(), user_input.size(), 0) == -1) {
					perror("Send: ");
				}
				usleep(1000);	// give the server enough time to processes the data
								// and send it such that our client can receive it
				char buffer[1024];
				ssize_t recv_len;
				recv_len = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
				if (recv_len > 0) {
					buffer[recv_len] = '\0'; // Null-terminate the received string
					std::cout << "Received: " << buffer << std::endl;
				} else if (recv_len == -1) {
					perror("Good_Receive: ");
				}
			}
		}
	}
	close(sock_fd);
	return (0);
}
