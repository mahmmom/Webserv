
#ifndef SERVERSETTINGS_HPP
# define SERVERSETTINGS_HPP

#include <string>
#include "BaseSettings.hpp"
#include "CGIDirective.hpp"
#include <vector>
#include <algorithm>

#define DEFAULT_SERVER_ROOT "/var/www"
#define DEFAULT_SERVER_AUTOINDEX "off"
#define DEFAULT_SERVER_PORT 80
#define DEFAULT_SERVER_IP "0.0.0.0"
#define DEFAULT_SERVER_INDEX "index.html"
#define DEFAULT_SERVER_CLIENT_MAX_BODY_SIZE 1048576 // 1 Mebibyte
#define DEFAULT_SERVER_KEEPALIVE_TIMEOUT 10

/*
	NOTES

	Note 1:	Use Forward Declaration because #include "LocationSettings.hpp" would result in an
			error (circular dependency) so that you can define the prototypes here. Then in the
			actual ServerSettings.cpp, in order to access the class, you can #include "LocationSettings.hpp"
			in there. That won't cause a circular depency because LocationSettings.hpp does not include 
			ServerSettings.cpp, only ServerSettings.hpp.
	
	Note 2: Since we are using a forward declaration, however, that still does not mean that I can use 
			the DirectiveNode to its full ability in this class. So if it was declared as 
			std::vector<LocationSettings > locations and I attempted to access that vector via an iterator 
			the way I do in the addLocation method below, then the compiler would nag. Why? Well it's a bit 
			confusing but listen up.
			
			We did end up including "LocationSettings.hpp" in ServerSettings.cpp, so that should be fine, right?
			Well yes and no. The thing is, including the header would allow you to access the member functions 
			of that class, like [LocationSettings locationSettings.getPath()] BUT, that doesn't change the fact 
			that it is still a forwardly declared class. That means that the compiler still doesn't really know 
			what it is yet, it's just saying it knows what it is (kinda like a mere "lip service"). So because 
			of that, you cant create a vector of LocationSettings. std::vector needs to know the size of the 
			type it contains to manage memory correctly. When you forward-declare a class, the compiler only 
			knows that the class exists but doesn't know its size or layout. So instead, only a pointer can be used. 
			A pointer is always 8 bytes and so, its size is known.
			
			An alternative would be to use a map of key-value pairs std::string path and
			LocationConfig locationConfig. No pointer is needed in this case. Checkout Nginx 2.0 but that's because 
			we wouldn't actually be handling locationConfig directly unlike in this implementation, instead the 
			LocationConfig object is simply assigned and the parsing of duplicates is actually done directly on 
			the map itself. If you use a map, then you can set a forwardly declared class as a VALUE (generally, but 
			not ALWAYS, see the last sentence of this paragraph to know why) but NEVER as a KEY. 
			Keys require full definitions because they need to be compared to maintain the order within the map.
			By default, std::map uses std::less<KeyType> to compare keys. std::less relies on the < operator or a custom 
			comparator, both of which need to "see" the full class definition to know how to compare instances of KeyType. 
			Values can be forward-declared since they don’t need ordering, just storage. The only time the compiler needs 
			the full definition of ValueType is when you actually create, copy, or assign an instance of that type (e.g., 
			when inserting or accessing values). For simply declaring the map, a forward declaration is enough.
*/
class LocationSettings; // Note 1

class ServerSettings : public BaseSettings
{
	private:
		int								port;
		std::string						ip;
		std::vector<LocationSettings* > locations; // Note 2
		CGIDirective					cgiExtensions;
		
		void setPort(const std::string& portStr, const std::string& listenValue);
		void setIP(const std::string& IPv4, const std::string& listenValue);

	public:
		ServerSettings(std::string& HttpRoot, std::string& HttpAutoIndex, 
			std::string& HttpClientMaxBodySize, std::string& HttpKeepAliveTimeout, 
			std::string& HttpErrorPagesContext, std::vector<DirectiveNode* >& HttpErrorArgs, 
			std::vector<DirectiveNode* >& HttpIndexArgs);
		ServerSettings(const ServerSettings& other);
		ServerSettings& operator=(const ServerSettings& other);
		~ServerSettings();


		void				addLocation(LocationSettings& locationSettings);
		LocationSettings*	findLocation(const std::string& uri);

		void			setListenValues(const std::string& listenValue);
		void	 		setDefaultValues();
		void			setCgiExtensions(const std::vector<std::string>& extensions);

		int&			getPort();
		std::string&	getIP();

		void debugger() const;
};

#endif