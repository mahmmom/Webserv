#include <stdio.h>
#include <string.h>
#include <sys/_endian.h>
#include <sys/_types/_socklen_t.h>
#include <sys/_types/_ssize_t.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define BACKLOG 10

/*
                getaddrinfo();
                socket();
                bind();
                listen();
*/ 


int main(int argc, char *argv[])
{
    int server_fd, client_fd;
    struct addrinfo hints, *res;
    socklen_t add_size;
    int yes = 1;
    struct sockaddr_storage their_add;
    char str[1024] = {0};

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "8080", &hints, &res) < 0)
    {
        printf("Getaddrinfo\n");
        return 1;
    }

    if ((server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
    {
        printf("Socket\n");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
    {
        printf("Socket options\n");
        return 1;
    }

    if (bind(server_fd, res->ai_addr, res->ai_addrlen))
    {
        printf("Bind\n");
        return 1;
    }

    if (listen(server_fd, BACKLOG) < 0)
    {
        printf("Listen\n");
        return 1;
    }

    add_size = sizeof(their_add);
    if((client_fd = accept(server_fd, (struct sockaddr *)&their_add, &add_size)) < 0)
    {
        printf("Accept\n");
        return 1;
    }

    if (recv(client_fd, str, 1023, 0) < 0)
    {
        printf("recv\n");
        return 1;
    }

    printf("received msg = %s\n", str);

    char *msg = "You Did it";

    if (send(client_fd, msg, strlen(msg), 0) < 0)
    {
        printf("Send\n");
        return 1;
    }

    close(client_fd);
    close(server_fd);
}
