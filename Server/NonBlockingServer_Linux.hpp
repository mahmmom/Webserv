#ifndef NONBLOCKINGSERVER_LINUX_HPP
#define NONBLOCKINGSERVER_LINUX_HPP

#ifdef __linux__

#include <sys/epoll.h>
#include <map>
#include <vector>
#include "Client.hpp"

#define PORT 9000
#define BACKLOG 10
#define MAXDATASIZE 1024
#define MAX_EVENTS 10

class NonBlockingServer_Linux {
private:
    static int count;
    int server_socket;
    int opt;
    int epoll_fd;
    std::vector<struct epoll_event> events;
    std::map<int, Client> clients;
    std::vector<int> to_remove;

    void setNonBlocking(int socket);
    void acceptNewClient();
    void handleClientRead(int client_socket);
    void handleClientWrite(int client_socket);
    void removeDisconnectedClients();

public:
    NonBlockingServer_Linux(int port);
    void run();
    ~NonBlockingServer_Linux();
};

#endif
#endif

/*
Certainly. I'll provide a detailed explanation of epoll and how its functions work.



epoll (event poll) is an I/O event notification facility introduced in Linux 2.5.44. It's designed to efficiently handle a large number of file descriptors, making it particularly useful for high-performance network servers.

Key epoll functions:

1. epoll_create / epoll_create1
2. epoll_ctl
3. epoll_wait

Let's dive into each of these:

1. epoll_create / epoll_create1:

   Syntax:
   ```c
   int epoll_create(int size);
   int epoll_create1(int flags);
   ```

   Purpose:
   - Creates an epoll instance and returns a file descriptor referring to that instance.
   
   Details:
   - The `size` parameter in `epoll_create` is ignored since Linux 2.6.8 but must be positive.
   - `epoll_create1` is newer and allows setting flags (currently, only EPOLL_CLOEXEC is supported).
   - The returned file descriptor is used in subsequent calls to epoll functions.

2. epoll_ctl:

   Syntax:
   ```c
   int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
   ```

   Purpose:
   - Used to add, modify, or remove file descriptors from the interest list of an epoll instance.

   Parameters:
   - `epfd`: The epoll file descriptor returned by epoll_create.
   - `op`: The operation to perform (EPOLL_CTL_ADD, EPOLL_CTL_MOD, EPOLL_CTL_DEL).
   - `fd`: The file descriptor to operate on.
   - `event`: Specifies the events to be monitored and any associated user data.

   The `epoll_event` structure:
   ```c
   struct epoll_event {
       uint32_t events;    
       epoll_data_t data;  
   };
   ```

   Common event types:
   - EPOLLIN: The associated file is available for read operations.
   - EPOLLOUT: The associated file is available for write operations.
   - EPOLLERR: Error condition happened on the associated file descriptor.
   - EPOLLHUP: Hang up happened on the associated file descriptor.
   - EPOLLET: Sets the Edge Triggered behavior for the associated file descriptor.

3. epoll_wait:

   Syntax:
   ```c
   int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
   ```

   Purpose:
   - Waits for events on the epoll instance.

   Parameters:
   - `epfd`: The epoll file descriptor.
   - `events`: An array where the ready events are returned.
   - `maxevents`: The maximum number of events to return.
   - `timeout`: The maximum wait time in milliseconds (-1 for indefinite).

   Returns:
   - The number of file descriptors ready for the requested I/O.

How epoll works internally:

1. When you create an epoll instance with `epoll_create`, the kernel creates a special file descriptor that represents the epoll instance.

2. You use `epoll_ctl` to add file descriptors to this epoll instance. The kernel maintains an efficient data structure (usually a red-black tree) to keep track of these file descriptors.

3. When you call `epoll_wait`, the kernel checks the monitored file descriptors for the specified events. It efficiently returns only the file descriptors that are ready, avoiding the need to check all file descriptors (as is done in `select` or `poll`).

4. epoll can operate in two modes:
   - Level-triggered (default): An event is reported continuously while the condition is true.
   - Edge-triggered: An event is reported only when the state changes.

Advantages of epoll:

1. Scalability: It performs well with a large number of file descriptors.
2. Efficiency: It only returns the file descriptors that are ready, reducing CPU usage.
3. Flexibility: It allows for both level-triggered and edge-triggered operation.

Use case example:

In a high-performance server:
1. Create an epoll instance with `epoll_create`.
2. Add the server's listening socket to this instance with `epoll_ctl`.
3. Enter a loop that calls `epoll_wait`.
4. When `epoll_wait` returns:
   - If it's an event on the listening socket, accept the new connection and add the new socket to epoll with `epoll_ctl`.
   - If it's an event on a client socket, handle the I/O operation.

This allows the server to efficiently handle many connections without the overhead of checking each connection individually.
*/


/*
Certainly. I'll provide a real example of how to use epoll with a server. This example will demonstrate a simple HTTP server that can handle multiple connections efficiently using epoll.



```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

#define MAXEVENTS 64
#define PORT 8080
#define BUFFER_SIZE 1024

static int make_socket_non_blocking(int sfd) {
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

static int create_and_bind() {
    struct sockaddr_in serv_addr;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        return -1;
    }

    return sfd;
}

int main() {
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    sfd = create_and_bind();
    if (sfd == -1)
        abort();

    s = make_socket_non_blocking(sfd);
    if (s == -1)
        abort();

    s = listen(sfd, SOMAXCONN);
    if (s == -1) {
        perror("listen");
        abort();
    }

    efd = epoll_create1(0);
    if (efd == -1) {
        perror("epoll_create");
        abort();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1) {
        perror("epoll_ctl");
        abort();
    }

    events = calloc(MAXEVENTS, sizeof event);

    while (1) {
        int n, i;

        n = epoll_wait(efd, events, MAXEVENTS, -1);
        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))) {
                fprintf(stderr, "epoll error\n");
                close(events[i].data.fd);
                continue;
            }

            else if (sfd == events[i].data.fd) {
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    infd = accept(sfd, &in_addr, &in_len);
                    if (infd == -1) {
                        if ((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK)) {
                            break;
                        } else {
                            perror("accept");
                            break;
                        }
                    }

                    s = getnameinfo(&in_addr, in_len,
                                    hbuf, sizeof hbuf,
                                    sbuf, sizeof sbuf,
                                    NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        printf("Accepted connection on descriptor %d "
                               "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }

                    s = make_socket_non_blocking(infd);
                    if (s == -1)
                        abort();

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1) {
                        perror("epoll_ctl");
                        abort();
                    }
                }
                continue;
            }
            else {
                int done = 0;

                while (1) {
                    ssize_t count;
                    char buf[BUFFER_SIZE];

                    count = read(events[i].data.fd, buf, sizeof buf);
                    if (count == -1) {
                        if (errno != EAGAIN) {
                            perror("read");
                            done = 1;
                        }
                        break;
                    }
                    else if (count == 0) {
                        done = 1;
                        break;
                    }

                    // Simple HTTP response
                    char *response = "HTTP/1.1 200 OK\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "Content-Length: 13\r\n\r\n"
                                     "Hello, World!";
                    
                    s = write(events[i].data.fd, response, strlen(response));
                    if (s == -1) {
                        perror("write");
                        abort();
                    }
                }

                if (done) {
                    printf("Closed connection on descriptor %d\n",
                           events[i].data.fd);
                    close(events[i].data.fd);
                }
            }
        }
    }

    free(events);
    close(sfd);

    return EXIT_SUCCESS;
}

```

Now, let's break down how this server works with epoll:

1. Server Initialization:
   - We create a socket (`create_and_bind()`), make it non-blocking (`make_socket_non_blocking()`), and start listening.
   - We create an epoll instance with `epoll_create1(0)`.
   - We add the server socket to the epoll instance using `epoll_ctl()` with `EPOLLIN | EPOLLET` flags, which means we're interested in read events and we want edge-triggered notifications.

2. Main Event Loop:
   - The server enters an infinite loop where it calls `epoll_wait()` to wait for events.

3. Handling New Connections:
   - When a new connection arrives, it triggers an event on the server socket.
   - We accept the new connection with `accept()`.
   - We make the new socket non-blocking and add it to the epoll instance.

4. Handling Client Requests:
   - When data is available on a client socket, it triggers an event.
   - We read the data from the socket.
   - In this simple example, we always send back a "Hello, World!" HTTP response.

5. Closing Connections:
   - If a read returns 0 bytes (indicating a closed connection) or if there's an error, we close the socket and remove it from epoll.

Key epoll-related points:

- We use edge-triggered mode (`EPOLLET`), which means we only get notified when the state changes (e.g., when new data arrives, not continuously while there's unread data).
- In edge-triggered mode, it's crucial to read/write all available data when an event is triggered. That's why we have inner while loops for accepting connections and reading data.
- We handle errors by checking for `EPOLLERR` and `EPOLLHUP` events.

To compile and run this server:

1. Save the code to a file, e.g., `epoll_server.c`
2. Compile it: `gcc -o epoll_server epoll_server.c`
3. Run it: `./epoll_server`

The server will start and listen on port 8080. You can test it by opening a web browser and navigating to `http://localhost:8080`, or by using a tool like curl: `curl http://localhost:8080`

This example demonstrates how epoll allows a single-threaded server to efficiently handle multiple connections. It can manage thousands of connections with minimal resource usage, making it suitable for high-performance servers.
*/