#include "MimeTypesSettings.hpp"
#include <iostream>

MimeTypesSettings::MimeTypesSettings(const std::string &filePath) : 
    _filePath(filePath), lineNumber(0), foundTypesBlock(false), foundClosingBrace(false) {}

MimeTypesSettings::~MimeTypesSettings() {}

// Transform an integer to a string
static std::string intToString(int number) {
    std::ostringstream oss;
    oss << number;
    return oss.str();
}

// Open file function
void MimeTypesSettings::openFile(std::ifstream& file) {
    file.open(_filePath.c_str());
    if (!file)
        throw std::runtime_error("Error: Failed to open file '" + _filePath + "'. Please check if the file exists and has the correct permissions.");
    file.seekg(0, std::ios::end);
    if (file.tellg() == 0)
    {
        file.close();
        throw std::runtime_error("Error: File '" + _filePath + "' is empty.");
    }
    file.seekg(0, std::ios::beg);
}

// If else statement
bool MimeTypesSettings::isMimeTypesComponent(const std::string &line) {
    if (!line.empty() && foundClosingBrace)
        throw std::runtime_error("Error: The file contains extra characters after the closing '}' brace.");

    if (line.find("types {") != std::string::npos && foundTypesBlock)
        throw std::runtime_error("Error: The 'types {' block must not be repeated.");

    if (line.empty() || line[0] == '#')
        return true;;

    if (!foundTypesBlock)
    {
        if (line.find("types {") != std::string::npos)
        {
            foundTypesBlock = true;
            return true;;
        }
        else
            throw std::runtime_error("Error: The file must start with 'types {' (found at line " + intToString(lineNumber) + ").");
    }

    if (line.find("}") != std::string::npos)
    {
        if (line.size() != 1)
            throw std::runtime_error("Error: The closing '}' brace must not contain any extra characters.");
        foundClosingBrace = true;
        return true;;
    }

    if(foundClosingBrace)
        throw std::runtime_error("Error: The file contains extra characters after the closing '}' brace.");

    return false;
}

// Remove Semi-colon and add to map
void MimeTypesSettings::addMimeType(const std::string &line)
{
    std::istringstream iss(line);
    std::string mimeType;
    std::string extension;

    iss >> mimeType;
    // Extract all extensions
    while (iss >> extension)
    {
        if (!extension.empty() && extension[extension.size() - 1] == ';')
        {
            // Remove the trailing semicolon before adding it
            extension.erase(extension.size() - 1);
            mimeTypes_[mimeType].push_back(extension);
            break;
        }
        else
            mimeTypes_[mimeType].push_back(extension);
    }
}

// Parse the MIME types file
void MimeTypesSettings::parse()
{
    std::ifstream file;
    std::string line;

    openFile(file);
    while (std::getline(file, line))
    {
        lineNumber++;
        trim(line);
        if (isMimeTypesComponent(line))
            continue;
        validateMimeTypeLine(line, lineNumber);
        addMimeType(line);
    }

    if (!foundClosingBrace)
        throw std::runtime_error("Error: closing '}' brace not found");

    file.close();
}

void MimeTypesSettings::validateMimeTypeLine(const std::string &line, int lineNumber)
{
    std::istringstream iss(line);
    std::string mimeType;
    std::string extension;
    bool foundSemicolon = false;
    
    if (!(iss >> mimeType))
        throw std::runtime_error("Error: Line " + intToString(lineNumber) + " must contain a MIME type and at least one extension.");

    while (iss >> extension)
    {
        if (foundSemicolon)
            throw std::runtime_error("Error: Line " + intToString(lineNumber) + " has extra characters after the semicolon.");

        if (!extension.empty() && extension[extension.size() - 1] == ';')
        {
            foundSemicolon = true;
            
            // Check if the semicolon is not at the end of the extension
            if (extension.size() > 1 && extension.find(';') != extension.size() - 1)
                throw std::runtime_error("Error: Line " + intToString(lineNumber) + " contains a misplaced semicolon.");
            
            // Remove the semicolon and check if the extension is empty
            extension.erase(extension.size() - 1);
            if (extension.empty())
                throw std::runtime_error("Error: Line " + intToString(lineNumber) + " contains an empty extension before the semicolon.");

            break;  // Exit the loop after finding the semicolon
        }
    }

    if (!foundSemicolon)
        throw std::runtime_error("Error: Line " + intToString(lineNumber) + " must end with a single semicolon.");

    // Check for any remaining input after the semicolon
    std::string extra;
    if (iss >> extra)
        throw std::runtime_error("Error: Line " + intToString(lineNumber) + " has extra characters after the semicolon.");
}

std::string MimeTypesSettings::getMimeType(const std::string &filepath) const
{
    std::string extension;

    size_t pos = filepath.find_last_of(".");
    if (pos != std::string::npos)
        extension = filepath.substr(pos + 1);
    else
        return "application/octet-stream";
    
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = mimeTypes_.begin(); it != mimeTypes_.end(); ++it)
    {
        const std::vector<std::string> &extensions = it->second;
        for (size_t i = 0; i < extensions.size(); ++i)
        {
            if (extensions[i] == extension)
                return it->first;
        }
    }
    return "application/octet-stream";
}

void MimeTypesSettings::printMimeTypes() const
{
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = mimeTypes_.begin(); it != mimeTypes_.end(); ++it)
    {
        std::cout << it->first << ":" << std::endl;
        const std::vector<std::string> &extensions = it->second;
        for (size_t i = 0; i < extensions.size(); ++i)
        {
            if (i == extensions.size() - 1)
                std::cout << "  └─ " << extensions[i] << std::endl << std::endl;
            else
                std::cout << "  ├─ " << extensions[i] << std::endl;
        }
    }
}

void MimeTypesSettings::trim(std::string &str) const
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");

    if (start == std::string::npos)
        str = "";
    else
        str = str.substr(start, end - start + 1);
}


