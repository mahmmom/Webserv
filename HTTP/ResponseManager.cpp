
#include "ResponseManager.hpp"

ResponseManager::ResponseManager(const std::string& compactResponse, bool closeConnection) : 
	compactResponse(compactResponse), closeConnection(closeConnection) 
{
	this->type = CompactResponse;
	this->bytesSent = 0;
}

ResponseManager::ResponseManager(const std::string& headers, const std::string& filePath, const long long& fileSize) :
	 fileSize(fileSize), filePath(filePath), headers(headers)
{
	(void) this->fileSize;
	fileStream.open(filePath.c_str(), std::ifstream::binary);
	this->type = ChunkedResponse;
	this->bytesSent = 0;
	this->headersFullySent = false;
}

#include <iostream>
std::string	ResponseManager::obtainChunk()
{
	if (fileStream.is_open())
	{
		if (bytesSent == 0)
		{
			char chunkBuffer[CHUNK_SIZE];
			fileStream.read(chunkBuffer, CHUNK_SIZE);
			size_t chunkBytesRead = fileStream.gcount();
			std::string chunkStr(chunkBuffer, chunkBytesRead);
			
			std::stringstream ss;
			ss << std::hex << chunkBytesRead << "\r\n";
			ss << chunkStr << "\r\n";
			chunk = ss.str();
			std::cout << chunk << std::endl;
			return (chunk);
		}
		else
			return (chunk);
	}
	return ("");
}

bool	ResponseManager::isFinished()
{
	if (type == ChunkedResponse)
	{
		if (fileStream.eof())
		{
			fileStream.close();
			return (true);
		}
	}
	else 
	{
		if (bytesSent >= compactResponse.size())
			return (true);
	}
	return (false);
}

void ResponseManager::setHeadersFullySent()
{
	this->headersFullySent = true;
}

void	ResponseManager::resetBytesSent()
{
	bytesSent = 0;
}

void	ResponseManager::updateBytesSent(size_t& bytesSent)
{
	this->bytesSent += bytesSent;
}


const bool& ResponseManager::getHeadersFullySent()
{
	return (this->headersFullySent);
}

const bool&	ResponseManager::getCloseConnection()
{
	return (this->closeConnection);
}

const size_t&	ResponseManager::getBytesSent()
{
	return (this->bytesSent);
}

const std::string&	ResponseManager::getCompactResponse()
{
	return (this->compactResponse);
}

const ResponseType&	ResponseManager::getType()
{
	return (this->type);
}

const std::string&	ResponseManager::getHeaders()
{
	return (this->headers);
}
