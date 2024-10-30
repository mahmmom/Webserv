
#ifndef RESPONSEMANAGER_HPP
# define RESPONSEMANAGER_HPP

#include "HTTPResponse.hpp"
#include <fstream>
#include <string>
#include <sstream>
#include "../Logger/Logger.hpp"
#include <vector>

#define CHUNK_SIZE 8192 // 8 Kb
// #define CHUNK_SIZE 65536 // 64 Kb

class ResponseManager
{
	private:
		ResponseType 	type;
		std::ifstream	fileStream;
		std::string		compactResponse;
		long long		fileSize;
		std::string		filePath;
		std::string		headers;
		size_t			bytesSent;
		bool			closeConnection;
		bool			headersFullySent;
		std::string		chunk;

	public:
		ResponseManager(const std::string& compactResponse, bool closeConnection);
		ResponseManager(const std::string& headers, const std::string& filePath, const long long& fileSize);

		bool				isFinished();
		std::string			obtainChunk();

		void				setHeadersFullySent();
		void				resetBytesSent();
		void				updateBytesSent(size_t& bytesSent);

		const bool&			getHeadersFullySent();
		const bool&			getCloseConnection();
		const size_t&		getBytesSent();
		const std::string&	getCompactResponse();
		const ResponseType& getType();
		const std::string&	getHeaders();
};

#endif