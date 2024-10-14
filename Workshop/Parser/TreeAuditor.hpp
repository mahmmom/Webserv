
#ifndef TREEAUDITOR_HPP
# define TREEAUDITOR_HPP

#include "ConfigNode.hpp"
#include "ContextNode.hpp"
#include "DirectiveNode.hpp"
#include <map>
#include <utility>

enum DirectiveDependency {
	Independent,
	ParentRequired
};

enum ArgsRequired {
	OneArg,
	OneOrTwo,
	OneOrMore,
	TwoOrMore
};

/*
	Can't set the function as static because the function need to acquire the directiveMap
	and thus, because they have to access a class member, they cannot be declared as static.
*/
class TreeAuditor
{
	private:
		std::map<std::string, std::pair<ArgsRequired, DirectiveDependency> > directiveMap;
		void	checkDirective(DirectiveNode* dirNode, std::pair<ArgsRequired, DirectiveDependency> pair);
		void	checkParent(DirectiveNode* node);
		void	checkDirectiveLogic(ConfigNode* rootNode);
		int		directiveInstanceCounter(ContextNode* parentNode, const std::string& directiveName);
		void	checkDuplicateDirectives(ConfigNode* rootNode);
	public:
		TreeAuditor();
		void	checkTreeLogic(ConfigNode* rootNode);
};

#endif
