
#include "LoadSettings.hpp"

/*
	Note 1: Unlike in LoadSettings::proccessHTTPNode function, we don't 
			have to use the DirectiveNode* vector anymore because now that 
			we are in a ServerSettings class and it has already been 
			constructed, the error_pages and index related data are 
			already stored in a map and vector, respectively for me 
			(look at the attributes of BaseSettings class which are inherited
			by its derived classes as well). 
			
			The LoadSettings class also does not have access to the setErrorPages
			function like the ServerSettings class so we did not have the liberity
			there to use it (and push data directly onto a map or vector). We also 
			don't need to implement here, kinda though we could if we want, because 
			nothing is "really" stored in the http context. It's more like a formwork
			for all the Server contexts to take and if they wanna fine-tune these 
			general confiugrations, they can do so by overriding them.

			But here, we can just use the setErrorPages and setIndex functions 
			directly in the server context on each individual directive of these 
			types that we encounter,because the maps  push any error_page or 
			index directives back to their respective data structures.

			BUT WAIT SHOULD WE IMPLEMENT INDEX TO BE OVERWRITTEN ALWAYS?
*/
void LoadSettings::processServerNode(ContextNode* serverNode, ServerSettings& serverSettings)
{
	std::vector<ConfigNode* >children = serverNode->getChildren();
	std::vector<ConfigNode* >::iterator it;

	for (it = children.begin(); it != children.end(); it++) {
		if ((*it)->getType() == Directive)
		{
			DirectiveNode* directive = static_cast<DirectiveNode* >(*it);
			if (directive->getDirectiveName() == "root")
				serverSettings.setRoot(directive->getArguments()[0]);
			else if (directive->getDirectiveName() == "autoindex")
				serverSettings.setAutoIndex(directive->getArguments()[0]);
			else if (directive->getDirectiveName() == "client_max_body_size")
				serverSettings.setClientMaxBodySize(directive->getArguments()[0]);
			else if (directive->getDirectiveName() == "error_page")
				serverSettings.setErrorPages(directive->getArguments(), "server"); // Note 1
			else if (directive->getDirectiveName() == "index")
				serverSettings.setIndex(directive->getArguments()); // ibid
		}
	}

	std::cout << "\n====== SERVER CONTEXT LEVEL ======\n";
	serverSettings.debugger();
}

/*
	-- DECRIPTION --
	This function takes the HTTP context configurations and applies them to
	each and every server in the configuration file. Then processServerNode 
	can have its own version of these directives and it can override them in
	that function if the write of the config file so desires.

	-- NOTES --
	Note 1: In here, we have to have the error_page and index directives
			as vectors of DirectiveNode* and push_back everytime we encounter
			such directive. Why? Because these directives can be duplicated, 
			unlike the others.

			So the main issue is that as of now, we are still processing all 
			of the arguments as vectors directly from the DirectiveNode* 's. 
			Now to explain the next point, consider the following scenario. We 
			have two error_page directives in the http context of the config file.
			If we decalre the HttpErrorArgs and HttpIndexArgs as std::vector<std::string> 
			right away, just like we did with the other directives (which, as a reminder, 
			cannot be duplicated), what would happen is that the second vector of 
			error_page directives would OVERWRITE the previous one in this loop! That's 
			bad. Same applies for index because it can also be duplicated.
*/
void LoadSettings::proccessHTTPNode(ContextNode* root)
{
	std::vector<ConfigNode* >children = root->getChildren();
	std::vector<ConfigNode* >::iterator it;

	for (it = children.begin(); it != children.end(); it++) {
		if ((*it)->getType() == Directive)
		{
			DirectiveNode* directive = static_cast<DirectiveNode* >(*it);
			if (directive->getDirectiveName() == "root")
				this->HttpRoot = directive->getArguments()[0];
			else if (directive->getDirectiveName() == "autoindex")
				this->HttpAutoIndex = directive->getArguments()[0];
			else if (directive->getDirectiveName() == "client_max_body_size")
				this->HttpClientMaxBodySize = directive->getArguments()[0];
			else if (directive->getDirectiveName() == "error_page")
				this->HttpErrorArgs.push_back(directive); // Note 1
			else if (directive->getDirectiveName() == "index")
				this->HttpIndexArgs.push_back(directive); // ibid
		}
	}

	for (it = children.begin(); it != children.end(); it++) {
		if ((*it)->getType() == Context)
		{
			std::string contextLevel = "http"; 
			ContextNode* serverNode = static_cast<ContextNode* >(*it);
			ServerSettings serverSettings(this->HttpRoot, this->HttpAutoIndex, 
				this->HttpClientMaxBodySize, contextLevel, this->HttpErrorArgs,
				this->HttpIndexArgs);
			processServerNode(serverNode, serverSettings);
		}
	}

	std::cout << "\n======== HTTP CONTEXT LEVEL ========\n";
	this->debugger();
}

LoadSettings::LoadSettings(ConfigNode* root)
{
	ContextNode* rootNode= static_cast<ContextNode* >(root);
	proccessHTTPNode(rootNode);
}

void LoadSettings::debugger() const
{
	// Print HttpRoot, HttpAutoIndex, and HttpClientMaxBodySize
	std::cout << "HttpRoot: " << HttpRoot << std::endl;
	std::cout << "HttpAutoIndex: " << HttpAutoIndex << std::endl;
	std::cout << "HttpClientMaxBodySize: " << HttpClientMaxBodySize << std::endl;

	// Print HttpErrorArgs
	std::cout << "HttpErrorArgs:" << std::endl;
	for (std::vector<DirectiveNode*>::const_iterator it = HttpErrorArgs.begin(); it != HttpErrorArgs.end(); ++it) {
		if (*it) {
			std::cout << "  Directive: " << (*it)->getDirectiveName() << std::endl;
			std::cout << "  Arguments: ";
			const std::vector<std::string>& args = (*it)->getArguments();
			for (std::vector<std::string>::const_iterator argIt = args.begin(); argIt != args.end(); ++argIt) {
				std::cout << *argIt << " ";
			}
			std::cout << std::endl;
			std::cout << "  Number of arguments: " << (*it)->getNumOfArguments() << std::endl;
		}
	}

	// Print HttpIndexArgs
	std::cout << "HttpIndexArgs:" << std::endl;
	for (std::vector<DirectiveNode*>::const_iterator it = HttpIndexArgs.begin(); it != HttpIndexArgs.end(); ++it) {
		if (*it) {
			std::cout << "  Directive: " << (*it)->getDirectiveName() << std::endl;
			std::cout << "  Arguments: ";
			const std::vector<std::string>& args = (*it)->getArguments();
			for (std::vector<std::string>::const_iterator argIt = args.begin(); argIt != args.end(); ++argIt) {
				std::cout << *argIt << " ";
			}
			std::cout << std::endl;
			std::cout << "  Number of arguments: " << (*it)->getNumOfArguments() << std::endl;
		}
	}
}
