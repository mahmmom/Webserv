
#include "Parser/ConfigParser.hpp"
#include "Config/BaseSettings.hpp"
#include "Parser/LoadSettings.hpp"

int main()
{
	try 
	{
		ConfigParser Parser(std::string("Parser/nginx.conf"));
		Parser.go();

		std::cout << "\n===============================================\n\n";

		LoadSettings loadSettings(Parser.getConfigTreeRoot());
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}
