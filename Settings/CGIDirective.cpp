
#include "CGIDirective.hpp"

CGIDirective::CGIDirective() : enabled(false) {}

void	CGIDirective::setExtensions(const std::string& extension)
{
	this->extensions.push_back(extension);
	this->enabled = true;
}

bool	CGIDirective::isEnabled() const
{
	return (enabled);
}

const std::vector<std::string>&	CGIDirective::getExtensions() const
{
	return (extensions);
}
