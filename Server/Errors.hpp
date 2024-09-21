#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>

class SocketException : public std::exception
{
    public:
        const char* what() const throw();
};

class SetsockoptException : public std::exception
{
    public:
        const char* what() const throw();
};

class BindException : public std::exception
{
    public:
        const char* what() const throw();
};

class ListenException : public std::exception
{
    public:
        const char* what() const throw();
};

class AcceptException : public std::exception
{
    public:
        const char* what() const throw();
};

class FcntlException : public std::exception
{
    public:
        const char* what() const throw();
};

class SelectException : public std::exception
{
    public:
        const char* what() const throw();
};

class ReadException : public std::exception
{
    public:
        const char* what() const throw();
};

class WriteException : public std::exception
{
    public:
        const char* what() const throw();
};

class CloseException : public std::exception
{
    public:
        const char* what() const throw();
};

/*
    My section
*/
class KqueueException : public std::exception
{
    public:
        const char* what() const throw();
};

#endif