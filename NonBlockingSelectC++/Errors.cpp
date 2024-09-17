#include "Errors.hpp"

const char* SocketException::what() const throw()
{
    return "Socket error";
}

const char* SetsockoptException::what() const throw()
{
    return "Setsockopt error";
}

const char* BindException::what() const throw()
{
    return "Bind error";
}

const char* ListenException::what() const throw()
{
    return "Listen error";
}

const char* AcceptException::what() const throw()
{
    return "Accept error";
}

const char* FcntlException::what() const throw()
{
    return "Fcntl error";
}

const char* SelectException::what() const throw()
{
    return "Select error";
}

const char* ReadException::what() const throw()
{
    return "Read error";
}

const char* WriteException::what() const throw()
{
    return "Write error";
}

const char* CloseException::what() const throw()
{
    return "Close error";
}