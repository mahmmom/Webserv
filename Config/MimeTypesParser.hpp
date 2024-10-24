#ifndef MIMETYPESPARSER_HPP
#define MIMETYPESPARSER_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <stdexcept>
#include "Colors.hpp"

class MimeTypesParser
{
public:
    MimeTypesParser(MimeTypesParser& other); // Need to fix this issue
    MimeTypesParser& operator=(MimeTypesParser& other);
    MimeTypesParser(const std::string &filePath);
    MimeTypesParser();
    ~MimeTypesParser();

    void parse();
    std::string getMimeType(const std::string &extension) const;
    void printMimeTypes() const;
    void openFile();
    bool isMimeTypesComponent(const std::string &line);
    void addMimeType(const std::string &line);
    std::ifstream &getFile();

private:
    std::map<std::string, std::vector<std::string> > mimeTypes_;
    std::string _filePath;
    std::ifstream _file;
    int lineNumber;
    bool foundTypesBlock;
    bool foundClosingBrace;

    void trim(std::string &str) const;
    void validateMimeTypeLine(const std::string &line, int lineNumber);
};

#endif