
#ifndef EVENTMANAGER_HPP
# define EVENTMANAGER_HPP

# include "../Logger/Logger.hpp"
# define DEFAULT_NUM_OF_EVENTS 200

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
		virtual int			eventListener() = 0;
		virtual void		deregisterEvent(int socketFD, EventType event) = 0;
		virtual void 		registerEvent(int socketFD, EventType event) = 0;
		virtual EventBlock	getEvent(const int& index) = 0;
};

#endif