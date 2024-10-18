
#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "ConfigTokenizer.hpp"
#include "SyntaxAuditor.hpp"
#include "TreeGenerator.hpp"
#include "TreeAuditor.hpp"

class ConfigParser {
	private:
		std::vector<std::string>	_tokens;
		std::string					_configFileName;
		ConfigNode*					_configTreeRoot;
	public:
		ConfigParser(const std::string& _configFileName);
		void		parse();
		ConfigNode*	getConfigTreeRoot();
};

#endif