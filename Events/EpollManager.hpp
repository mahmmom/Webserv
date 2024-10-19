
#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#if defined(__linux__)

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

        void log(const std::string& level, const std::string& message) const {
            std::cout << "[" << level << "] EpollManager: " << message << std::endl;
        }

        std::string intToString(int number) const {
            std::ostringstream oss;
            oss << number;
            return oss.str();
        }
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