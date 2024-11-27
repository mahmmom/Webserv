#ifndef REQUEST_MANAGER_HPP
#define REQUEST_MANAGER_HPP

#include <string>
#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdlib>

class RequestManager {
	public:
		RequestManager(size_t maxBodySize);
		~RequestManager();

		bool processChunkedData(const std::string& chunk);
		bool isRequestComplete() const;
		bool hasExceededMaxSize() const;
		std::string getBuffer() const;

	private:
		enum ParseState {
			CHUNK_REQUEST_SIZE,
			CHUNK_DATA,
			CHUNK_EXTENSION,
			CHUNK_TRAILER
		};

		size_t maxBodySize;
		size_t currentBodySize;
		ParseState currentState;
		size_t currentChunkSize;
		std::string requestBuffer;
		std::string parseBuffer;
		bool requestComplete;
		bool exceededMaxSize;

		size_t parseChunkSize(const std::string& chunk);
		bool validateChunkData(const std::string& chunk);
		void trimWhitespace(std::string& str);
		bool isHexDigits(const std::string& str);
};

#endif // REQUEST_MANAGER_HPP
