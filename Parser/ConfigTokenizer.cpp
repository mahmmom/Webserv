
#include "ConfigTokenizer.hpp"

const char* ConfigTokenizer::InvalidConfigFileException::what() const throw()
{
	return ("Unable to open config file");
}

void	ConfigTokenizer::extractPadding(std::string& configLine)
{
	std::string whitespaces = " \t\n\r\f\v";

	size_t	start = configLine.find_first_not_of(whitespaces);
	if (start != std::string::npos)
		configLine = configLine.substr(start);
	else
		configLine.clear();
	size_t	end = configLine.find_last_not_of(whitespaces);
	if (end != std::string::npos)
		configLine = configLine.substr(0, end + 1);
}

void	ConfigTokenizer::extractComments(std::string& configLine)
{
	size_t	pos = configLine.find("#");

	if (pos != std::string::npos)
		configLine = configLine.substr(0, pos);
}

void	ConfigTokenizer::splitByWSpace(std::string& configLine, std::vector<std::string>&tempTokens)
{
	std::istringstream 			stream(configLine);

	std::istream_iterator<std::string> begin(stream); // initialization constructor constructors an istream iterator that is associated with stream passed as an argument
	std::istream_iterator<std::string> end; // default constructor constructs an end-of-stream iterator.
	for (std::istream_iterator<std::string> it = begin; it != end; it++) {
		tempTokens.push_back(*it);
	}
}

void	ConfigTokenizer::splitByDelims(std::vector<std::string>& tokens, std::vector<std::string>& tempTokens)
{
	for (size_t i = 0; i < tempTokens.size(); i++) {
		std::string	build;

		for (std::string::iterator it = tempTokens[i].begin(); it != tempTokens[i].end(); it++) {
			if (*it == '{' || *it == '}' || *it == ';') {
				if (!build.empty()) // handle something like [{random}] to give [{, random, }] not [{, }, random]
				{
					tokens.push_back(build);
					build.clear();
				}
				tokens.push_back(std::string(1, *it));
			}
			else
				build += *it;
		}
		if (!build.empty())	// if a line only had { then dont push an empty line to the token list
			tokens.push_back(build);
	}
}

std::vector<std::string>	ConfigTokenizer::tokenize(std::string& configFileName)
{
	std::ifstream 				configStream;
	std::vector<std::string>	tokens;

	configStream.open(configFileName.c_str());
	if (!configStream.is_open())
		throw (ConfigTokenizer::InvalidConfigFileException());

	while (configStream.good()){
		std::string					configLine;
		std::vector<std::string>	tempTokens;

		std::getline(configStream, configLine);
		extractComments(configLine);
		extractPadding(configLine);
		if (configLine.empty())
			continue ;
		splitByWSpace(configLine, tempTokens);
		splitByDelims(tokens, tempTokens);
	}

	return (tokens);
}
