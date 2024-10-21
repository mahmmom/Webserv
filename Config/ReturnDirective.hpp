
#ifndef RETURNDIRECTIVE_HPP
# define RETURNDIRECTIVE_HPP

#include <string>
#include <vector>

class ReturnDirective
{
	private:
		int			statusCode;
		std::string textOrURL;
		bool		isEnabled;
	public:
		ReturnDirective();
		bool		getEnabled() const;
		void 		setStatusCode(const int& statusCode);
		void 		setTextOrURL(const std::string& textOrURL);
		int			getStatusCode() const;
		std::string	getTextOrURL() const;
};

#endif