
#include "ReturnDirective.hpp"

ReturnDirective::ReturnDirective() : statusCode(-1), isEnabled(false) {}

void ReturnDirective::setStatusCode(const int& statusCode)
{
	this->isEnabled = true;
	this->statusCode = statusCode;
}

void ReturnDirective::setTextOrURL(const std::string& textOrURL)
{
	this->isEnabled = true;
	this->textOrURL = textOrURL;
}

bool ReturnDirective::getEnabled() const
{
	return (isEnabled);
}

int	ReturnDirective::getStatusCode() const
{
	return (statusCode);
}

std::string	ReturnDirective::getTextOrURL() const
{
	return (textOrURL);
}
