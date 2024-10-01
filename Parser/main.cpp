
#include "ConfigTokenizer.hpp"
#include "ConfigParser.hpp"

int main()
{
	ConfigParser Parser(std::string("nginx.conf"));

	Parser.go();
}
