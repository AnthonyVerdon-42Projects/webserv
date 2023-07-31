#pragma once

#include <map>
#include <dirent.h>
#include <sys/socket.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "parsing/parsing.hpp"

#define BUFFER_SIZE 8192
#define DEBUG 1

#define INFINITE_LOOP_HTML "<html><body><h1>Infinite loop in CGI</h1></body></html>"
#define EXECUTION_CGI_FAILED_HTML "Content-type: text/html\n\n<html><body><h1>execution of CGI failed</h1></body></html>"
#define BAD_CGI_HEADER_HTML "Content-type: text/html\n\n<html><body><h1>bad CGI header</h1></body></html>"

void trimString(std::string& string, const char* charset);

typedef std::map<int, std::string> strMap;
typedef std::pair<int, std::string> strPair;

enum methods {
	GET = 0,
	POST,
	DELETE
};

enum headerAttributes {
	BODY,
	HEAD,
	HOST,
	USER_AGENT,
	ACCEPT,
	LOCATION,
	ACCEPT_LANGUAGE,
	ACCEPT_ENCODING,
	CONTENT_TYPE,
	BOUNDARY,
	CONTENT_LENGTH,
	ORIGIN,
	CONNECTION,
	REFERER,
	UPGRADE_INSECURE_REQUESTS,
	SEC_FETCH_DEST,
	SEC_FETCH_MODE,
	SEC_FETCH_SITE,
	SEC_FETCH_USER,
	PRAGMA,
	CACHE_CONTROL,
	TRANSFER_ENCODING
};

typedef struct s_cgi {
	bool	inCGI;
	int		pid;
	int		fds[2][2];
	time_t	begin_time;
} t_cgi;

class Request {
	public:
		Request(void);
		Request(int clientfd, const t_server& serverConfig);
		Request(const Request& other);
		Request& operator=(const Request& other);
		~Request(void);

		// Public member functions
		bool readRequest(std::string const& rawRequest);
		void respondToRequest(void);

		int getClientfd(void) const;
		const std::string& getStatusCode(void) const;
		t_cgi& getCGI(void);

		class RequestException : public std::exception {
			public:
				RequestException(int rv);
				char const *what(void) const throw();
				int rv(void) const throw();
			private:
				const int _rv;
		};

	// Private member functions
	private:
		void respondToGetRequest(void);
		void respondToPostRequest(void);
		void respondToDeleteRequest(void);
		void sendErrorResponse(void);
		void errorOnRequest(void);
		void processLine(const std::string& line, int lineToken);
		void parseHeader(const std::string& request);
		bool parseBody(const std::string& buffer);
		int setStatusCode(void);
		int getLineToken(const std::string& line);
		const std::string getMethod(std::string buffer);
		void directoryListing(DIR* directory, const std::string& dirName);
		void executeCGI(const std::string& fileName, char *executable);
		void initializeEnvpCGI(void);
		bool requestCGI(void);
		bool parseChunkedBody(const std::string& buffer);

	// Private member attributes
	private:
		int	_clientfd;
		std::string  _method;
		std::string _statusCode;
		std::string _boundary;
		std::string _query;
		std::string _chunkBuffer;
		strMap _requestHeader;
		std::map<std::string, std::string> _cgiEnv;
		t_cgi _cgi;
		t_server _serverConfig;
		t_location *_location;
		bool _isDirectory;
		bool _validRequest;
		bool _contentTooLarge;
		bool _chunkBufferFull;
};
