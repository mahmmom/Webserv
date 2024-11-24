
#include "RequestManager.hpp"

RequestManager::RequestManager(std::ofstream* file)
    : currentState(CHUNK_REQUEST_SIZE), 
      expectedChunkSize(0), 
      outputFile(file), 
      isComplete(false)
{}

RequestManager::~RequestManager() {}

// Implement these to prevent copying
RequestManager::RequestManager(const RequestManager& other)
    : currentState(CHUNK_REQUEST_SIZE), 
      expectedChunkSize(0), 
      outputFile(NULL), 
      isComplete(false)
{
    (void)other;
}

RequestManager& RequestManager::operator=(const RequestManager& other)
{
    (void)other;
    return *this;
}

bool RequestManager::processChunkedData(const std::string& data)
{
    size_t position = 0;
    
    // std::cout << "uhm did we 0" << std::endl;

    while (position < data.size() && !isComplete) {
        bool success = false;
        
        switch (currentState) {
            case CHUNK_REQUEST_SIZE:
                success = processChunkSize(data, position);
                break;
            case CHUNK_DATA:
                success = processChunkData(data, position);
                break;
            case CHUNK_ENDING:
                success = processChunkEnding(data, position);
                break;
            case TRAILER:
                success = processTrailer(data, position);
                break;
            case COMPLETE:
                return true;
        }
        
        if (!success) {
            return false;
        }
    }
    
    return true;
}

bool RequestManager::processChunkSize(const std::string& data, size_t& position)
{
    size_t lineEnd = data.find("\r\n", position);

    if (lineEnd == std::string::npos) {
        currentChunk += data.substr(position);
        position = data.size();
        return true;
    }

    std::string sizeLine = currentChunk + data.substr(position, lineEnd - position);
    position = lineEnd + 2;
    
    // Remove chunk extensions if present
    size_t semicolon = sizeLine.find(';');
    if (semicolon != std::string::npos) {
        sizeLine = sizeLine.substr(0, semicolon);
    }

    // Trim whitespace from sizeLine
    while (!sizeLine.empty() && isspace(sizeLine[0]))
        sizeLine.erase(0, 1);
    while (!sizeLine.empty() && isspace(sizeLine[sizeLine.size() - 1]))
        sizeLine.erase(sizeLine.size() - 1);

    expectedChunkSize = hexToDecimal(sizeLine);
    currentChunk.clear();
    
    if (expectedChunkSize == 0) {
        currentState = TRAILER;
        // If we have enough data, try to process the trailer immediately
        if (position < data.size()) {
            size_t savedPosition = position;
            if (processTrailer(data, position)) {
                return true;
            }
            position = savedPosition;
        }
    }
    else {
        currentState = CHUNK_DATA;
    }

    return true;
}

bool RequestManager::processChunkData(const std::string& data, size_t& position)
{
    std::cout << "never" << std::endl;
    size_t remainingChunkSize = expectedChunkSize - currentChunk.size();
    size_t availableData = remainingChunkSize;
    if (data.size() - position < remainingChunkSize) {
        availableData = data.size() - position;
    }
    
    // Write to file
    outputFile->write(data.c_str() + position, availableData);

    // Mirror data to outputBuffer for debugging
    outputBuffer.write(data.c_str() + position, availableData);
    std::cout << "test -> " << outputBuffer.str() << std::endl;

    position += availableData;
    currentChunk += data.substr(position - availableData, availableData);
    
    if (currentChunk.size() == expectedChunkSize) {
        currentChunk.clear();
        currentState = CHUNK_ENDING;
    }
    
    return true;
}

bool RequestManager::processChunkEnding(const std::string& data, size_t& position)
{
    if (position + 2 > data.size()) {
        return true;
    }
    
    if (data.substr(position, 2) != "\r\n") {
        return false;
    }
    
    position += 2;
    currentState = CHUNK_REQUEST_SIZE;
    return true;
}

bool RequestManager::processTrailer(const std::string& data, size_t& position)
{
    // First, check for immediate CRLF (no trailers) [basically an empty body or a POST size of 0]
    if (position + 2 <= data.size() && data.substr(position, 2) == "\r\n") {
        position += 2;
        currentState = COMPLETE;
        isComplete = true;
        return true;
    }
    
    // Otherwise, look for the end of trailers
    size_t endOfTrailers = data.find("\r\n\r\n", position);
    if (endOfTrailers == std::string::npos) {
        currentChunk += data.substr(position);
        position = data.size();
        return true;
    }
    
    position = endOfTrailers + 4;
    currentState = COMPLETE;
    isComplete = true;

    return true;
}

size_t RequestManager::hexToDecimal(const std::string& hex)
{
    std::stringstream ss;
    ss << std::hex << hex;
    size_t result;
    ss >> result;
    return result;
}

bool RequestManager::isRequestComplete() const 
{ 
    return isComplete; 
}

std::string RequestManager::getOutputBuffer() const
{
    return (outputBuffer.str());
}
