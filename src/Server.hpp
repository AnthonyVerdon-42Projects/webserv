#pragma once

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <csignal>
#include <exception>
#include <cstring>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <utility>
#include <sys/select.h>
#include <map>

#include "Request.hpp"

#define TIMEOUT_CGI 3
#define LISTEN_BACKLOG 128

class Server {
	public:
		Server();
		~Server();

		void start(void);
		void addAddress(const t_server& serverConfig);

		class ServerException : public std::exception {
			public:
				char const *what(void) const throw();
		};

	private:
		typedef std::map<int, t_server> socketMap;
		typedef std::map<int, Request> requestMap;

		socketMap	_sockets;
		requestMap	_requests;
		socklen_t	_addressLen;
		fd_set 		_readSet;
		fd_set 		_writeSet;
		t_server	_config;
		struct timeval _timeout;

		Server(Server const &other);
		Server &operator=(Server const &other);

		void _acceptConnection(int socketFd, const t_server& serverConfig);
		bool _processRequest(int clientFd, Request &request, fd_set* writeSet);
		void checkCGI(void);
};
