
#include "AliasDirective.hpp"

AliasDirective::AliasDirective() : isEnabled(false) {}

AliasDirective::AliasDirective(const AliasDirective& other)
{
	*this = other;
}

AliasDirective& AliasDirective::operator=(const AliasDirective& other)
{
	if (this != &other)
	{
		this->aliasURL = other.aliasURL;
		this->isEnabled = other.isEnabled;
	}
	return (*this);
}

void AliasDirective::setAliasURL(const std::string& aliasURL)
{
	this->isEnabled = true;
	this->aliasURL = aliasURL;
}

bool AliasDirective::getEnabled() const
{
	return (isEnabled);
}

std::string	AliasDirective::getAliasURL() const
{
	return (aliasURL);
}
