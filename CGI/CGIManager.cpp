
#include "CGIManager.hpp"
#include <sys/fcntl.h>

CGIManager::CGIManager(HTTPRequest& request, ServerSettings& serverSettings,
				EventManager *eventManager, int clientSocketFD,  const std::string &postPath)
				: clientSocketFD(clientSocketFD), postPathFD(-1), errorDetected(false)
{
	pipeFD[0] = -1;
	pipeFD[1] = -1;
	handleCgiDirective(request, serverSettings, eventManager, postPath);
}

CGIManager::~CGIManager()
{
	if (pipeFD[0] != -1)
	{
		close(pipeFD[0]);
		pipeFD[0] = -1;
	}
	if (postPathFD != -1)
	{
		close(postPathFD);
		postPathFD = -1;
	}
}

char		**CGIManager::setupEnvVars(HTTPRequest &request, ServerSettings &serverSettings)
{
	std::string					fullQuery;
	std::vector<std::string>	envVector;

	if (!request.getQueries().empty())
		envVector.insert(envVector.end(), request.getQueries().begin(), request.getQueries().end());
	envVector.push_back("CONTENT_TYPE=" + request.getHeader("content-type"));
	envVector.push_back("CONTENT_LENGTH=" + request.getHeader("content-length"));
	envVector.push_back("HTTP_COOKIE=" + request.getHeader("cookie"));
	for (size_t i = 0; i < request.getQueries().size(); i++) {
		if (i != 0)
			fullQuery += "&" + request.getQueries()[i];
		else
			fullQuery += request.getQueries()[i];
	}
	envVector.push_back("QUERY_STRING=" + fullQuery);
	envVector.push_back("HTTP_USER_AGENT= \"" + request.getHeader("user-agent") + "\"");
	envVector.push_back("REQUEST_METHOD=" + request.getMethod());
	envVector.push_back("SERVER_NAME=\"Racnhero\"");
	envVector.push_back("SERVER_PROTOCOL=" + request.getVersion());
	envVector.push_back("SCRIPT_FILENAME=" + serverSettings.getRoot() + request.getURI());
	envVector.push_back("SERVER_NAME=" + request.getHeader("host"));
	envVector.push_back("PATH=/opt/homebrew/bin:/opt/homebrew/sbin:/usr/local/bin:/System/Cryptexes/App/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/share/dotnet:~/.dotnet/tools:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/local/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/appleinternal/bin:/opt/homebrew/bin:/opt/homebrew/sbin");

	char	**envArray = new char *[envVector.size() + 1];
	size_t i = 0;
	while (i < envVector.size()) {
		envArray[i] = new char[envVector[i].length() + 1];
		strcpy(envArray[i], envVector[i].c_str());
		i++;
	}
	envArray[i] = NULL;
	return (envArray);
}

void CGIManager::handleCgiDirective(HTTPRequest& request, ServerSettings& serverSettings,
										EventManager *eventManager, const std::string &postPath)
{
	if (pipe(pipeFD) == -1) {
		Logger::log(Logger::ERROR,"Pipe creation failed", "CGIManager::handleCgiDirective");
		return (errorDetected = true, void());
	}

	char	**envp;
	envp = setupEnvVars(request, serverSettings);

	char	**argv;
	argv = new char *[2];
	argv[0] = new char[serverSettings.getRoot().length() + request.getURI().length() + 1];
	strcpy(argv[0], (serverSettings.getRoot() + request.getURI()).c_str());
	argv[1] = NULL;

	if (request.getMethod() == "POST") {
		if ((postPathFD = open(postPath.c_str(), O_RDONLY)) < 0)  {
				Logger::log(Logger::ERROR,"Opening path containing the POST method body failed", "CGIManager::handleCgiDirective");				
				errorDetected = true;
				return (delete2DArray(envp), delete2DArray(argv), void());
		}
	}

	if ((childPid = fork()) < 0) {
		Logger::log(Logger::ERROR,"Forking child process failed", "CGIManager::handleCgiDirective");
		errorDetected = true;
		return (delete2DArray(envp), delete2DArray(argv), void());
	}
	if (childPid == 0) {

		close(pipeFD[0]);
		dup2(pipeFD[1], STDOUT_FILENO);
		close(pipeFD[1]);

		if (request.getMethod() == "POST") {
			dup2(postPathFD, STDIN_FILENO);
			close(postPathFD);
		}

		if (execve(argv[0], argv, envp) < 0) {
			Logger::log(Logger::ERROR,"Forking child process failed", "CGIManager::handleCgiDirective");
			delete2DArray(envp), delete2DArray(argv);
			exit(EXIT_FAILURE);
		}
	}
	else {
		close(pipeFD[1]);
		int flags = fcntl(pipeFD[0], F_GETFL, NULL);
		if (flags < 0) {
			Logger::log(Logger::ERROR, "Failed to obtain pipe read end flags", "CGIManager::handleCgiDirective");
			return (errorDetected = true, void());
		}
		if (fcntl(pipeFD[0], F_SETFL, flags | O_NONBLOCK) < 0) {
			Logger::log(Logger::ERROR, "Failed to obtain pipe read end flags", "CGIManager::handleCgiDirective");
			return (errorDetected = true, void());
		}
	}
	eventManager->registerEvent(pipeFD[0], READ);
	delete2DArray(envp), delete2DArray(argv);
}

void CGIManager::appendCgiResponse(std::string& cgiResponseSnippet)
{
	cgiResponse += cgiResponseSnippet;
}

std::string CGIManager::generateCgiResponse()
{
	HTTPResponse response;

	if (cgiResponse.empty()) {

		response.buildDefaultErrorResponse("502", "Bad Gateway");
		return (response.generateResponse());
	}
	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");
	response.setBody(cgiResponse);
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Content-Type", "text/html");
	response.setHeaders("Content-Length", sizeTToString(cgiResponse.length()));
	response.setHeaders("Connection", "keep-alive");

	return (response.generateResponse());
}

bool CGIManager::isFile(const std::string& requestURI)
{
	struct stat pathStat;

	if (stat(requestURI.c_str(), &pathStat) != 0)
		return false;
	return (S_ISREG(pathStat.st_mode));
}

bool	CGIManager::checkFileExtension(HTTPRequest& request, ServerSettings& serverSettings)
{
	std::string fileExtension;

	size_t pos = request.getURI().find(".");
	if (pos == std::string::npos)
		return (false);
	fileExtension = request.getURI().substr(pos);

	const::std::vector<std::string>	extensions = serverSettings.getCgiDirective().getExtensions();
	for (size_t i = 0; i < extensions.size(); i++) {
		std::cout << "extend: " << fileExtension << " comapared with " << extensions[i] << std::endl;
		if (("*" + fileExtension) == extensions[i])
			return (true);
	}
	return (false);
}

bool CGIManager::isValidCGI(HTTPRequest& request, ServerSettings& serverSettings)
{
	std::cout << "here1" << std::endl;
	if (serverSettings.getRoot().find("cgi-bin/") == std::string::npos 
			&& (serverSettings.getRoot() + request.getURI()).find("cgi-bin/") == std::string::npos)
		return (false);
	std::cout << "here2" << std::endl;
	if (!isFile(serverSettings.getRoot() + request.getURI()))
		return (false);
	std::cout << "here3" << std::endl;
	if (!checkFileExtension(request, serverSettings))
		return (false);
	std::cout << "here4" << std::endl;
	return (true);
}

void CGIManager::delete2DArray(char **arr)
{
	int	i = 0;

	while (arr[i])
		delete (arr[i++]);
	delete(arr);
}

int	CGIManager::getChildPid()
{
	return (childPid);
}

bool CGIManager::getErrorDetected()
{
	return (errorDetected);
}

int	CGIManager::getCgiFD()
{
	return (pipeFD[0]);
}

int	CGIManager::getCgiClientSocketFD()
{
	return (clientSocketFD);
}

std::string	CGIManager::getCgiResponse()
{
	return (cgiResponse);
}

std::string CGIManager::sizeTToString(size_t value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}
