
#if defined(__APPLE__) || defined(__FreeBSD__)

#include "KqueueManager.hpp"
#include "EventManager.hpp"

#include <iostream>

/*
    GENERAL INFO:
        
        Function: KqueueManager::KqueueManager() constructor
			
			The reaosn why we have to initial the eventlist to a size of DEFAULT_NUM_OF_EVENTS 
			is because if we don't, then eventlist would be default to a size of 0, aka an 
			empty vector. That would cause me trouble because in the KqueueManager::eventListener()
			the call to:

			int nev = kevent(kq, NULL, 0, eventlist.data(), eventlist.size(), NULL);

			would constantly return 0 to nev because eventlist.size() is 0 and so, the loop which 
			handles those events -> [for (int i = 0; i < nev; i++)] in the ServerManager::run() will 
			never start; even though there are events which should actually be present in eventlist.data(), 
			it's just that eventlist.size() was never modified to reflect that. That is because kevent() 
			does not resize the vector for us. We have to handle that. That is also why we resize it if 
			we get more clients connected and eventually nev becomes equal to eventlist.size(). Btw, if 
			it so happened that we instead got 207 events in one instance and our default is 200, then 
			kevent would only detect 200 but the remaining 7 events are not lost. Instead they are still on 
			the queue (kqueue) and will be detected by the subsequent call to kevent. But to prevent that from 
			happening again, we just save ourselves the trouble and double eventlist's size everytime we reach
			capacity.
*/

KqueueManager::KqueueManager() : eventlist(DEFAULT_NUM_OF_EVENTS)
{
	if ((kq = kqueue()) == -1) {
		perror("Kqueue: ");
		exit(EXIT_FAILURE);
	}
}

void	KqueueManager::registerEvent(int socketFD, EventType event)
{
	struct kevent 	change_event;
	memset(&change_event, 0, sizeof(change_event));
	std::string		eventStr;

	if (event == READ) {
		EV_SET(&change_event, socketFD, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		eventStr = "READ";
	}
	if (event == WRITE) {
		EV_SET(&change_event, socketFD, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		eventStr = "WRITE";
	}
	if (kevent(kq, &change_event, 1, NULL, 0, 0) == -1) {
		Logger::log(Logger::ERROR, "Failed to register " + eventStr + " event for fd " + 
			Logger::intToString(socketFD) + ": " + strerror(errno), "KqueueManager::registerEvent");
	}
	else {
		Logger::log(Logger::DEBUG, "Registered new " + eventStr + " event for fd " + 
			Logger::intToString(socketFD), "KqueueManager::registerEvent");
	}
}

void	KqueueManager::deregisterEvent(int socketFD, EventType event)
{
	struct kevent 	change_event;
	std::string		eventStr;
	memset(&change_event, 0, sizeof(change_event));

	if (event == READ) {
		EV_SET(&change_event, socketFD, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		eventStr = "READ";
	}
	if (event == WRITE) {
		EV_SET(&change_event, socketFD, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		eventStr = "WRITE";
	}
	if (kevent(kq, &change_event, 1, NULL, 0, 0) == -1) {
		Logger::log(Logger::ERROR, "Failed to deregister " + eventStr + " event for fd " + 
			Logger::intToString(socketFD) + ": " + strerror(errno), "KqueueManager::deregisterEvent");
	}
	else {
		Logger::log(Logger::DEBUG, "Deregistered " + eventStr + " event for fd " + 
			Logger::intToString(socketFD), "KqueueManager::deregisterEvent");
	}
}

int	KqueueManager::eventListener()
{
	struct timespec timeout;
	timeout.tv_sec = KEVENT_TIMEOUT_INTERVAL;
	timeout.tv_nsec = 0;

	int nev = kevent(kq, NULL, 0, eventlist.data(), eventlist.size(), &timeout);

	if ((size_t) nev == eventlist.size()) {
		Logger::log(Logger::INFO, "Resizing the eventlist due to increased demand on server", "Kqueue::eventListener");
		eventlist.resize(eventlist.size() * 2);  // Double the size if full
	}
	return (nev);
}

/*
	Notes:
		 EV_EOF is a flag that can be combined with other flags using bitwise operations!
*/
EventBlock KqueueManager::getEvent(const int& index)
{
	EventBlock eventBlock;

	eventBlock.fd = eventlist[index].ident;
	eventBlock.isRead = (eventlist[index].filter == EVFILT_READ);
	eventBlock.isEOF = (eventlist[index].flags & EV_EOF); // Note 1
	eventBlock.isWrite = (eventlist[index].filter == EVFILT_WRITE);

	return (eventBlock);
}

#endif