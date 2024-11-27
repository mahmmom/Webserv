#include "RequestManager.hpp"

RequestManager::RequestManager(size_t maxBodySize) 
    : maxBodySize(maxBodySize), 
      currentBodySize(0), 
      currentState(CHUNK_REQUEST_SIZE), 
      currentChunkSize(0), 
      requestComplete(false),
      exceededMaxSize(false) {}

RequestManager::~RequestManager() {}

bool RequestManager::processChunkedData(const std::string& chunk) {
    parseBuffer += chunk;

    while (!parseBuffer.empty()) {
        switch (currentState) {
            case CHUNK_REQUEST_SIZE: {
                size_t crlfPos = parseBuffer.find("\r\n");
                if (crlfPos == std::string::npos) return true; // Wait for more data

                std::string chunkSizeStr = parseBuffer.substr(0, crlfPos);
                trimWhitespace(chunkSizeStr);

                // Check for valid hex digits
                if (!isHexDigits(chunkSizeStr)) {
                    return false; // Invalid chunk size
                }

                currentChunkSize = parseChunkSize(chunkSizeStr);

                // Check for 0-length chunk (end of chunked transfer)
                if (currentChunkSize == 0) {
                    currentState = CHUNK_TRAILER;
                    parseBuffer.erase(0, crlfPos + 2);
                    continue;
                }
    
                // Check if total body size would exceed limit
                if (currentBodySize + currentChunkSize > maxBodySize) {
                    exceededMaxSize = true;
                    return false; // Body too large
                }

                parseBuffer.erase(0, crlfPos + 2);
                currentState = CHUNK_DATA;
                break;
            }

            case CHUNK_DATA: {
                // Need at least chunk size + 2 bytes for CRLF
                if (parseBuffer.length() < currentChunkSize + 2) return true; // Wait for more data

                // Validate chunk data ends with CRLF
                if (!validateChunkData(parseBuffer)) return false;

                // Add chunk to request buffer
                requestBuffer += parseBuffer.substr(0, currentChunkSize);
                currentBodySize += currentChunkSize;

                // Remove chunk and CRLF
                parseBuffer.erase(0, currentChunkSize + 2);
                currentState = CHUNK_REQUEST_SIZE;
                break;
            }

            case CHUNK_TRAILER: {
                // Look for final CRLF to complete request
                if (parseBuffer.substr(0, 2) == "\r\n") {
                    requestComplete = true;
                    return true;
                }
                return true; // Wait for final CRLF
            }

            case CHUNK_EXTENSION: {
                // Ignore chunk extensions for simplicity
                currentState = CHUNK_DATA;
                break;
            }
        }
    }

    return true;
}

size_t RequestManager::parseChunkSize(const std::string& chunk) {
    char* endptr;
    return static_cast<size_t>(std::strtoul(chunk.c_str(), &endptr, 16));
}

bool RequestManager::validateChunkData(const std::string& chunk) {
    // Ensure chunk data is followed by CRLF
    return chunk.substr(currentChunkSize, 2) == "\r\n";
}

bool RequestManager::isRequestComplete() const {
    return requestComplete;
}

bool RequestManager::hasExceededMaxSize() const {
    return exceededMaxSize;
}

std::string RequestManager::getBuffer() const {
    return requestBuffer;
}

void RequestManager::trimWhitespace(std::string& str) {
    // Trim leading whitespace
    std::string::size_type nonWhiteStart = str.find_first_not_of(" \t");
    if (nonWhiteStart != std::string::npos) {
        str = str.substr(nonWhiteStart);
    } else {
        str.clear();
        return;
    }

    // Trim trailing whitespace
    std::string::size_type nonWhiteEnd = str.find_last_not_of(" \t");
    if (nonWhiteEnd != std::string::npos) {
        str = str.substr(0, nonWhiteEnd + 1);
    }
}

bool RequestManager::isHexDigits(const std::string& str) {
    for (std::string::size_type i = 0; i < str.length(); ++i) {
        if (!std::isxdigit(str[i])) {
            return false;
        }
    }
    return true;
}
