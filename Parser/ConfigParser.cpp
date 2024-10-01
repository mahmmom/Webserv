
#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"

ConfigParser::ConfigParser(const std::string& filename) : _configFileName(filename) {}


void	ConfigParser::go()
{
	ConfigTokenizer::tokenize(_configFileName);
}
