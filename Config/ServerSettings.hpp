
#ifndef SERVERSETTINGS_HPP
# define SERVERSETTINGS_HPP

#include <string>
#include "BaseSettings.hpp"

#define DEFAULT_SERVER_ROOT "/var/www"
#define DEFAULT_SERVER_AUTOINDEX "off"
#define DEFAULT_SERVER_PORT 80
#define DEFAULT_SERVER_IP "0.0.0.0"
#define DEFAULT_SERVER_INDEX "index.html"
#define DEFAULT_SERVER_CLIENT_MAX_BODY_SIZE 1048576 // 1 Mebibyte

class ServerSettings : public BaseSettings
{
	private:
		int			port;
		std::string	ip;
		void setPort(const std::string& portStr, const std::string& listenValue);
		void setIP(const std::string& IPv4, const std::string& listenValue);
	public:
		ServerSettings(std::string& HttpRoot, std::string& HttpAutoIndex, 
			std::string& HttpClientMaxBodySize, std::string& HttpErrorPagesContext,
			std::vector<DirectiveNode* >& HttpErrorArgs, 
			std::vector<DirectiveNode* >& HttpIndexArgs);

		void			setListenValues(const std::string &listenValue);
		void	 		setDefaultValues();
		int&			getPort();
		std::string&	getIP();

		void debugger() const;
};

#endif