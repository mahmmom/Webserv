
#ifndef SERVERSETTINGS_HPP
# define SERVERSETTINGS_HPP

#include <string>
#include "BaseSettings.hpp"
#include <vector>

#define DEFAULT_SERVER_ROOT "/var/www"
#define DEFAULT_SERVER_AUTOINDEX "off"
#define DEFAULT_SERVER_PORT 80
#define DEFAULT_SERVER_IP "0.0.0.0"
#define DEFAULT_SERVER_INDEX "index.html"
#define DEFAULT_SERVER_CLIENT_MAX_BODY_SIZE 1048576 // 1 Mebibyte

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
			what it is yet, 
			The reason behind that 
			is that the compiler still doesn't need to know the full internal details of LocationConfig. So instead, 
			only a pointer can be used. 
			
			An alternative would be to use a map of key-value pairs std::string path and
			LocationConfig locationConfig. No pointer is needed in this case. Checkout Nginx 2.0 but that's because 
			we wouldn't actually be handling locationConfig directly unlike in this implementation, instead the 
			LocationConfig object is simply assigned and the parsing of duplicates is actually done directly on 
			the map itself.
*/
class LocationSettings; // Note 1

class ServerSettings : public BaseSettings
{
	private:
		int			port;
		std::string	ip;
		void setPort(const std::string& portStr, const std::string& listenValue);
		void setIP(const std::string& IPv4, const std::string& listenValue);
		std::vector<LocationSettings* > locations; // Note 2

	public:
		ServerSettings(std::string& HttpRoot, std::string& HttpAutoIndex, 
			std::string& HttpClientMaxBodySize, std::string& HttpErrorPagesContext,
			std::vector<DirectiveNode* >& HttpErrorArgs, 
			std::vector<DirectiveNode* >& HttpIndexArgs);

		~ServerSettings();
		
		void				addLocation(LocationSettings& locationSettings);
		LocationSettings*	findLocation(const std::string& uri);

		void			setListenValues(const std::string &listenValue);
		void	 		setDefaultValues();
		int&			getPort();
		std::string&	getIP();

		void debugger() const;
};

#endif