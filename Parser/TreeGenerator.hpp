#ifndef TREEGENERATOR_HPP
# define TREEGENERATOR_HPP

#include "ContextNode.hpp"
#include "DirectiveNode.hpp"

class TreeGenerator
{
	public:
		static ConfigNode* generateTree(std::vector<std::string>& tokens);
};

#endif