
#include "SyntaxAuditor.hpp"

SyntaxAuditor::SyntaxError::SyntaxError(const std::string &errorMessage)
	: std::runtime_error(errorMessage) { }

void SyntaxAuditor::checkBraces(std::vector<std::string>& tokens)
{
	std::stack<std::string> braceStack;
	
	if (tokens.empty())
		throw (SyntaxAuditor::SyntaxError(EMPTY_CONFIG_FILE));
	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "{") {
			if (i == 0)
				throw (SyntaxError(UNEXPECTED_OPENING_BRACE)); // throw
			else if (i > 0 && tokens[i - 1] == "{")
				throw (SyntaxError(UNEXPECTED_OPENING_BRACE));
			else if (i > 0 && tokens[i - 1] == "}")
				throw (SyntaxError(UNEXPECTED_OPENING_BRACE));
			braceStack.push(tokens[i]);
		}
		else if (tokens[i] == "}") {
			if (braceStack.empty())
				throw (SyntaxError(UNEXPECTED_CLOSING_BRACE)); // throw excpetion like wtf } before a { is ever encounter
			braceStack.pop();
			if (braceStack.empty() && ((i + 1) != tokens.size()))
				throw (SyntaxError(UNEXPECTED_EOF)); // later if you wanna implement events, check if tokens[i + 1 ] != events instead
		}
	}
	if (!braceStack.empty())
		throw(SyntaxError(BRACE_MISMATCH)); // throw exception, there was a { without a closing }
}

/*
	Note 1: if (tokens[i] == "deny") because deny is the only thing that's not a method 
			that I will allow after lim_except so this keyword gets a special case; but 
			still, it must also be preceded by a {. So it handles a case like this:

			location / {
				limit_except GET POST 
				deny { all;
			}

			I want it to say in that "limit_except and its associated methods was not followed 
			by a {"" instead of "invalid method used with limit_except" 
*/
void SyntaxAuditor::checkRequiredContexts(std::vector<std::string>& tokens)
{
	if (std::find(tokens.begin(), tokens.end(), std::string("http")) == tokens.end())
		throw (SyntaxError(MISSING_HTTP));
	if (std::find(tokens.begin(), tokens.end(), std::string("server")) == tokens.end())
		throw (SyntaxError(MISSING_SERVER));

	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "http") {
			if (i + 1 < tokens.size() && tokens[i + 1] != "{")
				throw (SyntaxError(INVALID_HTTP));
		}
		else if (tokens[i] == "server") {
			if (i + 1 < tokens.size() && tokens[i + 1] != "{")
				throw (SyntaxError(INVALID_SERVER));
		}
		else if (tokens[i] == "location")
		{
			if (i + 1 < tokens.size() 
					&& (tokens[i + 1] == "}"
						|| tokens[i + 1] == "{"
						|| tokens[i + 1] == ";"))
				throw (SyntaxError(INVALID_LOCATION_ARGS));
			if (i + 2 < tokens.size() && tokens[i + 2] != "{")
				throw (SyntaxError(INVALID_LOCATION_CONTEXT));
		}
		else if (tokens[i] == "limit_except")
		{
			std::vector<std::string> valid_entries;

			valid_entries.push_back("GET");
			valid_entries.push_back("POST");
			valid_entries.push_back("HEAD");
			valid_entries.push_back("DELETE");

			i++;
			while (i < tokens.size() && (std::find(valid_entries.begin(), valid_entries.end(), tokens[i]) != valid_entries.end()))
				i++;
			if (i == tokens.size())	// chances are this condition will never happen due to previous checks but keep it just in case
				throw (SyntaxError(UNEXPECTED_EOF));
			if (tokens[i] != "{" && std::find(valid_entries.begin(), valid_entries.end(), tokens[i]) == valid_entries.end())
			{
				if (tokens[i] == "}")
					throw (SyntaxError(UNEXPECTED_CLOSING_BRACE));
				if (tokens[i] == "deny" || tokens[i] == ";")
					throw (SyntaxError(INVALID_LIM_EXCEPT_DIR));
				throw (SyntaxError(INVALID_LIM_EXCEPT_METHOD));
			}
			else if (tokens[i] != "{")
				throw (SyntaxError(INVALID_LIM_EXCEPT_DIR));
		}
	}
}

void SyntaxAuditor::checkContextsHierarchy(std::vector<std::string>& tokens)
{
	std::stack<std::string>	contextStack;

	for (size_t i = 0; i < tokens.size(); i++) {
		if (tokens[i] == "http")
			contextStack.push(tokens[i]);	// no need to check if it was empty before adding http because 
											// that is already handled by the next if statement if http context
											// is there and if it is not at all, checkRequiredContexts would have 
											// detected that
		else if (tokens[i] == "server") {
			if (contextStack.empty() || (contextStack.top() != "http"))
				throw (SyntaxError(INVALID_SERVER_POS));
			contextStack.push(tokens[i]);
		}
		else if (tokens[i] == "location") {
			if (contextStack.empty() || (contextStack.top() != "server"))
				throw (SyntaxError(INVALID_LOCATION_POS));
			contextStack.push(tokens[i]);
		}
		else if (tokens[i] == "limit_except") {
			if (contextStack.empty() || (contextStack.top() != "location"))
				throw (SyntaxError(INVALID_LIM_EXCEPT_POS));
			contextStack.push(tokens[i]);
		}
		else if (tokens[i] == "}") {
			if (contextStack.empty())
				throw (SyntaxError(UNEXPECTED_CLOSING_BRACE));
			contextStack.pop();
		}
	}
}

/*
	This function effectively only checks if that last directive in 
	a context (or before the appearance of another context) is 
	missing a semicolon. This is a limitation due to the way we 
	are parsing, where we abstract away all the lines and just list 
	all entries in a token list. So anyways, if a directive is placed 
	in a position before the aforementioned position, it will actually 
	be checked by the logic validator later on (where we check if each 
	directive has the right amount and value of arguments it is to 
	expect). So, basically, directives in those positions missing a 
	semicolon are parsed indirectly and rejected for having invalid 
	arguments instead.
*/
void SyntaxAuditor::checkDirectives(std::vector<std::string>& tokens)
{
	std::vector<std::string>	skipList;

	skipList.push_back("http");
	skipList.push_back("server");
	skipList.push_back("location");
	skipList.push_back("limit_except");
	skipList.push_back("{");
	skipList.push_back("}");

	std::vector<std::string> valid_entries;

	valid_entries.push_back("GET");
	valid_entries.push_back("POST");
	valid_entries.push_back("HEAD");
	valid_entries.push_back("DELETE");

	for (size_t i = 0; i < tokens.size(); i++) {
		if (i < tokens.size() && std::find(skipList.begin(), skipList.end(), tokens[i]) != skipList.end())
		{
			if (tokens[i] == "location")
				i++;
			else if (tokens[i] == "limit_except")
			{
				i++;
				while (i < tokens.size() && std::find(valid_entries.begin(), valid_entries.end(), tokens[i]) != valid_entries.end())
					i++;
			}
			continue;
		}
		if (i < tokens.size() && tokens[i] == ";")
			throw (SyntaxError(UNEXPECTED_SEMICOLON));
		while (i < tokens.size() && tokens[i] != ";" && std::find(skipList.begin(), skipList.end(), tokens[i]) == skipList.end())
			i++;
		if (i == tokens.size() || std::find(skipList.begin(), skipList.end(), tokens[i]) != skipList.end())
			throw (SyntaxError(MISSING_SEMICOLON));
	}
}

void SyntaxAuditor::checkConfigSyntax(std::vector<std::string>& tokens)
{
	std::cout << "check 1\n";
	checkBraces(tokens);
	std::cout << "check 2\n";
	checkRequiredContexts(tokens);
	std::cout << "check 3\n";
	checkContextsHierarchy(tokens);
	std::cout << "check 4\n";
	checkDirectives(tokens);
}
