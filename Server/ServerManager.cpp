// ServerManager.hpp
// ServerManager.cpp

#include "ServerManager.hpp"
#include <iostream>

ServerManager::ServerManager(int port) {
    #if defined(__APPLE__) || defined(__FreeBSD__)

        std::cout << "Starting macOS server..." << std::endl;
        server = new NonBlockingServer_MacOS(port);
    #elif __linux__
        std::cout << "Starting Linux server..." << std::endl;
        server = new NonBlockingServer_Linux(port);
    #endif
}

void ServerManager::run() {
    server->run();
}

ServerManager::~ServerManager() {
    delete server;
}

/*
Certainly! I'll provide an overview of epoll, poll, kqueue, and select, their general uses, and the key differences between them. These are all I/O multiplexing mechanisms used in network programming, but they have different characteristics and are used on different operating systems.



1. select:

General use:
- The oldest and most portable of the four mechanisms.
- Used for monitoring multiple file descriptors to see if they are ready for I/O operations.

Characteristics:
- Available on almost all UNIX-like systems and Windows.
- Has a limit on the number of file descriptors it can monitor (typically 1024).
- Relatively slow for large numbers of connections.

2. poll:

General use:
- Similar to select, used for monitoring multiple file descriptors.
- An improved version of select.

Characteristics:
- Available on most UNIX-like systems.
- No built-in limit on the number of file descriptors.
- Slightly better performance than select for large numbers of connections.

3. epoll:

General use:
- A Linux-specific I/O event notification facility.
- Designed for handling a large number of concurrent connections efficiently.

Characteristics:
- Only available on Linux systems.
- Highly scalable and efficient for a large number of connections.
- Uses an edge-triggered or level-triggered interface.
- Provides better performance than select and poll for a large number of file descriptors.

4. kqueue:

General use:
- A scalable event notification interface.
- Used in BSD-derived systems including macOS and FreeBSD.

Characteristics:
- Available on BSD-derived systems.
- Highly efficient and scalable, similar to epoll.
- Can monitor various types of events, not just file descriptors.
- Provides fine-grained control over event notifications.

Key Differences:

1. Availability:
   - select: Most widely available (UNIX, Linux, Windows)
   - poll: Available on most UNIX-like systems
   - epoll: Linux-specific
   - kqueue: BSD-derived systems (including macOS)

2. Scalability:
   - select and poll: Less efficient for a large number of connections
   - epoll and kqueue: Highly scalable, efficient for many connections

3. Performance:
   - select and poll: Lower performance for many connections
   - epoll and kqueue: Higher performance, especially with many connections

4. Complexity:
   - select and poll: Simpler to use and understand
   - epoll and kqueue: More complex, but offer more features and better performance

5. Flexibility:
   - select and poll: Limited to I/O events on file descriptors
   - epoll: Focused on I/O events, but with more options
   - kqueue: Can monitor various types of events beyond just I/O

6. Portability:
   - select: Most portable
   - poll: Fairly portable across UNIX-like systems
   - epoll and kqueue: Less portable, specific to certain operating systems

In general, for new applications:
- On Linux, epoll is preferred for its performance and scalability.
- On BSD-derived systems (including macOS), kqueue is the go-to choice.
- For maximum portability, select or poll might be used, possibly with a library that abstracts the differences between systems.

Many modern networking libraries and frameworks abstract these differences, allowing developers to write portable code that uses the most efficient mechanism available on each platform.
*/