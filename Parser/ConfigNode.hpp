#ifndef CONFIGNODE_HPP
# define CONFIGNODE_HPP

#include <string>
#include <vector>
#include <iostream>

enum NodeType
{
	Context,
	Directive
};

class ConfigNode
{
	private:
		NodeType	type;		
		ConfigNode*	parent;
	public:
		ConfigNode(NodeType type, ConfigNode* parentNode = NULL);
		virtual ~ConfigNode();
		NodeType 	getType() const;
		ConfigNode* getParent() const;
};

#endif