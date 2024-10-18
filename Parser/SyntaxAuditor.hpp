#ifndef SYNTAXAUDITOR_HPP
# define SYNTAXAUDITOR_HPP

#include <vector>
#include <stack>
#include <string>
#include <iostream>
#include <exception>
#include <algorithm>

/*
	UNEXPECTED_EOF:	shows when you have a full set of braces that have been
					opened and closed properly, but there is some extra content
					outside of the last closing brace. This content could be 
					anything, including extra braces.

	BRACE_MISMATCH: shows when you have an extra opening brace somewhere within
					topmost context of the config file and that brace does not 
					have a matching closing brace.
*/

#define EMPTY_CONFIG_FILE			"Configuration file is empty"
#define UNEXPECTED_OPENING_BRACE	"Unexpected \"{\""
#define UNEXPECTED_CLOSING_BRACE	"Unexpected \"}\""
#define BRACE_MISMATCH				"Opening brace not matched with closing brace"
#define MISSING_HTTP				"Missing HTTP context"
#define MISSING_SERVER				"Missing Server context"
#define INVALID_HTTP				"HTTP directive must be immediately followed by an {"
#define INVALID_SERVER				"server directive must be immediately followed by an {"
#define INVALID_LOCATION_ARGS		"Invalid number of arguments for the location directive"
#define INVALID_LOCATION_CONTEXT	"Location context and URI must be followed by an {"
#define UNEXPECTED_EOF				"Unexpected EOF"
#define INVALID_SERVER_POS			"server directive is not allowed here"
#define INVALID_LOCATION_POS		"location directive is not allowed here"
#define INVALID_LIM_EXCEPT_DIR		"limit_except directive and associated methods must be follow by an {"
#define INVALID_LIM_EXCEPT_POS		"limit_except directive is not allowed here"
#define INVALID_LIM_EXCEPT_METHOD	"invalid method used with limit_except"
#define UNEXPECTED_SEMICOLON		"Unexpected ;"
#define MISSING_SEMICOLON			"Directive is missing a ;"
#define	UNKNOWN_CONTEXT				"Unknown context present in file"

class SyntaxAuditor
{
	private:
		static void checkBraces(std::vector<std::string>& tokens);
		static void checkRequiredContexts(std::vector<std::string>& tokens);
		static void checkContextsHierarchy(std::vector<std::string>& tokens);
		static void checkDirectives(std::vector<std::string>& tokens);
	public:
		class SyntaxError;
		static void checkConfigSyntax(std::vector<std::string>& tokens);
};

class SyntaxAuditor::SyntaxError : public std::runtime_error
{
	public:
		SyntaxError(const std::string &errorMessage);

};

#endif