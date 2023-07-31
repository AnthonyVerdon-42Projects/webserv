#include <algorithm>
#include "Request.hpp"
#include "Server.hpp"

Server::Server(void)
	: _addressLen(sizeof(sockaddr_in))
{
	FD_ZERO(&_readSet);
	FD_ZERO(&_writeSet);
	_timeout.tv_sec = 0;
	_timeout.tv_usec = 10000;
}

void Server::addAddress(const t_server& serverConfig)
{
	sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(serverConfig.port);
	socketAddress.sin_addr.s_addr = inet_addr(serverConfig.host.c_str());

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		throw (ServerException());

	int option = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) < 0)
		throw (ServerException());

	if (bind(fd, (sockaddr*)&socketAddress, _addressLen) < 0)
		throw (ServerException());

	if (listen(fd, LISTEN_BACKLOG) < 0)
		throw (ServerException());

	FD_SET(fd, &_readSet);
	FD_SET(fd, &_writeSet);
	_sockets[fd] = serverConfig;
	_sockets[fd].socketAddress = socketAddress;
}

Server::Server(Server const &other)
{
	(void)other;
}

Server &Server::operator=(Server const &other)
{
	(void)other;
	return *this;
}

Server::~Server()
{
	for (requestMap::iterator it = _requests.begin(); it != _requests.end(); )
	{
		if (it->second.getCGI().inCGI == true)
		{
			close(it->second.getCGI().fds[1][0]);
			kill(it->second.getCGI().pid, SIGKILL);
		}
		it++;
	}
	for (socketMap::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		close(it->first);
	for (requestMap::iterator it = _requests.begin(); it != _requests.end(); ++it)
		close(it->first);
}

void Server::start(void)
{
	fd_set readSet;
	fd_set writeSet;

	for (socketMap::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
		std::cout << "Listening on " << inet_ntoa(it->second.socketAddress.sin_addr) << ":" << ntohs(it->second.socketAddress.sin_port) << std::endl;
	while (true)
	{
		readSet = _readSet;
		writeSet = _writeSet;
		int rv = select(FD_SETSIZE + 1, &readSet, &writeSet, NULL, &_timeout);
		if (rv < 0)
			throw (Request::RequestException(0));
		checkCGI();
		if (rv == 0)
			continue ;
		for (socketMap::iterator it = _sockets.begin(); it != _sockets.end(); ++it)
			if (FD_ISSET(it->first, &readSet))
				_acceptConnection(it->first, it->second);

		for (requestMap::iterator it = _requests.begin(); it != _requests.end(); )
		{
			if (FD_ISSET(it->first, &readSet) && it->second.getCGI().inCGI == false)
			{
				if (_processRequest(it->first, it->second, &writeSet))
				{
					_requests.erase(it++);
					continue ;
				}
			}
			++it;
		}
	}
}

void Server::_acceptConnection(int socketFd, const t_server& serverConfig)
{
	int fd = accept(socketFd, (sockaddr *)&(serverConfig.socketAddress), &_addressLen);
	if (fd < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			throw (Request::RequestException(0));
		return ;
	}
	FD_SET(fd, &_readSet);
	FD_SET(fd, &_writeSet);
	_requests[fd] = Request(fd, serverConfig);
}

bool Server::_processRequest(int clientFd, Request &request, fd_set* writeSet)
{
	int rv;
	try {
		std::string header_buffer(BUFFER_SIZE, 0);
		rv = recv(clientFd, &header_buffer[0], BUFFER_SIZE, 0);
		if (rv <= 0)
			throw (Request::RequestException(rv));
		if (request.readRequest(header_buffer) && FD_ISSET(clientFd, writeSet))
			request.respondToRequest();
	}
	catch (const Request::RequestException & e) {
		close(clientFd);
		FD_CLR(clientFd, &_readSet);
		FD_CLR(clientFd, &_writeSet);
		if (e.rv() == -1)
			std::cerr << "error: " << strerror(errno) << "\n";
		return true;
	}
	return false;
}

char const *Server::ServerException::what(void) const throw() {
	return strerror(errno);
}
