
#ifndef ALIASDIRECTIVE_HPP
# define ALIASDIRECTIVE_HPP

#include <string>

class AliasDirective
{
	private:
		std::string	aliasURL;
		bool		isEnabled;
	public:
		AliasDirective(const AliasDirective& other);
		AliasDirective& operator=(const AliasDirective& other);
		AliasDirective();

		bool		getEnabled() const;
		std::string	getAliasURL() const;

		void 		setAliasURL(const std::string& aliasURL);
};

#endif