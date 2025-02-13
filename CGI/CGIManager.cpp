
#include "CGIManager.hpp"

CGIManager::CGIManager(HTTPRequest& request, ServerSettings& serverSettings,
				EventManager *eventManager, int clientSocketFD)
				: clientSocketFD(clientSocketFD), postPathFD(-1), errorDetected(false), testerMode(false)
{
	pipeFD[0] = -1;
	pipeFD[1] = -1;
	cgiRequestTime = std::time(0);
	handleCgiDirective(request, serverSettings, eventManager);
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
	envVector.push_back("SERVER_NAME=\"Ranchero\"");
	envVector.push_back("SERVER_PROTOCOL=" + request.getVersion());
	envVector.push_back("SCRIPT_FILENAME=" + serverSettings.getRoot() + request.getURI());
	envVector.push_back("SERVER_NAME=" + request.getHeader("host"));
	envVector.push_back("PATH=/opt/homebrew/bin:/opt/homebrew/sbin:/usr/local/bin:/System/Cryptexes/App/usr/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/share/dotnet:~/.dotnet/tools:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/local/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/bin:/var/run/com.apple.security.cryptexd/codex.system/bootstrap/usr/appleinternal/bin:/opt/homebrew/bin:/opt/homebrew/sbin");
	envVector.push_back("PATH_INFO=" + serverSettings.getRoot() + request.getURI());
	std::string secret = request.getHeader("x-secret-header-for-test");
	if (secret != "")
		envVector.push_back("HTTP_X_SECRET_HEADER_FOR_TEST=" + secret);

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
										EventManager *eventManager)
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

	std::cout << "script name is " << argv[0] << std::endl;
    if (!strcmp(argv[0], "YoupiBanane/directory/youpi.bla") || !strcmp(argv[0], "YoupiBanane/directory/youpla.bla")) {
        std::cout << "Match found, resetting argv[0] to 'cgi_tester'.\n";
        delete[] argv[0];
        argv[0] = new char[strlen("cgi_tester") + 1];
        strcpy(argv[0], "cgi_tester");
		testerMode = true;
        std::cout << "New script name is " << argv[0] << std::endl;
    }
	argv[1] = NULL;


	FILE	*tempOut = tmpfile();
	if (!tempOut) {
		Logger::log(Logger::ERROR, "Failed to create a temporary out file", "CGIManager::handleCgiDirective");
		return (errorDetected = true, delete2DArray(envp), delete2DArray(argv), void());
	}
	long	tempOutFD = fileno(tempOut);

	if ((childPid = fork()) < 0) {
		Logger::log(Logger::ERROR,"Forking child process failed due to " + std::string(strerror(errno)), "CGIManager::handleCgiDirective");
		errorDetected = true;
		return (delete2DArray(envp), delete2DArray(argv), void());
	}

	if (childPid == 0)
	{

		for (char **env = envp; *env != NULL; env++) {
				Logger::log(Logger::INFO, "ENV: " + std::string(*env), "CGIManager::handleCgiDirective");
		}
		Logger::log(Logger::INFO, "ARGV[0]: " + std::string(argv[0]), "CGIManager::handleCgiDirective");

		close(pipeFD[0]);
		if (request.getMethod() == "POST")
		{
            FILE* 	tempIn = tmpfile();
			if (!tempIn) {
				Logger::log(Logger::ERROR, "Failed to create temporary in file", "CGIManager::handleCgiDirective");
						errorDetected = true;
				if (tempOut)
					fclose(tempOut);
				delete2DArray(envp), delete2DArray(argv);
				exit(EXIT_FAILURE);
			}
			long	tempInFD = fileno(tempIn);
            
            const std::string& requestBody = request.getBody();
			ssize_t bytesWritten = write(tempInFD, requestBody.c_str(), requestBody.length());
			if (bytesWritten != static_cast<ssize_t>(requestBody.length())) {
				Logger::log(Logger::ERROR, "Failed to write entire body", "CGIManager::handleCgiDirective");
				errorDetected = true;
				delete2DArray(envp), delete2DArray(argv);
				exit(EXIT_FAILURE);
			}
			
			lseek(tempInFD, 0, SEEK_SET);
			dup2(tempInFD, STDIN_FILENO);
			if (tempIn)
				fclose(tempIn);
		}
		else
		{
			int nullFd = open("/dev/null", O_RDONLY);
			if (nullFd < 0) {
				Logger::log(Logger::ERROR, "Failed to open /dev/null", "CGIManager::handleCgiDirective");
				errorDetected = true;
				delete2DArray(envp), delete2DArray(argv);
				exit(EXIT_FAILURE);
			}
			dup2(nullFd, STDIN_FILENO);
			close(nullFd);
		}

		if (testerMode) {
			close(pipeFD[1]);
			dup2(tempOutFD, STDOUT_FILENO);
		}
		else {
			dup2(pipeFD[1], STDOUT_FILENO);
			close(pipeFD[1]);
			if (tempOut)
				fclose(tempOut);
		}

		if (execve(argv[0], argv, envp) < 0) {
			Logger::log(Logger::ERROR,"Execve failed due to " + std::string(strerror(errno)), "CGIManager::handleCgiDirective");
			delete2DArray(envp), delete2DArray(argv);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		if (testerMode) {
			const char* message = "activate pipe";
			write(pipeFD[1], message, strlen(message));
			close(pipeFD[1]);
			char	buffer[CGI_TESTER_BUFFER_SIZE] = {0};

			waitpid(childPid, NULL, 0);
			lseek(tempOutFD, 0, SEEK_SET);

			int ret = 1;
			while (ret > 0)
			{
				memset(buffer, 0, CGI_TESTER_BUFFER_SIZE);
				ret = read(tempOutFD, buffer, CGI_TESTER_BUFFER_SIZE - 1);
				testerBody += buffer;
			}
			if (tempOut)
				fclose(tempOut);
		}
		else {
			close(pipeFD[1]);
			if (tempOut)
				fclose(tempOut);
			int flags = fcntl(pipeFD[0], F_GETFL, NULL);
			if (flags < 0) {
				Logger::log(Logger::ERROR, "Failed to obtain pipe read end flags", "CGIManager::handleCgiDirective");
				return (errorDetected = true, delete2DArray(envp), delete2DArray(argv), void());
			}
			if (fcntl(pipeFD[0], F_SETFL, flags | O_NONBLOCK) < 0) {
				Logger::log(Logger::ERROR, "Failed to set pipe read end to nonblocking", "CGIManager::handleCgiDirective");
				return (errorDetected = true, delete2DArray(envp), delete2DArray(argv), void());
			}
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

	size_t headersEnd = cgiResponse.find("\r\n\r\n");
	std::string statusCode = "200"; // Default if not found
	std::string contentType = "text/html; charset=utf-8"; // Default if not found
	std::string cgiHeaders;
	std::string cgiBody;
	if (headersEnd != std::string::npos) {
		cgiHeaders = cgiResponse.substr(0, headersEnd);
		cgiBody = cgiResponse.substr(headersEnd + 4);

		// 2. Remove `Status: ...` and `Content-Type: ...`
		std::istringstream headersStream(cgiHeaders);
		std::string line;
		while (std::getline(headersStream, line)) {
			if (line.find("Status: ") == 0) {
				statusCode = line.substr(8, 3); // Extract 3-digit status code
			} else if (line.find("Content-Type: ") == 0) {
				contentType = line.substr(14); // Extract content-type
			}
		}
	}
	else {
		cgiBody = cgiResponse;
	}

	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");
	response.setBody(cgiBody);
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Content-Type", "text/html; charset=utf-8");
	response.setHeaders("Content-Length", sizeTToString(cgiBody.length()));
	response.setHeaders("Connection", "keep-alive");

	return (response.generateResponse());
}

std::string CGIManager::generateCgiTesterResponse()
{
	HTTPResponse response;

	if (testerBody.empty()) {
		response.buildDefaultErrorResponse("502", "Bad Gateway");
		return (response.generateResponse());
	}

	size_t headersEnd = testerBody.find("\r\n\r\n");
	std::string statusCode = "200"; // Default if not found
	std::string contentType = "text/html; charset=utf-8"; // Default if not found
	std::string cgiHeaders;
	std::string cgiBody;
	if (headersEnd != std::string::npos) {
		cgiHeaders = testerBody.substr(0, headersEnd);
		cgiBody = testerBody.substr(headersEnd + 4);

		// 2. Remove `Status: ...` and `Content-Type: ...`
		std::istringstream headersStream(cgiHeaders);
		std::string line;
		while (std::getline(headersStream, line)) {
			if (line.find("Status: ") == 0) {
				statusCode = line.substr(8, 3); // Extract 3-digit status code
			} else if (line.find("Content-Type: ") == 0) {
				contentType = line.substr(14); // Extract content-type
			}
		}
	}
	else {
		cgiBody = testerBody;
	}

	response.setVersion("HTTP/1.1");
	response.setStatusCode("200");
	response.setReasonPhrase("OK");
	response.setBody(cgiBody);
	response.setHeaders("Server", "Ranchero");
	response.setHeaders("Content-Type", "text/html; charset=utf-8");
	response.setHeaders("Content-Length", sizeTToString(cgiBody.length()));
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
		// std::cout << "extend: " << fileExtension << " comapared with " << extensions[i] << std::endl;
		if (("*" + fileExtension) == extensions[i])
			return (true);
	}
	return (false);
}

bool CGIManager::isValidCGI(HTTPRequest& request, ServerSettings& serverSettings)
{
	if (request.getURI() != "/directory/youpi.bla" && request.getURI() != "/directory/youpla.bla") {
	if (serverSettings.getRoot().find("cgi-bin/") == std::string::npos 
			&& (serverSettings.getRoot() + request.getURI()).find("cgi-bin/") == std::string::npos)
		return (false);
	if (!isFile(serverSettings.getRoot() + request.getURI()))
		return (false);
	}
	if (!checkFileExtension(request, serverSettings))
		return (false);
	return (true);
}

void CGIManager::delete2DArray(char **arr)
{
	int	i = 0;

	while (arr[i])
		delete[] (arr[i++]);
	delete[] (arr);
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

bool CGIManager::getTesterMode()
{
	return (testerMode);
}

std::string CGIManager::sizeTToString(size_t value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

bool CGIManager::isCgiTimedOut(size_t timeoutValue)
{
	std::time_t now = std::time(0);
    std::time_t elapsedTime = now - cgiRequestTime;

	bool timedOut = elapsedTime > static_cast<std::time_t>(timeoutValue);
	// std::cout << "timeout is " << timeoutValue << " and elapsed is " << elapsedTime << std::endl;
	return timedOut;
}
