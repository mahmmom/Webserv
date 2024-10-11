
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
		void setAllowedMethods(const std::vector<std::string>& allowedMethods);
		LocationSettings(const std::string& path, const ServerSettings& serverSettings);

		void debugger() const;
};

#endif
