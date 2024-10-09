
#ifndef LOADSETTIINGS_HPP
# define LOADSETTIINGS_HPP

#include <vector>
#include <string>
#include "ConfigNode.hpp"
#include "DirectiveNode.hpp"
#include "ContextNode.hpp"
#include "../Config/ServerSettings.hpp"

class LoadSettings
{
	private:
		std::string HttpRoot;
		std::string HttpAutoIndex;
		std::string HttpClientMaxBodySize;
		std::vector<DirectiveNode* > HttpErrorArgs;
		std::vector<DirectiveNode* > HttpIndexArgs;

		void processServerNode(ContextNode* serverNode, ServerSettings& serverSettings);
		void proccessHTTPNode(ContextNode* root);
	public:
		LoadSettings(ConfigNode* root);
};

#endif