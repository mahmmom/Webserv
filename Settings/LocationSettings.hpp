
#ifndef LOCATIONSETTINGS_HPP
# define LOCATIONSETTINGS_HPP

#include "BaseSettings.hpp"
#include "ServerSettings.hpp"
#include "AliasDirective.hpp"
#include <string>
#include <vector>
#include <algorithm>

class LocationSettings : public BaseSettings
{
	private:
		std::string					path;
		std::vector<std::string>	allowedMethods;
		AliasDirective				aliasDirective;
	public:
		LocationSettings(const std::string& path, const ServerSettings& serverSettings);
		LocationSettings(const LocationSettings& other);
		LocationSettings& operator=(const LocationSettings& other);

		bool isMethodAllowed(const std::string& method);
		void setAllowedMethods(const std::vector<std::string>& allowedMethods);
		void setAliasDirective(const std::string& aliasURL);

		const std::vector<std::string>&	getAllowedMethods() const;
		const std::string&				getPath() const;
		AliasDirective&					getAliasDirective();

		void debugger() const;
};

#endif
