
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

	// Remove the preceeding slash so that our isFile and isDirectory functions can detect them
	if (aliasURL[0] == '/')
		this->aliasURL = normalizeURI((aliasURL).substr(1)); 
	else
		this->aliasURL = aliasURL;
}

/*
	GENERAL

		Overall, this function aims to merge trailing slashes if present in a request into 
		one slash thereby changing [ GET //////index//index.html ] to [ GET /index/index.html ]

	NOTES

		Note 1:	This is used to check for ".." sequence to prevent directory traversal 
				and stop attackers from accessing directories and files outside the scope 
				of the web application.
*/
std::string AliasDirective::normalizeURI(const std::string originalURI) const
{
    std::string normalizedURI;
    bool lastWasSlash = false;

    for (size_t i = 0; i < originalURI.size(); ++i) {
        if (originalURI[i] == '.' && i + 1 < originalURI.size() && originalURI[i + 1] == '.') { // Note 1
            i++; // Skip the next character
            continue; // Continue to the next iteration
        }

        if (originalURI[i] == '/') {
            if (!lastWasSlash) {
                normalizedURI += originalURI[i];
                lastWasSlash = true; // Set flag for last character being a slash
            }
        } else {
            normalizedURI += originalURI[i];
            lastWasSlash = false; // Reset flag since current character is not a slash
        }
    }

	return (normalizedURI); // Update the original URI with the normalized version
}

std::string AliasDirective::updateURL(const std::string& originalURL, const std::string& locationPath) const
{
	// Extract the remaining path after the locationPath
	std::string remainingPath = originalURL.substr(locationPath.length());

	return (normalizeURI(aliasURL + remainingPath));
}

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
