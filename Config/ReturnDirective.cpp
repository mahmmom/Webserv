
#include "ReturnDirective.hpp"

ReturnDirective::ReturnDirective() : statusCode(-1), isEnabled(false) {}

ReturnDirective::ReturnDirective(const ReturnDirective& other)
{
	*this = other;
}

ReturnDirective& ReturnDirective::operator=(const ReturnDirective& other)
{
	if (this != &other)
	{
		this->statusCode = other.statusCode;
		this->textOrURL = other.textOrURL;
		this->isEnabled = other.isEnabled;
	}
	return (*this);
}

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
