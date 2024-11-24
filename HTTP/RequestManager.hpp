
#ifndef REQUEST_MANAGER_HPP
#define REQUEST_MANAGER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

class RequestManager {
	private:
		enum ChunkState {
			CHUNK_REQUEST_SIZE,
			CHUNK_DATA,
			CHUNK_ENDING,
			TRAILER,
			COMPLETE
		};

		ChunkState      	currentState;
		std::string     	currentChunk;
		size_t          	expectedChunkSize;
		std::ofstream*  	outputFile;  // Changed to pointer for C++98
		std::ostringstream	outputBuffer; // Mirror buffer for debugging
		bool            	isComplete;
		
		bool processChunkSize(const std::string& data, size_t& position);
		bool processChunkData(const std::string& data, size_t& position);
		bool processChunkEnding(const std::string& data, size_t& position);
		bool processTrailer(const std::string& data, size_t& position);
		size_t hexToDecimal(const std::string& hex);

	public:
		explicit RequestManager(std::ofstream* file); // Changed to take pointer
		~RequestManager();

		// Disable copy constructor and assignment operator
		RequestManager(const RequestManager& other);
		RequestManager& operator=(const RequestManager& other);

		bool processChunkedData(const std::string& data);
		bool isRequestComplete() const;

		std::string getOutputBuffer() const;
};

#endif