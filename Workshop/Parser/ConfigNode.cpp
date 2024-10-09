
#include "ConfigNode.hpp"

ConfigNode::ConfigNode(NodeType type, ConfigNode* parentNode) : type(type), parent(parentNode) {}

ConfigNode::~ConfigNode() {}

NodeType 	ConfigNode::getType() const
{
	return (type);
}

ConfigNode* ConfigNode::getParent() const
{
	return (parent);
}
