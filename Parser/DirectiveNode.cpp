
#include "DirectiveNode.hpp"

DirectiveNode::DirectiveNode(const std::string& directiveName, ConfigNode* parent) : 
	ConfigNode(Directive, parent), directiveName(directiveName), numOfArguments(0) {}

DirectiveNode::~DirectiveNode()
{
	arguments.clear();
}

void	DirectiveNode::addArgument(std::string& argument)
{
	arguments.push_back(argument);
	numOfArguments++;
}

const std::string&	DirectiveNode::getDirectiveName() const
{
	return (directiveName);
}

const std::vector<std::string>&	DirectiveNode::getArguments() const
{
	return	(arguments);
}

const int&	DirectiveNode::getNumOfArguments() const
{
	return (numOfArguments);
}
