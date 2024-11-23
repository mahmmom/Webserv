
#ifndef ALIASDIRECTIVE_HPP
# define ALIASDIRECTIVE_HPP

#include <string>
#include <iostream>

class AliasDirective
{
	private:
		std::string	aliasURL;
		bool		isEnabled;
	public:
		AliasDirective(const AliasDirective& other);
		AliasDirective& operator=(const AliasDirective& other);
		AliasDirective();

		std::string	updateURL(const std::string& originalURL, const std::string& locationPath) const;
		// bool		findMatchingURL(const std::string& URL) const;		
		void 		resetAliasDirective();

		bool		getEnabled() const;
		std::string	getAliasURL() const;

		void 		setAliasURL(const std::string& aliasURL);
};

#endif