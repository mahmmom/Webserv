#ifndef TREEGENERATOR_HPP
# define TREEGENERATOR_HPP

#include "ContextNode.hpp"
#include "DirectiveNode.hpp"

class TreeGenerator
{
	private:
		static ConfigNode*	handleLimitContext(ConfigNode* parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it);
		static ConfigNode*	handleLocationContext(ConfigNode* parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it);
		static ConfigNode* 	handleBasicContext(ConfigNode* parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it);
		static ConfigNode*	spawn(ConfigNode *parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it);
	public:
		static ConfigNode*	generateTree(std::vector<std::string>& tokens);
};

#endif