
#ifndef CONFIG_TOKENIZER_HPP
# define CONFIG_TOKENIZER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstdlib>
#include <exception>

class ConfigTokenizer
{
	private:
		static void	extractPadding(std::string& configLine);
		static void	extractComments(std::string& configLine);
		static void	splitByWSpace(std::string& configLine, std::vector<std::string>& tokens);
		static void splitByDelims(std::vector<std::string>& tokens, std::vector<std::string>& tempTokens);
	public:
		class InvalidConfigFileException;
		static void	tokenize(std::string& configFileName);
};

class ConfigTokenizer::InvalidConfigFileException : public std::exception
{
	public:
		const char* what() const throw();
};

#endif