
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
#include <sys/fcntl.h>

class CGIManager
{
	private:
		int			childPid;
		int			clientSocketFD;
		int			pipeFD[2];
		int			postPathFD;
		bool		errorDetected;
		std::string	cgiResponse;

		std::string sizeTToString(size_t value);
		char**		setupEnvVars(HTTPRequest& request, ServerSettings& serverSettings);
		void		delete2DArray(char **arr);
	public:
		CGIManager(HTTPRequest& request, ServerSettings& serverSettings,
					EventManager *eventManager, int clientSocket,  const std::string &postPath = "");
		~CGIManager();
		
		void			handleCgiDirective(HTTPRequest& request, ServerSettings& serverSetings, 
											EventManager *eventManager, const std::string &postPath);
		std::string		generateCgiResponse();
		void			appendCgiResponse(std::string& cgiResponseSnippet);

		bool		getErrorDetected();
		int			getChildPid();
		int			getCgiFD();
		int			getCgiClientSocketFD();
		std::string	getCgiResponse();


		static bool	isFile(const std::string& requestURI);
		static bool	checkFileExtension(HTTPRequest& request, ServerSettings& serverSettings);
		static bool	isValidCGI(HTTPRequest& request, ServerSettings& serverSettings);
};

#endif