
#include "ServerSettings.hpp"

ServerSettings::ServerSettings(std::string& HttpRoot, 
		std::string& HttpAutoIndex, std::string& HttpClientMaxBodySize,
		std::string& HttpErrorPagesContext, std::vector<DirectiveNode* >& HttpErrorArgs, 
		std::vector<DirectiveNode* >& HttpIndexArgs)
		: BaseSettings(HttpRoot, HttpAutoIndex, HttpClientMaxBodySize, HttpErrorPagesContext, HttpErrorArgs, HttpIndexArgs)
{
	setDefaultValues();
}

void ServerSettings::setDefaultValues()
{
	this->port = DEFAULT_SERVER_PORT;
	this->ip = DEFAULT_SERVER_IP;
	if (this->root.empty())
		this->root = DEFAULT_SERVER_ROOT;
	if (this->autoIndex.empty())
		this->autoIndex = DEFAULT_SERVER_AUTOINDEX;
	if (this->clientMaxBodySize == std::numeric_limits<size_t>::max())
		this->clientMaxBodySize = DEFAULT_SERVER_CLIENT_MAX_BODY_SIZE;
	if (this->index.empty())
		this->index.push_back(DEFAULT_SERVER_INDEX);
}

void ServerSettings::setPort(const std::string& portStr, const std::string& listenValue)
{
	std::stringstream ss(portStr);
	size_t	portVal;

	ss >> portVal;
	if (ss.fail() || !ss.eof())
		throw (std::runtime_error("invalid port in '\"" + listenValue + "\" of the \"listen\" directive"));
	if (portVal > 65535)
		throw (std::runtime_error("invalid port in '\"" + listenValue + "\" of the \"listen\" directive"));
	this->port = portVal;
}

void ServerSettings::setIP(const std::string& IPv4, const std::string& listenValue)
{
	if (IPv4 == "localhost")
		return ;

	int			octetCount = 00;
	std::string octet;
	std::stringstream ss(IPv4);
	while (ss.good() && octetCount < 4)
	{
		std::getline(ss, octet, '.');
		std::stringstream numberStream(octet);
		int octetDigits;
		numberStream >> octetDigits;
		if (numberStream.fail() || !numberStream.eof())
			throw (std::runtime_error("no host found in '\"" + listenValue + "\" of the \"listen\" directive"));
		if (octetDigits < 0 || octetDigits > 255)
			throw (std::runtime_error("no host found in '\"" + listenValue + "\" of the \"listen\" directive"));
		octetCount++;
	}
	if (ss.good())
		throw (std::runtime_error("no host found in '\"" + listenValue + "\" of the \"listen\" directive"));
	this->ip = IPv4;
}

/*
	NOTES

	Note 1: We are basically rejecting anything that is a number, ':', or '.'
			in the listen directive. Examples: 

			anyrandomtext:8080;			localhostrandom:8080;
			randomlocalhost:8080;		localhost:8080alsorandom;

	Note 2: handles 	:8080;
*/
void	ServerSettings::setListenValues(const std::string &listenValue)
{
	size_t	i = listenValue.find("localhost");

	if (i == std::string::npos)
		i = 0;

	// Note 1
	if (listenValue.substr(0, i).find_first_not_of(".:0123456789") != std::string::npos) // search before the localhost entry
		throw (std::runtime_error("host not found in \"" + listenValue + "\" of the \"listen\" directive"));
	if (listenValue.substr(i + std::string("localhost").size()).find_first_not_of(".:0123456789") != std::string::npos) // search after the localhost entry
		throw (std::runtime_error("host not found in \"" + listenValue + "\" of the \"listen\" directive"));


	std::string IPv4;
	std::string	portStr;
	size_t pos = listenValue.find(":");

	if (pos != std::string::npos) {
		IPv4 = listenValue.substr(0, pos);
		portStr = listenValue.substr(pos + 1);
		if (IPv4.empty())
			throw (std::runtime_error("no host found in '\"" + listenValue + "\" of the \"listen\" directive")); // Note 2
	}
	else
		portStr = listenValue;

	setPort(portStr, listenValue);
	setIP(IPv4, listenValue);
}

int&	ServerSettings::getPort()
{
	return (port);
}

std::string&	ServerSettings::getIP()
{
	return (ip);
}

void ServerSettings::debugger() const 
{
	// Call debugger for BaseSettings members
	std::cout << "root: " << root << std::endl;
	std::cout << "autoIndex: " << autoIndex << std::endl;
	std::cout << "clientMaxBodySize: " << clientMaxBodySize << std::endl;

	// Print errorPages
	std::cout << "errorPages:" << std::endl;
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
		std::cout << "  [" << it->first << "] = " << it->second << std::endl;
	}

	// Print errorPagesLevel
	std::cout << "errorPagesLevel:" << std::endl;
	for (std::map<int, std::string>::const_iterator it = errorPagesLevel.begin(); it != errorPagesLevel.end(); ++it) {
		std::cout << "  [" << it->first << "] = " << it->second << std::endl;
	}

	// Print index
	std::cout << "index:" << std::endl;
	for (std::vector<std::string>::const_iterator it = index.begin(); it != index.end(); ++it) {
		std::cout << "  " << *it << std::endl;
	}

	// Print returnDirective
	std::cout << "returnDirective:" << std::endl;
	std::cout << "    statusCode: " << returnDirective.getStatusCode() << std::endl;
	std::cout << "    textOrURL : " << returnDirective.getTextOrURL() << std::endl;

	// Print ServerSettings members
	std::cout << "port: " << port << std::endl;
	std::cout << "ip: " << ip << std::endl;
}
