
#include "TreeGenerator.hpp"

void printTree(const ConfigNode* node, int level = 0);

ConfigNode* TreeGenerator::handleLimitContext(ConfigNode* parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it)
{
    if (it == tokens.end())
        return (NULL);
    ContextNode* contextNode = new ContextNode(*it, parent);
    it++;
    while (it != tokens.end() && *it != "{")
    {
        contextNode->addMethod(*it);
        it++;
    }
    if (it == tokens.end())
        return (NULL);
    it++;
    return (contextNode);
}

ConfigNode* TreeGenerator::handleLocationContext(ConfigNode* parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it)
{
    if (it == tokens.end())
        return (NULL);
    ContextNode* contextNode = new ContextNode(*it, parent, *(it + 1));
    it += 3;
    return (contextNode);
}

ConfigNode* TreeGenerator::handleBasicContext(ConfigNode* parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it)
{
    if (it == tokens.end())
        return (NULL);
    ContextNode* contextNode = new ContextNode(*it, parent);
    it += 2;
    return (contextNode);
}

ConfigNode* TreeGenerator::spawn(ConfigNode* parent, std::vector<std::string>& tokens, std::vector<std::string>::iterator& it)
{
    ConfigNode* node = NULL;

    if (it == tokens.end())
            return (NULL);
    if (*it == "http" || *it == "server")
        node = handleBasicContext(parent, tokens, it);
    else if (*it == "location")
        node = handleLocationContext(parent, tokens, it);
    else if (*it == "limit_except")
        node = handleLimitContext(parent, tokens, it);
    else
    {
        DirectiveNode* directiveNode = new DirectiveNode(*it, parent);
        it++;
        while (it != tokens.end() && *it != ";") {
            directiveNode->addArgument(*it);
            it++;
        }
        if (it == tokens.end()) {
            delete(directiveNode);
            return (NULL);
        }
        it++; // skip past semicolon
        node = directiveNode;
    }
    if (node != NULL && node->getType() == Context) {
        while (it != tokens.end() && *it != "}")
            static_cast<ContextNode*>(node)->addChild(spawn(node, tokens, it));
        if (it != tokens.end() && *it == "}")
            it++; // skip past the closing brace
    }
    return (node);
}

ConfigNode* TreeGenerator::generateTree(std::vector<std::string>& tokens)
{
    std::vector<std::string>::iterator  it = tokens.begin();

    ConfigNode* tree = spawn(NULL, tokens, it);

    std::cout << std::endl;
    if (tree != NULL)
        printTree(tree);
    else
        std::cout << "Sorry mate, tree is NULL\n";
    return (tree);
}

void printTree(const ConfigNode* node, int level) {
    // Indentation based on tree depth level
    std::string indent(level * 4, ' ');

    // Check the type of node (Context or Directive)
    if (node->getType() == Context) {
        // Cast to ContextNode to access children and methods
        const ContextNode* contextNode = static_cast<const ContextNode*>(node);
        std::cout << indent << "(" << contextNode->getContextName() << ")" << std::endl;

        // Special handling for "limit_except" context
        if (contextNode->getContextName() == "limit_except") {
            // Print methods at a deeper indentation level
            const std::vector<std::string>& methods = contextNode->getMethods();
            std::cout << indent << "    Methods: "; // Extra indentation for methods
            for (std::vector<std::string>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
                std::cout << *it << " "; // Print each method
            }
            std::cout << std::endl;
        }

        // Recursively print all children, regardless of context type
        const std::vector<ConfigNode*>& children = contextNode->getChildren();
        for (std::vector<ConfigNode*>::const_iterator it = children.begin(); it != children.end(); ++it) {
            printTree(*it, level + 1);
        }

    } else if (node->getType() == Directive) {
        // Uniform handling for all directives
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
