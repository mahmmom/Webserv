
#ifndef LOCATIONSETTINGS_HPP
# define LOCATIONSETTINGS_HPP

#include "BaseSettings.hpp"
#include "ServerSettings.hpp"
#include <string>
#include <vector>

class LocationSettings : public BaseSettings
{
	private:
		std::string					path;
		std::vector<std::string>	allowedMethods;
	public:
		LocationSettings(std::string& path, const ServerSettings& serverSettings);
};

#endif
