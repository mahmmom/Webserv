
#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

class ConfigParser {
	private:
		std::vector<std::string>	_tokens;
		std::string					_configFileName;
	public:
		ConfigParser(const std::string& _configFileName);
		void						go();
};

#endif