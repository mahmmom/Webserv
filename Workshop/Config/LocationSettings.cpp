
#include "LocationSettings.hpp"

LocationSettings::LocationSettings(std::string& path, const ServerSettings& serverSettings)
	: 	path(path), 
		BaseSettings(serverSettings.getRoot(),
			serverSettings.getAutoindex(),
			serverSettings.getClientMaxBodySize(),
			serverSettings.getErrorPages(),
			serverSettings.getErrorPageLevel(),
			serverSettings.getIndex())
{
}
