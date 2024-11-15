
#ifndef CGIMANAGER_CPP
# define CGIMANAGER_CPP

#include "../Logger/Logger.hpp"
#include "../HTTP/HTTPRequest.hpp"
#include "../Settings/ServerSettings.hpp"
#include "../Events/EventManager.hpp"
#include <unistd.h>
#include <vector>
#include <sys/stat.h>

class CGIManager
{
	private:
		int		childPid;
		int		pipeFD[2];
		int		postPathFD;
		bool	errorDetected;

		char	**setupEnvVars(HTTPRequest& request, ServerSettings& serverSettings);
		bool	isFile(const std::string& requestURI);
		void	delete2DArray(char **arr);
		bool	checkFileExtension(HTTPRequest& request, ServerSettings& serverSettings);
	public:
		CGIManager(HTTPRequest& request, ServerSettings& serverSettings,
					EventManager *eventManager, int clientSocket,  const std::string &postPath = "");
		
		void	handleCgiDirective(HTTPRequest& request, ServerSettings& serverSetings, 
									EventManager *eventManager, const std::string &postPath);
		bool	isValidCGI(HTTPRequest& request, ServerSettings& serverSettings);

		bool	getErrorDetected();
		int		getChildPid();
};

#endif