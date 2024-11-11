
#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#if defined(__linux__)

#define EPOLL_TIMEOUT_INTERVAL 5000 // ms

#include "EventManager.hpp"
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

class EpollManager : public EventManager {
    private:
        int epfd;
        std::vector<struct epoll_event> events;
        std::map<int, int> registeredEvents; // fd -> event types (bit flags)
    public:
        EpollManager();
        ~EpollManager();
        int         eventListener();
        void        deregisterEvent(int socketFD, EventType event);
        void        registerEvent(int socketFD, EventType event);
        EventBlock  getEvent(const int& index);
};

#endif

#endif // EPOLLMANAGER_HPP