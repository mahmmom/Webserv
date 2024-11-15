
#include "CGIDirective.hpp"

CGIDirective::CGIDirective() : enabled(false) {}

CGIDirective::CGIDirective(const CGIDirective& other)
    : extensions(other.extensions), enabled(other.enabled) {}

// Assignment Operator
CGIDirective& CGIDirective::operator=(const CGIDirective& other) {
    if (this != &other) {
        extensions = other.extensions;
        enabled = other.enabled;
    }
    return *this;
}

// Destructor
CGIDirective::~CGIDirective() {}

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
