
#include "ConfigTokenizer.hpp"

void	ConfigTokenizer::extract_white_spaces(std::string& configLine)
{
	size_t	start = configLine.find_first_not_of(" \t");
	if (start != std::string::npos)
		configLine = configLine.substr(start);
	else
		configLine.clear();
	size_t	end = configLine.find_last_not_of(" \t");
	if (end != std::string::npos)
		configLine = configLine.substr(0, end + 1);
}

void	ConfigTokenizer::extract_comments(std::string& configLine)
{
	size_t	pos = configLine.find("#");

	if (pos != std::string::npos)
		configLine = configLine.substr(0, pos);
}

void	ConfigTokenizer::split_line(std::string& configLine, std::vector<std::string>& tokens)
{
	std::istringstream 			stream(configLine);

	std::istream_iterator<std::string> begin(stream); // initialization constructor constructors an istream iterator that is associated with stream passed as an argument
	std::istream_iterator<std::string> end; // default constructor constructs an end-of-stream iterator.
	for (std::istream_iterator<std::string> it = begin; it != end; it++) {
		size_t pos = (*it).find(";");
		if (pos != std::string::npos) {
			tokens.push_back((*it).substr(0, pos));
			tokens.push_back((*it).substr(pos));
			continue ;
		}
		tokens.push_back(*it);
	}
}

void	ConfigTokenizer::tokenize(std::string& configFileName)
{
	std::ifstream 				configStream;
	std::vector<std::string>	tokens;

	configStream.open(configFileName.c_str());
	if (!configStream.is_open()) {
		std::cerr << "Unable to open file" << configFileName << std::endl;
		exit(1);
	}
	while (configStream.good()){
		std::string	configLine;

		std::getline(configStream, configLine);
		extract_comments(configLine);
		extract_white_spaces(configLine);
		if (configLine.empty())
			continue ;
		split_line(configLine, tokens);
		std::cout << configLine << "." << std::endl;
	}

	std::cout << "==========================" << std::endl;
	for (size_t i = 0; i < tokens.size(); i++) {
		std::cout << tokens[i] << std::endl;
	}
}
