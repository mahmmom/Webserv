
#include "TreeAuditor.hpp"

/*
	To be implemented:
		server_name ?
		try_files ?
		keepalive_timeout
		cgi_extension (maybe in a different style and/or name)
*/
TreeAuditor::TreeAuditor()
{
	directiveMap["root"] = std::make_pair(OneArg, Independent);
	directiveMap["listen"] = std::make_pair(OneArg, ParentRequired);
	directiveMap["autoindex"] = std::make_pair(OneArg, Independent);
	directiveMap["client_max_body_size"] = std::make_pair(OneArg, Independent);
	directiveMap["error_page"] = std::make_pair(TwoOrMore, Independent);
	directiveMap["index"] = std::make_pair(OneOrMore, Independent);
	directiveMap["return"] = std::make_pair(OneOrTwo, ParentRequired);
	directiveMap["deny"] = std::make_pair(OneArg, ParentRequired);
}

void	TreeAuditor::checkDirective(DirectiveNode* dirNode, std::pair<ArgsRequired, DirectiveDependency> pair)
{
	bool	valid = true;

	if (pair.first == OneArg && (dirNode->getNumOfArguments() != 1))
		valid = false;
	else if (pair.first == OneOrTwo && (dirNode->getNumOfArguments() != 1 && dirNode->getNumOfArguments() != 2))
		valid = false;
	else if (pair.first == OneOrMore && dirNode->getArguments().empty())
		valid = false;
	else if (pair.first == TwoOrMore && dirNode->getNumOfArguments() < 2)
		valid = false;


	if (dirNode->getDirectiveName() == "auto_index")
		std::cout << "Result is -> " << valid << std::endl;

	if (valid == false)
		throw (std::runtime_error("Invalid number of arguments in \"" + dirNode->getDirectiveName() + "\" directive"));
}

void	TreeAuditor::checkParent(DirectiveNode* node)
{
	ContextNode*	parentNode = static_cast<ContextNode *>(node->getParent());
	if (node->getDirectiveName() == "listen")
	{
		if (parentNode->getContextName() != "server")
			throw (std::runtime_error("\"listen\" directive is not allowed in this context"));
	}

	else if (node->getDirectiveName() == "return")
	{
		if (parentNode->getContextName() != "location" && parentNode->getContextName() != "server")
			throw (std::runtime_error("\"return\" directive is not allowed in this context"));
	}
	else if (node->getDirectiveName() == "deny")
	{
		if (parentNode->getContextName() != "limit_except")
			throw (std::runtime_error("Server only supports \"deny\" directive in a \"limit_except\" block"));
	}
}

void	TreeAuditor::checkDirectiveLogic(ConfigNode* node)
{
	if (node->getType() == Context) {
		ContextNode* comCast = static_cast<ContextNode *>(node);
		if (comCast->getContextName() == "limit_except")
		{
			if (comCast->getMethods().empty())
				throw (std::runtime_error("Invalid number of arguments in \"limit_except\" directive"));
			if (static_cast<ContextNode *>(comCast->getParent())->getContextName() != "location")
				throw (std::runtime_error("\"limit_except\" directive is not allowed in this context"));
		}
		std::vector<ConfigNode *>children = comCast->getChildren();
		std::vector<ConfigNode *>::const_iterator it;
		for (it = children.begin(); it != children.end(); it++) {
				checkTreeLogic(*it);
		}
	}
	if (node->getType() == Directive) {
		DirectiveNode* dirCast = static_cast<DirectiveNode *>(node);
		std::map<std::string, std::pair<ArgsRequired, DirectiveDependency> >::const_iterator it = directiveMap.find(dirCast->getDirectiveName());
		if (it != directiveMap.end())
		{
			if (it->second.second == ParentRequired)
				checkParent(dirCast);
			checkDirective(dirCast, it->second);
		}
		else
			throw (std::runtime_error("Unknown directive \"" + dirCast->getDirectiveName() + "\""));
	}
}

int	TreeAuditor::directiveInstanceCounter(ContextNode* parentNode, const std::string& directiveName)
{
	std::vector<ConfigNode *>children = parentNode->getChildren();
	std::vector<ConfigNode *>::const_iterator it;
	int	count = 0;

	for (it = children.begin(); it != children.end(); it++) {
		if ((*it)->getType() == Directive) {
			if (static_cast<DirectiveNode *>(*it)->getDirectiveName() == directiveName)
				count++;
		}
	}
	return (count);
}

void	TreeAuditor::checkDuplicateDirectives(ConfigNode* node)
{
	if (node->getType() == Context) {
		ContextNode* contextNode = static_cast<ContextNode *>(node);
		std::vector<ConfigNode*>::const_iterator it;
		for (it = contextNode->getChildren().begin(); it != contextNode->getChildren().end(); it++) {
			if ((*it)->getType() == Context) {

				if (static_cast<ContextNode *>(*it)->getContextName() == "location") {
					std::vector<ConfigNode *>children = static_cast<ContextNode *>(*it)->getChildren();
					std::vector<ConfigNode *>::iterator it;
					size_t	lim_except_count = 0;
					for (it = children.begin(); it != children.end(); it++) {
						if ((*it)->getType() == Context && static_cast<ContextNode *>(*it)->getContextName() == "limit_except")
							lim_except_count++;
						if (lim_except_count > 1)
							throw (std::runtime_error("\"limit_except\" directive is duplicated"));
					}
				}

				checkDuplicateDirectives(*it);
			}
			else {
				if (directiveInstanceCounter(static_cast<ContextNode *>(node), static_cast<DirectiveNode* >(*it)->getDirectiveName()) != 1)
					throw (std::runtime_error("\"" + static_cast<DirectiveNode* >(*it)->getDirectiveName() + "\" directive is duplicated"));
			}
		}
	}
}

void	TreeAuditor::checkTreeLogic(ConfigNode* rootNode)
{
	checkDirectiveLogic(rootNode);
	checkDuplicateDirectives(rootNode);
}
