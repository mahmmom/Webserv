
#include "EventManager.hpp"

/*
	The destructor must be present simply because for constructors and 
	destructors of derived classes, the derived class has to first call:
		* base class constructor and then the derived class constructor
		* derived class destructor and then the based class destructor

	So, in our case, we dont need a constructor. However, we do need 
	destructors for the derived classes because we need to close 'kq' 
	for Kqueue and we need to close 'epfd' for Epoll. However, since 
	these derived classes are based on the abstract base class EventManager, 
	we must have a definiton for the EventManager destructor as well, 
	because we can no longer use the default destructor for the derived 
	classes. Hence, to prevent the compiler from nagging, we had to implement 
	an empty destructor here.
*/
EventManager::~EventManager() {}