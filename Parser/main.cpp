
#include "ConfigTokenizer.hpp"
#include "ConfigParser.hpp"

int main()
{
	try 
	{
		ConfigParser Parser(std::string("nginx.conf"));
		Parser.go();
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
