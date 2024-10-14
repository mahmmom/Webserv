
#include "KqueueManager.hpp"
#include "EventManager.hpp"

KqueueManager::KqueueManager()
{
	if ((kq = kqueue()) == -1) {
		perror("Kqueue: ");
		exit(EXIT_FAILURE);
	}
}

void	KqueueManager::registerEvent(int socketFD, EventType event)
{
	if (event == READ)
	{
		struct kevent change_event;
		EV_SET(&change_event, socketFD, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		changelist.push_back(change_event);
		kevent(kq, changelist.data(), changelist.size(), NULL, 0, 0);
	}
	if (event == WRITE)
	{
		struct kevent change_event;
		EV_SET(&change_event, socketFD, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		templist.push_back(change_event);
	}
}

void	KqueueManager::deregisterEvent(int socketFD, EventType event)
{
	if (event == READ)
		;
	if (event == WRITE)
	{
		struct kevent change_event;
		EV_SET(&change_event, socketFD, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		templist.push_back(change_event);
	}
}

int	KqueueManager::eventListener()
{
	int nev = kevent(kq, NULL, 0, eventlist.data(), eventlist.size(), NULL);

	return (nev);
}

EventBlock KqueueManager::getEvent(const int& index)
{
	EventBlock eventBlock;

	eventBlock.fd = eventlist[index].ident;
	eventBlock.isRead = (eventlist[index].filter == EVFILT_READ);
	eventBlock.isEOF = (eventlist[index].filter == EV_EOF);
	eventBlock.isWrite = (eventlist[index].filter == WRITE);

	return (eventBlock);
}
