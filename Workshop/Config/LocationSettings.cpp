
#include "LocationSettings.hpp"

LocationSettings::LocationSettings(std::string& path, const ServerSettings& serverSettings)
	: BaseSettings(serverSettings.getRoot(),
			serverSettings.getAutoindex(),
			serverSettings.getClientMaxBodySize(),
			serverSettings.getErrorPages(),
			serverSettings.getErrorPagesLevel(),
			serverSettings.getIndex()), 
			path(path)
{
}
