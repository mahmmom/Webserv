
#ifndef BASESETTINGS_HPP
# define BASESETTINGS_HPP

// #include 
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include "../Parser/DirectiveNode.hpp"

class BaseSettings {
	protected:
		std::string					root; //
		std::string					autoIndex; //
		size_t						clientMaxBodySize; //
		std::map<int, std::string>	errorPages; //
		std::map<int, std::string>	errorPagesLevel; //
		std::vector<std::string>	index; //
		// std::map<int, std::string>	returnDirective;

	public:
		BaseSettings(std::string& HttpRoot, 
			std::string& HttpAutoIndex, std::string& HttpClientMaxBodySize,
			std::string& HttpErrorPagesContext, std::vector<DirectiveNode* >& HttpErrorArgs,
			std::vector<DirectiveNode* >& HttpIndexArgs);

		BaseSettings(std::string serverRoot, 
			std::string serverAutoIndex, size_t serverClientMaxBodySize,
			std::map<int, std::string> serverErrorPages, 
			std::map<int, std::string> serverPageLevel, 
			std::vector<std::string> serverIndex);

		std::string getRoot() const;
		std::string getAutoindex() const;
		size_t getClientMaxBodySize() const;
		std::map<int, std::string> getErrorPages() const;
		std::map<int, std::string> getErrorPagesLevel() const;
		std::vector<std::string> getIndex() const;

		void	setRoot(const std::string& root);
		void	setAutoIndex(const std::string& autoindex);
		void	setClientMaxBodySize(const std::string& clientMaxBodySize);
		void	setErrorPages(const std::vector<std::string>& errorArgs, const std::string& errorPagesContext);
		void	setIndex(const std::vector<std::string>& indexArgs);

		void	debugger() const;
};

#endif