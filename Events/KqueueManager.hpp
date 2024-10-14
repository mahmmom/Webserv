
#ifndef KQUEUEMANAGER_HPP
# define KQUEUEMANAGER_HPP

#include "EventManager.hpp"
#include <sys/event.h>  // for kqueue
#include <vector>


class KqueueManager : public EventManager
{
	private:
		int kq;
	    std::vector<struct kevent> eventlist;
	public:
		KqueueManager();
		int			eventListener();
		void		deregisterEvent(int socketFD, EventType event);
		void 		registerEvent(int socketFD, EventType event);
		EventBlock	getEvent(const int& index);
};

#endif