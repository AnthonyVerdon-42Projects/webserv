#include "Request.hpp"
#include "Server.hpp"
#include <ctime>
#include <sys/wait.h>

void Request::initializeEnvpCGI(void) {
	_cgiEnv["SERVER_SOFTWARE"] = "BOZOSERVER/2.0";
	_cgiEnv["SERVER_NAME"] = _serverConfig.server_name;
	_cgiEnv["SERVER_PORT"] = _serverConfig.port;
	_cgiEnv["SERVER_PROTOCOL"] = "HTTP/1.1";
	_cgiEnv["REQUEST_METHOD"] = _method;
	_cgiEnv["GATEWAY_INTERFACE"] = "CGI/1.1";
	_cgiEnv["CONTENT_TYPE"] = _requestHeader[CONTENT_TYPE];
	_cgiEnv["CONTENT_LENGTH"] = _requestHeader[CONTENT_LENGTH];
	_cgiEnv["REMOTE_ADDR"] = "0.0.0.0";
	_cgiEnv["PATH"] = _location->root;
	_cgiEnv["HTTP_USER_AGENT"] = _requestHeader[USER_AGENT];
}

static char **getEnvpInArray(std::map<std::string, std::string> _cgiEnv) {
	char **arrayEnvpVariable =  new char *[_cgiEnv.size() + 1];
	int i = 0;
	std::string *tmp;
	std::map<std::string, std::string>::iterator it;
	for (it = _cgiEnv.begin(); it != _cgiEnv.end(); ++it) {
		tmp = new std::string;
		*tmp = it->first + "=" + it->second + "\0";
		arrayEnvpVariable[i] = (char *)tmp->c_str();
		++i;
	}
	arrayEnvpVariable[i] = NULL;
	return (arrayEnvpVariable);
}

bool Request::requestCGI(void)
{
	std::string fileName = _requestHeader[HEAD];
	for (size_t i = 0; i < _location->executableCGI.size(); ++i)
	{
		if (fileName.length() > _location->extensionCGI[i].length() && fileName.substr(fileName.length() - _location->extensionCGI[i].length()) == _location->extensionCGI[i]) {
			_cgi.inCGI = true;
			executeCGI(fileName, (char *)_location->executableCGI[i].c_str());
			return (true);
		}
	}
	_cgi.inCGI = false;
	return (false);
}

void Server::checkCGI(void)
{
	for (requestMap::iterator it = _requests.begin(); it != _requests.end(); ++it)
	{
		if (it->second.getCGI().inCGI == false)
			continue ;
		std::ostringstream ss;
		bool stopCGI = false;
		if (std::time(NULL) - it->second.getCGI().begin_time > TIMEOUT_CGI)
		{
			stopCGI = true;
			if (kill(it->second.getCGI().pid, SIGKILL) < 0)
				throw (Request::RequestException(0));
			std::string InfiniteLoopHTML = INFINITE_LOOP_HTML;
			ss << "HTTP/1.1 508 Loop Detected\r\n";
			ss << "Content-type: text/html\r\n";
			ss << "Content-Length: " << InfiniteLoopHTML.size() << "\r\n\r\n";
			ss << InfiniteLoopHTML;
		}
		else if (waitpid(it->second.getCGI().pid, NULL, WNOHANG) > 0)
		{
			std::string statusCode = it->second.getStatusCode();
			stopCGI = true;
			char buffer[BUFFER_SIZE] = {0};
			int fileSize = read(it->second.getCGI().fds[1][0], buffer, BUFFER_SIZE);
			if (fileSize < 0)
				throw (Request::RequestException(fileSize));
			std::string responseCGI = buffer;
			if (responseCGI.empty() || responseCGI.find("\n\n") == std::string::npos || responseCGI.find("Content-Type: text/html") == std::string::npos)
			{
				std::ifstream errorFile("includes/defaultPages/502");
				std::ostringstream errorStream;
				statusCode = "502 Bad Gateway";
				errorStream << "Content-Type: text/html\n\n";
				errorStream << errorFile.rdbuf();
				responseCGI = errorStream.str();
			}
			std::string tmpHeader = responseCGI.substr(0, responseCGI.find("\n\n") + 1);
			std::string tmpBody = responseCGI.substr(responseCGI.find("\n\n") + 2, std::string::npos);
			size_t contentLength = tmpBody.length();
			ss << "HTTP/1.1 " << statusCode << "\r\n";
			ss << tmpHeader;
			ss << "Content-Length: " << contentLength << "\r\n\r\n";
			ss << tmpBody;
		}
		if (stopCGI)
		{
			if (close(it->second.getCGI().fds[1][0]) == -1)
				throw (Request::RequestException(0));
			it->second.getCGI().inCGI = false;
			if (send(it->second.getClientfd(), ss.str().c_str(), ss.str().size(), 0) < 0)
				throw (Request::RequestException(0));
			ss.str("");
			ss.clear();
		}
	}
}

void Request::executeCGI(const std::string& fileName, char *executableCGI) {
	initializeEnvpCGI();
	_cgiEnv["SCRIPT_NAME"] = fileName;
	_cgiEnv["URL"] = fileName;
	if (pipe(_cgi.fds[0]) == -1
		|| pipe(_cgi.fds[1]) == -1)
		throw (Request::RequestException(0));
	int pid = fork();
	if (pid == 0)
	{
		if (dup2(_cgi.fds[0][0], 0) == -1
			|| dup2(_cgi.fds[1][1], 1) == -1
			|| close(_cgi.fds[0][0]) == -1
			|| close(_cgi.fds[0][1]) == -1
			|| close(_cgi.fds[1][0]) == -1
			|| close(_cgi.fds[1][1]) == -1)
			throw (Request::RequestException(0));

		_cgiEnv["QUERY_STRING"] = _query;
		char *args[4] = {executableCGI, (char *)(fileName.c_str()),
						(char *)(_query.c_str()),NULL};
		char **EnvpVariables = getEnvpInArray(_cgiEnv);
		execve(executableCGI, args, EnvpVariables);
		for (size_t i = 0; EnvpVariables[i]; ++i)
			delete EnvpVariables[i];
		delete [] EnvpVariables;
		exit(EXIT_FAILURE);
	}
	else if (pid != 0)
	{
		if (close(_cgi.fds[0][0]) == -1
			|| close(_cgi.fds[0][1]) == -1
			|| close(_cgi.fds[1][1]) == -1)
			throw (Request::RequestException(0));
		if (pid > 0)
		{
			_cgi.inCGI = true;
			_cgi.pid = pid;
			_cgi.begin_time = std::time(NULL);
		}
		else if (pid < 0)
		{
			_cgi.inCGI = false;
			if (close(_cgi.fds[1][0]) == -1)
				throw (Request::RequestException(0));
		}
	}
}
