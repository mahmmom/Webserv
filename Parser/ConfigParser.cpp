
#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename) : _configFileName(filename) {}

ConfigNode* ConfigParser::getConfigTreeRoot()
{
	return (_configTreeRoot);
}

void	ConfigParser::parse()
{
	_tokens = ConfigTokenizer::tokenize(_configFileName);
	SyntaxAuditor::checkConfigSyntax(_tokens);
	_configTreeRoot = TreeGenerator::generateTree(_tokens);
	TreeAuditor	treeAudit;
	treeAudit.checkTreeLogic(_configTreeRoot);
}
