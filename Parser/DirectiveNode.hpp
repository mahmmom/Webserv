#ifndef DIRECTIVENODE_HPP
# define DIRECTIVENODE_HPP

#include "ConfigNode.hpp"

class DirectiveNode : public ConfigNode
{
	private:
		std::string					directiveName;
		std::vector<std::string>	arguments;
		int							numOfArguments;
	public:
		DirectiveNode(std::string& directiveName, ConfigNode* parent);
		~DirectiveNode();
		void							addArgument(std::string& argument);
		const std::string&				getDirectiveName() const;
		const std::vector<std::string>&	getArguments() const;
		const int&						getNumOfArguments() const;
};

#endif