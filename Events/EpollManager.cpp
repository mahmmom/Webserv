
#if defined(__linux__)

#include "EpollManager.hpp"

EpollManager::EpollManager() : events(DEFAULT_NUM_OF_EVENTS)
{
    epfd = epoll_create(DEFAULT_NUM_OF_EVENTS);

    if (epfd == -1) {
        perror("Epoll: ");
        exit(EXIT_FAILURE);
    }
}

EpollManager::~EpollManager()
{
    close(epfd);
}

// void EpollManager::registerEvent(int fd, EventType event)
// {
//     struct epoll_event epollEvent;
//     epollEvent.data.fd = fd;
//     int op;
//     std::string eventType;
//     std::map<int, int>::iterator it = registeredEvents.find(fd);

//     if (it == registeredEvents.end()) {
//         op = EPOLL_CTL_ADD;
//         registeredEvents[fd] = 0;
//         eventType = (event == READ) ? "READ (new)" : "WRITE (new)";
//     } 
//     else {
//         op = EPOLL_CTL_MOD;
//         eventType = (event == READ) ? "READ (mod)" : "WRITE (mod)";
//     }

//     if (event == READ) {
//         registeredEvents[fd] |= EPOLLIN | EPOLLRDHUP;
//     } 
//     else {
//         registeredEvents[fd] |= EPOLLOUT;
//     }

//     epollEvent.events = registeredEvents[fd];

//     if (epoll_ctl(epfd, op, fd, &epollEvent) < 0) {
//         Logger::log(Logger::ERROR, "Failed to register " + eventType + " event for fd " + 
// 			Logger::intToString(fd) + ": " + strerror(errno), "EpollManager::registerEvent");

//     } else {
//         Logger::log(Logger::DEBUG, "Registered new " + eventType + " event for fd " + 
// 			Logger::intToString(fd), "EpollManager::registerEvent");
//     }
// }

// void EpollManager::registerEvent(int fd, EventType event)
// {
//     struct epoll_event epollEvent;
//     epollEvent.data.fd = fd;
//     epollEvent.events = (event == READ) ? (EPOLLIN | EPOLLRDHUP) : EPOLLOUT;
    
//     int op;
//     std::string eventType;
//     std::map<int, int>::iterator it = registeredEvents.find(fd);
    
//     if (it == registeredEvents.end()) {
//         op = EPOLL_CTL_ADD;
//         eventType = (event == READ) ? "READ (new)" : "WRITE (new)";
//         registeredEvents[fd] = epollEvent.events;
//     }
//     else {
//         op = EPOLL_CTL_MOD;
//         eventType = (event == READ) ? "READ (mod)" : "WRITE (mod)";
//         registeredEvents[fd] = epollEvent.events;
//     }

//     if (epoll_ctl(epfd, op, fd, &epollEvent) < 0) {
//         Logger::log(Logger::ERROR, "Failed to register " + eventType + " event for fd " +
//             Logger::intToString(fd) + ": " + strerror(errno), "EpollManager::registerEvent");
//     } else {
//         Logger::log(Logger::DEBUG, "Registered new " + eventType + " event for fd " +
//             Logger::intToString(fd), "EpollManager::registerEvent");
//     }
// }

/*
    NOTES

        Note 1:
            (EPOLLIN | EPOLLRDHUP), you're telling epoll, "I want to be notified 
            if data is available for reading (EPOLLIN) or if the connection has 
            been half-closed (EPOLLRDHUP)." EPOLLRDHUP triggers when the remote 
            side of the socket has performed an orderly shutdown (like closing 
            the connection). More specifically, it indicates that the connection 
            is half-closed: you can no longer send data (write), but you can still 
            receive data (read). If you attempt to read, you'll likely get EOF 
            (end of file), signaling no more data is available.

            If event != READ, we just go with EPOLLOUT. That’s the flag you need 
            when you want to write something to the client, basically sending the 
            HTTP response back to their HTTP request.

    Note 2:

            In Kqueue, if you want to modify the events, you can just add more event flags to 
            the already-registered file descriptor, and the system will keep track of all the 
            event types for that file descriptor. So, with kqueue, you don't need to explicitly 
            combine flags or worry about overwriting them. If the file descriptor is already 
            registered, you can add new events to it.

            In epoll, however, event flags are more tightly tied to the registration of the file 
            descriptor. When you register a file descriptor with epoll, you provide a set of event 
            flags (e.g., EPOLLIN, EPOLLOUT), but these flags are overwritten when you modify the 
            registration. To add new event flags without losing the old ones, you need to combine 
            the flags manually. This is why you use the bitwise OR (|=) operator to add (newEvents) 
            to the existing flags. Without combining, you might accidentally overwrite existing 
            events with new ones.
*/
void EpollManager::registerEvent(int fd, EventType event)
{
    struct epoll_event epollEvent;
    epollEvent.data.fd = fd;
    
    int newEvents = (event == READ) ? (EPOLLIN | EPOLLRDHUP) : EPOLLOUT; // Note 1
    int op;
    std::string eventType;
    
    std::map<int, int>::iterator it = registeredEvents.find(fd);
    if (it == registeredEvents.end()) {
        op = EPOLL_CTL_ADD;
        eventType = (event == READ) ? "READ (new)" : "WRITE (new)";
        registeredEvents[fd] = newEvents;
    }
    else {
        op = EPOLL_CTL_MOD;
        eventType = (event == READ) ? "READ (mod)" : "WRITE (mod)";
        registeredEvents[fd] |= newEvents;  // Note 2 (COMBINE the flags, don't overwrite!)
    }
    
    epollEvent.events = registeredEvents[fd];  // Use the COMBINED flags
    
    if (epoll_ctl(epfd, op, fd, &epollEvent) < 0) {
        Logger::log(Logger::ERROR, "Failed to register " + eventType + " event for fd " +
            Logger::intToString(fd) + ": " + strerror(errno), "EpollManager::registerEvent");
    } else {
        Logger::log(Logger::DEBUG, "Registered new " + eventType + " event for fd " +
            Logger::intToString(fd), "EpollManager::registerEvent");
    }
}

// void EpollManager::deregisterEvent(int fd, EventType event)
// {
//     std::map<int, int>::iterator it = registeredEvents.find(fd);

//     if (it == registeredEvents.end()) {
//         Logger::log(Logger::WARN, "Attempted to deregister event for non-registered fd " + 
//             Logger::intToString(fd), "EpollManager::deregisterEvent");
//         return ;
//     }

//     std::string eventType = (event == READ) ? "READ" : "WRITE";
    
//     if (event == READ) {
//         it->second &= ~(EPOLLIN | EPOLLRDHUP);
//     } else {
//         it->second &= ~EPOLLOUT;
//     }

//     if (it->second == 0) {
//         epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
//         registeredEvents.erase(it);
//         Logger::log(Logger::DEBUG, "Removed all events events for fd " +
//             Logger::intToString(fd), "EpollManager::deregisterEvent");
//     } 
//     else {
//         struct epoll_event epollEvent;
//         epollEvent.data.fd = fd;
//         epollEvent.events = it->second;
//         if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &epollEvent) < 0) {
//             Logger::log(Logger::ERROR, "Failed to deregister " + eventType + " event for fd " + 
// 			    Logger::intToString(fd) + ": " + strerror(errno), "EpollManager::deregisterEvent");
//         }
//         else {
//             Logger::log(Logger::DEBUG, "Deregistered " + eventType + " event for fd " + 
//                 Logger::intToString(fd), "EpollManager::deregisterEvent");
//         }
//     }
// }

/*
    Notes:

        Note 1: it->second &= ~EPOLLOUT; this is the line that actually deregisters the
                WRITE event. This is a bitwise NOT operation on the EPOLLOUT flag, which 
                inverts all bits except for the EPOLLOUT bit. So it effectively removes 
                the "EPOLLOUT mask flag" and keeps all the other event flags registered. 
                Then, it checks are there actually any other events remaining? 
                
                If no, which satisifies if (it->second == 0), then there are no more 
                events to monitor for that file descriptor (fd). Then do 
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL); this line deregisters the 
                file descriptor (fd) from epoll, effectively removing it from monitoring.
                registeredEvents.erase(it): Finally, the file descriptor is removed from 
                the registeredEvents map because it's no longer being tracked.

                If yes, After clearing EPOLLOUT, if there are still events left (like EPOLLIN), 
                you still need to call epoll_ctl to update the event set for that file 
                descriptor in epoll. This is because:

                    * epoll_ctl(EPOLL_CTL_DEL) deletes a file descriptor from epoll entirely 
                    (removes all events associated with it).

                    * epoll_ctl(EPOLL_CTL_MOD) is used to update the existing registration 
                    of a file descriptor, changing the events it's monitored for.
*/
void EpollManager::deregisterEvent(int fd, EventType event)
{
    std::map<int, int>::iterator it = registeredEvents.find(fd);
    if (it == registeredEvents.end()) {
        Logger::log(Logger::WARN, "Attempted to deregister event for non-registered fd " +
            Logger::intToString(fd), "EpollManager::deregisterEvent");
        return;
    }

    // When deregistering READ, just remove the whole fd from tracking
    if (event == READ) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        registeredEvents.erase(it);  // Remove the ENTIRE fd from our tracking
        Logger::log(Logger::DEBUG, "Deregistered READ event for fd " +
            Logger::intToString(fd), "EpollManager::deregisterEvent");
        return;
    }

    it->second &= ~EPOLLOUT; // Note 1
    if (it->second == 0) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        registeredEvents.erase(it);
    }
    else {
        struct epoll_event epollEvent;
        epollEvent.data.fd = fd;
        epollEvent.events = it->second;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &epollEvent);
    }
    Logger::log(Logger::DEBUG, "Deregistered WRITE event for fd " +
        Logger::intToString(fd), "EpollManager::deregisterEvent");
}

int EpollManager::eventListener()
{
    int nev = epoll_wait(epfd, &events[0], static_cast<int>(events.size()), EPOLL_TIMEOUT_INTERVAL);

    if (nev == static_cast<int>(events.size())) {
        Logger::log(Logger::INFO, "Resizing the eventlist due to increased demand on server", "EpollManager::eventListener");
        events.resize(events.size() * 2);
    }

    return nev;
}

EventBlock EpollManager::getEvent(const int& index)
{
    EventBlock eventBlock;

    eventBlock.fd = events[index].data.fd;
    eventBlock.isRead = (events[index].events & EPOLLIN) != 0;
    eventBlock.isWrite = (events[index].events & EPOLLOUT) != 0;
    eventBlock.isEOF = (events[index].events & (EPOLLHUP | EPOLLRDHUP)) != 0;

    return eventBlock;
}

#endif