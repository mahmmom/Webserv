
#if defined(__linux__)

#include "EpollManager.hpp"

EpollManager::EpollManager() : events(DEFAULT_NUM_OF_EVENTS)
{
    epfd = epoll_create(DEFAULT_NUM_OF_EVENTS);

    if (epfd == -1) {
        log("ERROR", "Failed to create epoll instance");
        std::exit(EXIT_FAILURE);
    }
}

EpollManager::~EpollManager()
{
    close(epfd);
}

void EpollManager::registerEvent(int fd, EventType event)
{
    struct epoll_event epollEvent;
    epollEvent.data.fd = fd;
    int op;
    std::string eventType;
    std::map<int, int>::iterator it = registeredEvents.find(fd);

    if (it == registeredEvents.end()) {
        op = EPOLL_CTL_ADD;
        registeredEvents[fd] = 0;
        eventType = (event == READ) ? "READ (new)" : "WRITE (new)";
    } 
    else {
        op = EPOLL_CTL_MOD;
        eventType = (event == READ) ? "READ (mod)" : "WRITE (mod)";
    }

    if (event == READ) {
        registeredEvents[fd] |= EPOLLIN | EPOLLRDHUP;
    } 
    else {
        registeredEvents[fd] |= EPOLLOUT;
    }

    epollEvent.events = registeredEvents[fd];

    if (epoll_ctl(epfd, op, fd, &epollEvent) < 0) {
        log("ERROR", "Failed to register " + eventType + " event for fd " + intToString(fd));
    } else {
        log("DEBUG", "Registered " + eventType + " event for fd " + intToString(fd));
    }
}

void EpollManager::deregisterEvent(int fd, EventType event)
{
    std::map<int, int>::iterator it = registeredEvents.find(fd);

    if (it == registeredEvents.end()) {
        log("WARNING", "Attempted to deregister event for non-registered fd " + intToString(fd));
        return;
    }

    std::string eventType = (event == READ) ? "READ" : "WRITE";
    
    if (event == READ) {
        it->second &= ~(EPOLLIN | EPOLLRDHUP);
    } else {
        it->second &= ~EPOLLOUT;
    }

    if (it->second == 0) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        registeredEvents.erase(it);
        log("DEBUG", "Removed all events for fd " + intToString(fd));
    } 
    else {
        struct epoll_event epollEvent;
        epollEvent.data.fd = fd;
        epollEvent.events = it->second;
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &epollEvent) < 0) {
            log("ERROR", "Failed to deregister " + eventType + " event for fd " + intToString(fd));
        }
        else {
            log("DEBUG", "Deregistered " + eventType + " event for fd " + intToString(fd));
        }
    }
}

int EpollManager::eventListener()
{
    int nev = epoll_wait(epfd, &events[0], static_cast<int>(events.size()), -1);

    if (nev < 0) {
        log("ERROR", "epoll_wait failed");
    }
    else if (nev == static_cast<int>(events.size())) {
        events.resize(events.size() * 2);
        log("DEBUG", "Increased event buffer size to " + intToString(events.size()));
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