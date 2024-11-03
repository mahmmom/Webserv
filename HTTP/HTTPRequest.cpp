
#include "HTTPRequest.hpp"
#include <algorithm>

void	HTTPRequest::debugger()
{
	std::cout << "\n*********************************************" << std::endl;
	std::cout << "============ METHOD ============" << std::endl;
	std::cout << "Method -> " << method << std::endl;
	std::cout << "================================\n" << std::endl;

	std::cout << "============ URI ============" << std::endl;
	std::cout << "URI -> " << uri << std::endl;
	std::cout << "=============================\n" << std::endl;

	std::cout << "============ VERSION ============" << std::endl;
	std::cout << "Version -> " << version << std::endl;
	std::cout << "=================================\n" << std::endl;

	std::cout << "============ QUERIES ============" << std::endl;
	for (size_t i = 0; i < queries.size(); i++) {
		std::cout << "Query [" << i << "] -> " << queries[i] << std::endl;
	}
	std::cout << "=================================\n" << std::endl;

	std::cout << "============ MAP ============" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
	std::cout << "===========================\n" << std::endl;

	std::cout << "============ STATUS ============" << std::endl;
	std::cout << "Status -> " << status << std::endl;
	std::cout << "================================\n" << std::endl;

	std::cout << "*********************************************\n" << std::endl;
}

/*
		CONSTRUCTOR
*/
HTTPRequest::HTTPRequest(const std::string& full_request, int clientSocketFD) : 
	status(0), clientSocketFD(clientSocketFD)
{
	// (void) status;
	tokenizeHeaderFields(full_request);
}

/*
	Note 1: If there were no two consecutive \r\n in the whole request,
			then that means that the requests were incomplete as a proper 
			HTTP request as to have an empty line at the bottom of it. 
			Thus, if there is no empty line at the bottom, it will  
			assume that there is at least one more header field which was 
			not sent. In this case, HTTP servers do not return a status code, 
			they wait for more to come (or discard the request). For us, I 
			guess it is easier to just discard and disconnect the client.

	Note 2: remember, that .find() returns the pos from the beginning
			of the string (fullRequest) and not at an offset from i. 
			I need the offset for the third argument of the substring 
			because it the len argument. Thus, I calculate the offset 
			(aka len) myself as pos - i.

	Note 3: .find() returns the index (or position) at which the first 
			occurence of the str argument ("\r\n") in this case and thus, 
			for the next iteration, meaning that i is now at the \r position.
			So to skip past both \r and \n, we add a further 2 to the len 
			(pos - i).
*/
void HTTPRequest::tokenizeHeaderFields(const std::string& fullRequest) {
	std::vector<std::string>	headerFields;

	if (fullRequest.find("\r\n\r\n") == std::string::npos) { // Note 1
		std::cout << "Incomplete headers" << std::endl;
		return ;
	}

	for (size_t i = 0; i < fullRequest.size();) {
		size_t pos = fullRequest.find("\r\n", i);
		if (pos == std::string::npos)
			return ;	// set status to bad request here (NEVERMIND, NO DON'T!)
		std::string	field = fullRequest.substr(i, pos - i); // Note 2
		if (field.empty())
			break ;
		headerFields.push_back(field);
		i += (pos - i) + 2;	// Note 3
	}
	processRequestLine(headerFields[0]);
	buildHeaderMap(headerFields);
	normalizeURI();
	// debugger();
}

/*
	Note 1:	checking for http://example.com/path? which is valid but just 
			indicates empty query string
	Note 2:	checking for http://example.com/path?key1=val1&key2=val2& which 
			is valid but just indicates that there is an empty query argument
			in the query list, so we just neglect it and don't even want to 
			include it in our queries vector. 
*/
void	HTTPRequest::extractQueries(const std::string& querieString)
{
	std::stringstream	ss;
	std::string			token;

	if (querieString.empty())	// Note 1
		return ;
	ss << querieString;
	while (std::getline(ss, token, '&'))
	{
		if (!token.empty())	// Note 2
			queries.push_back(token);
	}
	return ;
}

/*
	Note 1: this section attempts to decode the URL-encoding 
			which converts all spaces to "%20". Here, I am 
			resetting these %20 back to their original form, 
			spaces (hence the decoding). It is also important
			to note that this decoding before the extractQueries
			section because I the extractQueries assigns the 
			queries vector in the HTTP class. I want the query
			string arguments to already be decoded by the time 
			I extract them. I don't want them to be written with
			%20's. So if a key-value pair was something like 
			"First Name=Joe", then it would be encoded as 
			/form?full%20name=joe. In my vector, I want this 
			"full name=joe", not "full%20name=joe".
*/
bool	HTTPRequest::processURI(std::string& uriTok)
{
	if (uriTok.find("/") == std::string::npos)
		return (false); // confirmed
	if (uriTok.find("%20") != std::string::npos) {	// Note 1
		for (size_t i = 0; i < uriTok.length(); i++) {
			size_t pos = uriTok.find("%20", i);
			if (pos != std::string::npos)
				uriTok.replace(pos, 3," ");
		}
	}
	size_t	breakPoint = 0;
	breakPoint = uriTok.find("?", 0);
	if (breakPoint != std::string::npos) {
		extractQueries(uriTok.substr(breakPoint + 1));
		uri = uriTok.substr(0, breakPoint);
		return (true);
	}
	uri = uriTok;
	return (true);
}

/*
	Note 1: this line aims to check if no version is present in
			the request line, which is unacceptable of course.
	
	Note 2: pos != 0 makes sure that there are no weird characters
			before HTTP/ so for example: hiHTTP/1.1 would be 
			rejected.
	
	Note 3: this checks if there is anything after the version besides 
			a space, which should be rejected.
*/
bool	HTTPRequest::processVersion(std::string& versionTok)
{
	if (versionTok.empty()) // Note 1
		return (setStatus(400), false); // CONFIRMED
	size_t pos = versionTok.find("HTTP/");
	if (pos == std::string::npos && pos != 0) // Note 2
		return (setStatus(400), false); // CONFIRMED
	if (versionTok.substr(5, 3) != "1.1")
		return (setStatus(505), false);	// CONFIRMED
	if (versionTok.substr(8).find_first_not_of(" ") != std::string::npos) // Note 3
		return (setStatus(400), false); // CONFIRMED
	version = versionTok;
	return (true);
}

bool HTTPRequest::isRequestLineComplete(const std::string& requestLine)
{
    size_t pos = 0;
    size_t tokenCount = 0;

    while (pos != std::string::npos) {
        // Find the first non-space character
        pos = requestLine.find_first_not_of(' ', pos);
        if (pos == std::string::npos)
            break;
        tokenCount++;

        // Find the next space after the token
        pos = requestLine.find_first_of(' ', pos);
    }

    return tokenCount == 3;
}

/*
	Note 1:	As per RFC 7230 3.1.1, this is how a request-line should look:
			request-line   = method SP request-target SP HTTP-version CRLF
			Now Nginx has its implementation where if there is a SP before the
			method, it rejects the request. But with the other SP's, technically
			although there should only be a single one, Nginx accepts multiple
			spaces and even accepts trailing spaces (those that come after the 
			HTTP-version). So that's why I have the if i != 0 check.
*/
bool	HTTPRequest::processRequestLine(const std::string& requestLine)
{
	std::string					methodTok;
	std::string					uriTok;
	std::string					versionTok;
	size_t						i;
	size_t						pos;
	std::vector<std::string>	allowedMethods;

	allowedMethods.push_back("GET");
	allowedMethods.push_back("POST");
	allowedMethods.push_back("HEAD");
	allowedMethods.push_back("DELETE");

	if (!isRequestLineComplete(requestLine))
		return (setStatus(400), false);

	i = 0;
	pos = 0;
	i = requestLine.find_first_not_of(" ", pos);
	pos = requestLine.find(" ", i);
	if ((i != 0) || pos == std::string::npos) // Note 1
		return (setStatus(400), false);
	methodTok = requestLine.substr(i, pos - i);
	if (std::find(allowedMethods.begin(), allowedMethods.end(), methodTok) == allowedMethods.end())
		return (setStatus(501), false);
	method = methodTok;

	i = requestLine.find_first_not_of(" ", pos);
	pos = requestLine.find(" ", i);
	if (pos == std::string::npos)
		return(setStatus(400), false);
	uriTok = requestLine.substr(i, pos - i);
	if (!processURI(uriTok))
		return(false);

	i = requestLine.find_first_not_of(" ", pos);
	if (i == std::string::npos)
		return (setStatus(400), false);
	versionTok = requestLine.substr(i);
	if (!processVersion(versionTok))
		return (false);

	return (true);
}

bool	HTTPRequest::processHostField(std::string& hostValue)
{
	if (hostValue.find(" ") != std::string::npos)
		return (setStatus(400), false);
	return (true);
}

bool	HTTPRequest::processPostRequest()
{
	/*
		If there is no content-length, our server will consider
		that acceptable only if we are using transfer-encoding AND 
		this transfer-encoding has to be chunked!
	*/
	if (headers.find("content-length") == headers.end())
	{
		std::map<std::string, std::string>::const_iterator it;
		it = headers.find("transfer-encoding");
		if (it == headers.end() || it->second != "chunked")
			return (setStatus(411), false); // RFC 7230 3.3.3
	}
	if (headers.find("content-length") != headers.end()) {
		if (headers.find("transfer-encoding") != headers.end())
			return (headers.erase("content-length"), true); // RFC 2616 SECTION 4.4 bullet point 3
		if (headers.find("content-length")->second.find_first_not_of("0123456789") != std::string::npos)
			return (setStatus(400), false); // RFC 7230 3.3.2		
	}
	return (true);
}

bool	HTTPRequest::checkDuplicates(std::string& headerFieldName)
{
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); it++) {
		if (it->first == headerFieldName)
			return (false);
	}
	return (true);
}

/*
	Nginx ignores header field entries where there is a whitespace 
	before the ":", instead of completely rejecting the request. 
	But RFC 7230 3.2 clearly states: 
		header-field   = field-name ":" OWS field-value OWS 
	where OWS is optional white space. There is no OWS before 
	the ":" though.
*/
bool	HTTPRequest::buildHeaderMap(std::vector<std::string>& headerFields)
{
	std::vector<std::string>::const_iterator it = headerFields.begin() + 1;
	for (; it != headerFields.end(); it++) {
		std::string	field = *it;
		std::string	name;
		std::string	lowerName;
		std::string	value;

		size_t pos = field.find(":", 0);
		name = field.substr(0, pos);
		if (name.find(" ") != std::string::npos) // Note 1
			continue ;
		transform(name.begin(), name.end(), name.begin(), ::tolower);
		if (name == "host" || name == "content-length" || name == "transfer-encoding") {
			if (!checkDuplicates(name))
				return (setStatus(400), false);
		}
		pos = field.find_first_not_of(" ", pos + 1); // pos + 1 to skip the ":"
		value = field.substr(pos, field.length());
		headers[name] = value;
		if (name == "host") {
			if (!processHostField(value))
				return (false);
		}
	}
	if (headers.find("host") == headers.end())
		return (setStatus(400), false);
	if (this->getMethod() == "POST") {
		if (!processPostRequest())
			return (false);
	}
	return (true);
}

/*
	GENERAL

		Overall, this function aims to merge trailing slashes if present in a request into 
		one slash thereby changing [ GET //////index//index.html ] to [ GET /index/index.html ]

	NOTES

		Note 1:	This is used to check for ".." sequence to prevent directory traversal 
				and stop attackers from accessing directories and files outside the scope 
				of the web application.
*/
void HTTPRequest::normalizeURI()
{
    std::string normalizedURI;
    bool lastWasSlash = false;

    for (size_t i = 0; i < uri.size(); ++i) {
        if (uri[i] == '.' && i + 1 < uri.size() && uri[i + 1] == '.') { // Note 1
            i++; // Skip the next character
            continue; // Continue to the next iteration
        }

        if (uri[i] == '/') {
            if (!lastWasSlash) {
                normalizedURI += uri[i];
                lastWasSlash = true; // Set flag for last character being a slash
            }
        } else {
            normalizedURI += uri[i];
            lastWasSlash = false; // Reset flag since current character is not a slash
        }
    }

    uri = normalizedURI; // Update the original URI with the normalized version
}


void HTTPRequest::setStatus(const int& status)
{
	this->status = status;
}

std::string	HTTPRequest::getHeader(const std::string& headerName)
{
	std::map<std::string, std::string>::const_iterator it;

	it = headers.find(headerName);
	if (it != headers.end())
		return (it->second);
	else
		return ("");
}

const std::string& 	HTTPRequest::getURI() const
{
	return (uri);
}

const std::string&	HTTPRequest::getMethod() const
{
	return (method);
}

const int& HTTPRequest::getStatus() const
{
	return (status);
}

void HTTPRequest::setURI(const std::string& uri)
{
	this->uri = uri;
}

/*
		ORTHODOX CANONICAL FORM
*/
HTTPRequest::HTTPRequest() {}

HTTPRequest::~HTTPRequest() {}
