
#ifndef CONFIG_TOKENIZER_HPP
# define CONFIG_TOKENIZER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <cstdlib>

class ConfigTokenizer
{
	private:
		static void	extract_white_spaces(std::string& configLine);
		static void	extract_comments(std::string& configLine);
		static void	split_line(std::string& configLine, std::vector<std::string>& tokens);
	public:	
		static void	tokenize(std::string& configFileName);
};

#endif