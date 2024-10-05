
#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"
#include "SyntaxAuditor.hpp"
#include "TreeGenerator.hpp"
#include "TreeAuditor.hpp"

ConfigParser::ConfigParser(const std::string& filename) : _configFileName(filename) {}


void	ConfigParser::go()
{
	_tokens = ConfigTokenizer::tokenize(_configFileName);

	// std::cout << "==========================" << std::endl;
	// for (size_t i = 0; i < _tokens.size(); i++) {
	// 	std::cout << _tokens[i] << std::endl;
	// }
	SyntaxAuditor::checkConfigSyntax(_tokens);

	ConfigNode* root = TreeGenerator::generateTree(_tokens);

	TreeAuditor	treeAudit;

	treeAudit.checkTreeLogic(root);
}
