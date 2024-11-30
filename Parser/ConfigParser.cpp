
#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename) : configFileName(filename), configTreeRoot(NULL) {}

ConfigParser::~ConfigParser()
{
	if (configTreeRoot)
    	delete configTreeRoot;
}

void	ConfigParser::parse()
{
	tokens = ConfigTokenizer::tokenize(configFileName);
	SyntaxAuditor::checkConfigSyntax(tokens);
	configTreeRoot = TreeGenerator::generateTree(tokens);
	TreeAuditor	treeAudit;
	treeAudit.checkTreeLogic(configTreeRoot);
}

ConfigNode* ConfigParser::getConfigTreeRoot()
{
	return (configTreeRoot);
}
