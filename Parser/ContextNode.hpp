
#ifndef CONTEXTNODE_HPP
# define CONTEXTNODE_HPP

#include "ConfigNode.hpp"

/*
	Note 1 :	Not all contexts have paths, in fact only the location context has a path.
				But I would still like to use this node for all types of contexts. Hence, 
				by setting the path = "No path for this context" in the prototype of the 
				function, we are defaulting path to that value if no path has been provided. 

				So yes, you can call the constructor like this:
					ContextNode* random_context = new ContextNode("name", parent)
				You don't have to specify a path in the constructor and it'll still work because 
				of this defaulting through the prototype.

		[?]		Lastly, the parent can also be NULL if we don't specify it. So the root context 
				does not have to have a parent. Now, I defaulted that to NULL in the constructor 
				of the base class ConfigNode, but I am not sure if I also have to do that for 
				ContextNode. I feel like I have to, in order to actually default it to NULL. We 
				shall see later when we build the tree.
*/
class ContextNode : public ConfigNode
{
	private:
		std::string					contextName;
		std::string					path;
		std::vector<std::string>	methods;
		std::vector<ConfigNode*>	children;
	public:
		ContextNode(std::string& contextName, ConfigNode* parent, const std::string& path = "No path for this context"); // Note 1
		~ContextNode();
		void	addMethod(std::string& method);
		void	addChild(ConfigNode* child);
		const std::string&					getContextName() const;
		const std::string&					getPath() const;
		const std::vector<ConfigNode*>&		getChildren() const;
		const std::vector<std::string>&		getMethods() const;
};

#endif