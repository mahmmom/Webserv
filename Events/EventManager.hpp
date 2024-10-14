
#ifndef EVENTMANAGER_HPP
# define EVENTMANAGER_HPP

enum EventType {
	READ,
	WRITE
};

struct EventBlock
{
	int		fd;
	bool	isRead;
	bool	isWrite;
	bool	isEOF;
};

class EventManager
{
	public:
		virtual int			eventListener();
		virtual void		deregisterEvent(int socketFD, EventType event);
		virtual void 		registerEvent(int socketFD, EventType event);
		virtual EventBlock	getEvent(const int& index);
};

#endif