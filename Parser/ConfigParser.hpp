
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
		std::vector<std::string>	tokens;
		std::string					configFileName;
		ConfigNode*					configTreeRoot;
	public:
		ConfigParser(const std::string& _configFileName);
		~ConfigParser();
		void		parse();
		ConfigNode*	getConfigTreeRoot();
};

#endif