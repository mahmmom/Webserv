
#include "ServerSettings.hpp"

ServerSettings::ServerSettings(std::string& HttpRoot, 
		std::string& HttpAutoIndex, std::string& HttpClientMaxBodySize,
		std::string& HttpErrorPagesContext, std::vector<DirectiveNode* >& HttpErrorArgs, 
		std::vector<DirectiveNode* >& HttpIndexArgs)
		: BaseSettings(HttpRoot, HttpAutoIndex, HttpClientMaxBodySize, HttpErrorPagesContext, HttpErrorArgs, HttpIndexArgs)
{}
