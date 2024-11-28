
#ifndef CGIMANAGER_CPP
# define CGIMANAGER_CPP

#include "../Logger/Logger.hpp"
#include "../HTTP/HTTPRequest.hpp"
#include "../HTTP/HTTPResponse.hpp"
#include "../Settings/ServerSettings.hpp"
#include "../Events/EventManager.hpp"
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <sys/fcntl.h>

# define CGI_TESTER_BUFFER_SIZE 65536 // 64 KB

class CGIManager
{
	private:
		int			childPid;
		int			clientSocketFD;
		int			pipeFD[2];
		int			postPathFD;
		bool		errorDetected;
		bool		testerMode;
		std::string	cgiResponse;
		std::time_t	cgiRequestTime;
		std::string	requestBody;
		std::string	testerBody;

		std::string sizeTToString(size_t value);
		char**		setupEnvVars(HTTPRequest& request, ServerSettings& serverSettings);
		void		delete2DArray(char **arr);
	public:
		CGIManager(HTTPRequest& request, ServerSettings& serverSettings,
					EventManager *eventManager, int clientSocket);
		~CGIManager();
		
		void			handleCgiDirective(HTTPRequest& request, ServerSettings& serverSetings, 
											EventManager *eventManager);

		std::string		generateCgiResponse();
		std::string		generateCgiTesterResponse();
		void			appendCgiResponse(std::string& cgiResponseSnippet);

		bool		getErrorDetected();
		int			getChildPid();
		int			getCgiFD();
		int			getCgiClientSocketFD();
		std::string	getCgiResponse();
		bool		getTesterMode();

		bool		isCgiTimedOut(size_t timeoutValue);
		static bool	isFile(const std::string& requestURI);
		static bool	checkFileExtension(HTTPRequest& request, ServerSettings& serverSettings);
		static bool	isValidCGI(HTTPRequest& request, ServerSettings& serverSettings);
};

#endif