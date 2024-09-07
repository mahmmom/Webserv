/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char str[1024] = {0};
    char *reply = "I got you khalas";

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL) 
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    while (1)
    {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
    
        if (!fork())
        {
            close(sockfd); // child doesn't need the listener
            while (1)
            {
                memset(str, 0, sizeof(str)); // Clear the buffer
                int numbytes = recv(new_fd, str, sizeof(str) - 1, 0);
                if (numbytes <= 0)
                {
                    // Handle the end of the connection or error
                    if (numbytes == 0)
                        printf("server: client disconnected\n");
                    else
                        perror("recv");
                    break;
                }
                str[numbytes] = '\0'; // Null-terminate the received string
                printf("msg received: %s", str);
                send(new_fd, reply, strlen(reply), 0);
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }
}


/*
======================================= :SIG ACTION: ========================================

sigchld_handler:
This function was defined earlier and is responsible for cleaning up zombie child processes after they terminate.
A zombie process occurs when a process has finished execution, but its parent hasn't read its exit status using wait() or waitpid().
This leads to the child process lingering in the process table as a zombie.

sa.sa_handler = sigchld_handler:
This sets the sa_handler field of the sigaction structure to the custom signal handler sigchld_handler.
This handler will be invoked when the SIGCHLD signal is sent, i.e., when a child process terminates.

sigemptyset(&sa.sa_mask):
This function initializes the signal set sa_mask to exclude all signals.
The sa_mask is used to specify which signals should be blocked while the signal handler is executing.
Here, we are not blocking any additional signals.

sa.sa_flags = SA_RESTART:
The SA_RESTART flag ensures that system calls that are interrupted by this signal are automatically restarted.
For example, if a recv() system call is interrupted by a SIGCHLD, it will restart automatically instead of failing with EINTR.

sigaction(SIGCHLD, &sa, NULL):
This sets up the signal handler for SIGCHLD signals.
The sigaction() system call is more powerful than the older signal() function because it allows fine-grained control of signal handling.

First parameter SIGCHLD: This specifies the signal we want to handle (SIGCHLD in this case, for when a child process terminates).
Second parameter &sa: This points to the sigaction structure, which contains the new action to be associated with SIGCHLD.
Third parameter NULL: This can optionally point to a sigaction structure to store the old signal handling behavior.
Here, it's NULL, meaning we don't care about the old action.

Error Handling:
perror("sigaction"): If sigaction() fails, this will print an error message to stderr.
exit(1): Terminates the program if the call to sigaction() fails.
*/