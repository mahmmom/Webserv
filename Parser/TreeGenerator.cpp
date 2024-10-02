
#include "TreeGenerator.hpp"


void printTree(const ConfigNode* node, int level = 0) {
   // Indentation based on tree depth level
   std::string indent(level * 4, ' ');


   // Check the type of node (Context or Directive)
   if (node->getType() == Context) {
       // Cast to ContextNode to access children
       const ContextNode* contextNode = static_cast<const ContextNode*>(node);
       std::cout << indent << "(" << contextNode->getContextName() << ")" << std::endl;


       // Recursively print all children
       const std::vector<ConfigNode*>& children = contextNode->getChildren();
		for (std::vector<ConfigNode*>::const_iterator it = children.begin(); it != children.end(); ++it) {
			printTree(*it, level + 1);
		}
   } else if (node->getType() == Directive) {
       // Cast to DirectiveNode to access directive values
       const DirectiveNode* directiveNode = static_cast<const DirectiveNode*>(node);
       std::cout << indent << directiveNode->getDirectiveName() << " ";


       // Print the values associated with the directive
		const std::vector<std::string>& args = directiveNode->getArguments();
		for (std::vector<std::string>::const_iterator it = args.begin(); it != args.end(); ++it) {
			std::cout << *it << " "; // Use dereference to get the actual string value
		}
       std::cout << std::endl;
   }
}


ConfigNode* TreeGenerator::generateTree(std::vector<std::string>& tokens)
{
	ContextNode* root = new ContextNode(tokens[0], NULL);

	DirectiveNode* child = new DirectiveNode(tokens[2], root);
	child->addArgument(tokens[3]);

    root->addChild(child);

    printTree(root);
	return (root);
}
