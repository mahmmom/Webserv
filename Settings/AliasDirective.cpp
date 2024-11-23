
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

std::string AliasDirective::updateURL(const std::string& originalURL) const
{
   // If alias is not enabled or URL doesn't match, return original
   if (!isEnabled)
       return originalURL;
   
   // Get the part of URL after the alias
   std::string remainingPath;
   if (originalURL.length() > aliasURL.length()) {
       remainingPath = originalURL.substr(aliasURL.length());
   }
   
   return (aliasURL + "/" + remainingPath);
}

// bool AliasDirective::findMatchingURL(const std::string& URL) const
// {
//     if (aliasURL.empty())
//         return false;

//     if (URL.length() < aliasURL.length()) // Check if URL is long enough to contain aliasURL
//         return false;

//     std::cout << "compare " << URL.substr(0, aliasURL.length()) << " with " << aliasURL << std::endl;
//     if (URL.substr(0, aliasURL.length()) == aliasURL)
//         return true;

//     return false;
// }

void AliasDirective::resetAliasDirective()
{
	this->isEnabled = false;
	this->aliasURL.clear();
}

bool AliasDirective::getEnabled() const
{
	return (isEnabled);
}

std::string	AliasDirective::getAliasURL() const
{
	return (aliasURL);
}
