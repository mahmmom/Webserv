
#include "ContextNode.hpp"

ContextNode::ContextNode(std::string& contextName, ConfigNode* parent, const std::string& path) :
	ConfigNode(Context, parent), contextName(contextName), path(path) {}

/*
	Note 1 :	Since children is a vector of pointers, deleting (*it) on each element
				effectively causes all these elements in the vector to become dangling 
				pointers. As such, accessing them can lead to undefined behavior or in 
				other words, a SEGFAULT. Hence, it is also important to clear the vector 
				after freeing each one of its elements.
*/
ContextNode::~ContextNode()
{
	std::vector<ConfigNode*>::iterator it;

	for (it = children.begin(); it != children.end(); it++) {
		delete (*it);
	}
	children.clear();	// Note 1
}

void	ContextNode::addChild(ConfigNode* child)
{
	children.push_back(child);
}

const std::string&	ContextNode::getContextName() const
{
	return (contextName);
}

const std::string&	ContextNode::getPath() const
{
	return (path);
}

const std::vector<ConfigNode*>&	ContextNode::getChildren() const
{
	return (children);
}
