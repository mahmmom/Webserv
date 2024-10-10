
#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string& filename) : _configFileName(filename) {}

ConfigNode* ConfigParser::getConfigTreeRoot()
{
	return (_configTreeRoot);
}

void	ConfigParser::go()
{
	_tokens = ConfigTokenizer::tokenize(_configFileName);

	// std::cout << "==========================" << std::endl;
	// for (size_t i = 0; i < _tokens.size(); i++) {
	// 	std::cout << _tokens[i] << std::endl;
	// }
	SyntaxAuditor::checkConfigSyntax(_tokens);

	_configTreeRoot = TreeGenerator::generateTree(_tokens);

	TreeAuditor	treeAudit;
	treeAudit.checkTreeLogic(_configTreeRoot);
}
