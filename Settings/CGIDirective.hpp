
#ifndef CGIDIRECTIVE_HPP
# define CGIDIRECTIVE_HPP

#include <vector>
#include <string>

class CGIDirective
{
	private:
		std::vector<std::string> 	extensions;
		bool						enabled;
	public:
		CGIDirective();
		CGIDirective(const CGIDirective& other);
		~CGIDirective();
		CGIDirective& operator=(const CGIDirective& other);
		void	setExtensions(const std::string& extension);
		bool	isEnabled() const;

		const std::vector<std::string>&	getExtensions() const;
};

#endif