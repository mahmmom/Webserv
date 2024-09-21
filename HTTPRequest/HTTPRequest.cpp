
#include "HTTPRequest.hpp"

/*
		CONSTRUCTOR
*/
HTTPRequest::HTTPRequest(const std::string& full_request) {
	tokenizeHeaderFields(full_request);
}

void HTTPRequest::tokenizeHeaderFields(const std::string& fullRequest) {
	std::vector<std::string>	headerFields;

	for (size_t i = 0; i < fullRequest.size();) {
		size_t pos = fullRequest.find("\r\n", i);
		std::string	field = fullRequest.substr(i, pos - i);
		if (field.empty())
			break ;
		headerFields.push_back(field);
		i += (pos - i) + 2;
	}
	// std::cout << "==========================" << std::endl;
	// for (size_t i = 0; i < headerFields.size(); i++) {
	// 	std::cout << "[" << i << "]" << headerFields[i] << std::endl;
	// }
	// std::cout << "==========================" << std::endl;
	processRequestLine(headerFields[0]);
	buildHeaderMap(headerFields);
}

bool	processRequestLine(const std::string& requestLine)
{
	return (true);
}

void printMap(const std::map<std::string, std::string>& myMap) {
	std::cout << "============ MAP ============" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = myMap.begin(); it != myMap.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
	std::cout << "===========================" << std::endl;
}

bool	HTTPRequest::buildHeaderMap(std::vector<std::string>& headerFields) {
	

	std::vector<std::string>::const_iterator it = headerFields.begin() + 1;
	for (; it != headerFields.end(); it++) {
		std::string	field = *it;
		std::string	name;
		std::string	lowerName;
		std::string	value;

		size_t pos = field.find(":", 0);
		name = field.substr(0, pos);
		transform(name.begin(), name.end(), name.begin(), ::tolower);
		pos = field.find_first_not_of(" ", pos + 1); // pos + 1 to skip the ":"
		value = field.substr(pos, field.length());
		headers[name] = value;
	}

	printMap(headers);
	std::cout << "==========================" << std::endl;

	return (true);
}

/* 
		ORTHODOX CANONICAL FORM
*/
HTTPRequest::HTTPRequest() {}

HTTPRequest::HTTPRequest(const HTTPRequest& other) {
	*this = other;
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& other) {
	(void) other;
	return (*this);
}

HTTPRequest::~HTTPRequest() {}
