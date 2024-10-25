#ifndef MIMETYPESSETTINGS_HPP
#define MIMETYPESSETTINGS_HPP

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <stdexcept>
#include <algorithm>

class MimeTypesSettings
{
public:
    MimeTypesSettings(const std::string &filePath);
    ~MimeTypesSettings();

    void parse();
    std::string getMimeType(const std::string &filepath) const;
    void printMimeTypes() const;
    void openFile(std::ifstream& file);
    bool isMimeTypesComponent(const std::string &line);
    void addMimeType(const std::string &line);

private:
    std::map<std::string, std::vector<std::string> > mimeTypes_;
    std::string _filePath;
    int lineNumber;
    bool foundTypesBlock;
    bool foundClosingBrace;

    void trim(std::string &str) const;
    void validateMimeTypeLine(const std::string &line, int lineNumber);
};

#endif