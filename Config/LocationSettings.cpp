
#include "LocationSettings.hpp"

LocationSettings::LocationSettings(const std::string& path, const ServerSettings& serverSettings)
	: BaseSettings(serverSettings.getRoot(),
			serverSettings.getAutoindex(),
			serverSettings.getClientMaxBodySize(),
			serverSettings.getErrorPages(),
			serverSettings.getErrorPagesLevel(),
			serverSettings.getIndex()), 
			path(path)
{
}

void LocationSettings::setAllowedMethods(const std::vector<std::string>& allowedMethods)
{
	this->allowedMethods = allowedMethods;
}
void LocationSettings::debugger() const
{
	// Call debugger for BaseSettings members
	std::cout << "root: " << root << std::endl;
	std::cout << "autoIndex: " << autoIndex << std::endl;
	std::cout << "clientMaxBodySize: " << clientMaxBodySize << std::endl;

	// Print errorPages
	std::cout << "errorPages:" << std::endl;
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
		std::cout << "  [" << it->first << "] = " << it->second << std::endl;
	}

	// Print errorPagesLevel
	std::cout << "errorPagesLevel:" << std::endl;
	for (std::map<int, std::string>::const_iterator it = errorPagesLevel.begin(); it != errorPagesLevel.end(); ++it) {
		std::cout << "  [" << it->first << "] = " << it->second << std::endl;
	}

	// Print index
	std::cout << "index:" << std::endl;
	for (std::vector<std::string>::const_iterator it = index.begin(); it != index.end(); ++it) {
		std::cout << "  " << *it << std::endl;
	}

	// Print returnDirective
	std::cout << "returnDirective:" << std::endl;
	std::cout << "    statusCode: " << returnDirective.getStatusCode() << std::endl;
	std::cout << "    textOrURL : " << returnDirective.getTextOrURL() << std::endl;

	// Print LocationSettings members
	std::cout << "path: " << path << std::endl;

	// Print allowedMethods
	std::cout << "allowedMethods:" << std::endl;
	for (std::vector<std::string>::const_iterator it = allowedMethods.begin(); it != allowedMethods.end(); ++it) {
		std::cout << "  " << *it << std::endl;
	}
}
