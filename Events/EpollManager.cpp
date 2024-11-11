
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
        Logger::log(Logger::ERROR, "Failed to register " + eventType + " event for fd " + 
			Logger::intToString(fd) + ": " + strerror(errno), "EpollManager::registerEvent");

    } else {
        Logger::log(Logger::DEBUG, "Registered new " + eventType + " event for fd " + 
			Logger::intToString(fd), "EpollManager::registerEvent");
    }
}

void EpollManager::deregisterEvent(int fd, EventType event)
{
    std::map<int, int>::iterator it = registeredEvents.find(fd);

    if (it == registeredEvents.end()) {
        Logger::log(Logger::WARN, "Attempted to deregister event for non-registered fd " + 
            Logger::intToString(fd), "EpollManager::deregisterEvent");
        return ;
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
        Logger::log(Logger::DEBUG, "Removed all events events for fd " +
            Logger::intToString(fd), "EpollManager::deregisterEvent");
    } 
    else {
        struct epoll_event epollEvent;
        epollEvent.data.fd = fd;
        epollEvent.events = it->second;
        if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &epollEvent) < 0) {
            Logger::log(Logger::ERROR, "Failed to deregister " + eventType + " event for fd " + 
			    Logger::intToString(fd) + ": " + strerror(errno), "EpollManager::deregisterEvent");
        }
        else {
            Logger::log(Logger::DEBUG, "Deregistered " + eventType + " event for fd " + 
                Logger::intToString(fd), "EpollManager::deregisterEvent");
        }
    }
}

int EpollManager::eventListener()
{
    int nev = epoll_wait(epfd, &events[0], static_cast<int>(events.size()), EPOLL_TIMEOUT_INTERVAL);

    if (nev < 0) {
        Logger::log(Logger::ERROR, "function epoll_wait failed", "EpollManager::eventListener");
    }
    else if (nev == static_cast<int>(events.size())) {
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