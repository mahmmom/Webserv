
#ifndef BASESETTINGS_HPP
# define BASESETTINGS_HPP

#define MIN_KEEPALIVE_TIMEOUT 5
#define MAX_KEEPALIVE_TIMEOUT 30

// #include 
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>
#include <limits>
#include "ReturnDirective.hpp"
#include "../Parser/DirectiveNode.hpp"

class BaseSettings {
	protected:
		std::string					root; //
		std::string					autoIndex; //
		size_t						clientMaxBodySize; //
		size_t						keepaliveTimeout;
		std::map<int, std::string>	errorPages; //
		std::map<int, std::string>	errorPagesLevel; //
		std::vector<std::string>	index; //
		ReturnDirective				returnDirective;


		BaseSettings(const BaseSettings& other);
		BaseSettings& operator=(const BaseSettings& other);
		virtual ~BaseSettings();
	public:
		// Constructors
		BaseSettings(std::string& HttpRoot, 							// This constructor is used as a base class constructor 
						std::string& HttpAutoIndex, 					// when defining the ServerSettings class directly from 
						std::string& HttpClientMaxBodySize,				// the tree where all the attributes are still in 
						std::string& HttpKeepaliveTimeout,				// string format. So basically when we are at the 
						std::string& HttpErrorPagesContext, 			// http level of the LoadSettings functions and it 
						std::vector<DirectiveNode* >& HttpErrorArgs,	// is used to create a ServerSettings class.
						std::vector<DirectiveNode* >& HttpIndexArgs);

		BaseSettings(std::string serverRoot, 							// This constructor is used as a base class constructor when 
						std::string serverAutoIndex, 					// defining the LocationSettings class where it takes the  
						size_t serverClientMaxBodySize,					// ServerSettings attributes directly. So since the ServerSettings
						size_t HttpKeepaliveTimeout,	
						std::map<int, std::string> serverErrorPages,	// has all its attributes properly set in the appropriate 
						std::map<int, std::string> serverPageLevel, 	// corresponding data types, we extract them in that form as opposed
						std::vector<std::string> serverIndex);			// to the first scenario, where all the data is still std::string.

		// Getters
		const std::string& getRoot() const;
		const std::string& getAutoindex() const;
		const size_t& getClientMaxBodySize() const;
		const size_t& getKeepaliveTimeout() const;
		const std::map<int, std::string>& getErrorPages() const;
		const std::map<int, std::string>& getErrorPagesLevel() const;
		const std::vector<std::string>& getIndex() const;
		const ReturnDirective& getReturnDirective() const;

		// Setters
		void	setRoot(const std::string& root);
		void	setAutoIndex(const std::string& autoindex);
		void	setClientMaxBodySize(const std::string& clientMaxBodySize);
		void	setKeepAliveTimeout(const std::string& keepAliveTimeout);
		void	setErrorPages(const std::vector<std::string>& errorArgs, const std::string& errorPagesContext);
		void	setIndex(const std::vector<std::string>& indexArgs);
		void	setReturn(const std::vector<std::string>& returnArgs);

		void	debugger() const;
};

#endif