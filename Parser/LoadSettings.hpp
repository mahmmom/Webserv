
#ifndef LOADSETTIINGS_HPP
# define LOADSETTIINGS_HPP

#define DEFAULT_HTTP_ROOT "/var/www"
#define DEFAULT_HTTP_INDEX "index.html"
#define DEFAULT_HTTP_AUTOINDEX "off"
#define DEFAULT_HTTP_CLIENT_MAX_BODY_SIZE "1M" // 1 Mebibyte

#include <vector>
#include <string>
#include "ConfigNode.hpp"
#include "DirectiveNode.hpp"
#include "ContextNode.hpp"
#include "../Config/ServerSettings.hpp"
#include "../Config/LocationSettings.hpp"

class LoadSettings
{
	private:
		ConfigNode* rootNode;
		std::string HttpRoot;
		std::string HttpAutoIndex;
		std::string HttpClientMaxBodySize;
		std::vector<DirectiveNode* > HttpErrorArgs;
		std::vector<DirectiveNode* > HttpIndexArgs;

		void processLocationNode(ContextNode* locationNode, LocationSettings& locationSettings);
		void processServerNode(ContextNode* serverNode, ServerSettings& serverSettings);
		void processHTTPNode(ContextNode* root, std::vector<ServerSettings>& serverSettingsVector);
	public:
		LoadSettings(ConfigNode* root);
		void	loadServers(std::vector<ServerSettings>& serverSettingsVector);
		void	debugger() const;
};

#endif