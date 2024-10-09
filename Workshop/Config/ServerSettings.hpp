
#ifndef SERVERSETTINGS_HPP
# define SERVERSETTINGS_HPP

#include <string>
#include "BaseSettings.hpp"
#
class ServerSettings : public BaseSettings
{
	private:
		int			port;
		std::string	ip;
	public:
		ServerSettings(std::string& HttpRoot, std::string& HttpAutoIndex, 
			std::string& HttpClientMaxBodySize, std::string& HttpErrorPagesContext,
			std::vector<DirectiveNode* >& HttpErrorArgs, 
			std::vector<DirectiveNode* >& HttpIndexArgs);
};

#endif