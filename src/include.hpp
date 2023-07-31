#ifndef INCLUDE_HPP_
#define INCLUDE_HPP_

#include <sstream>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cerrno>
#include <cstdlib>
#include <string>
#include <limits>
#include <map>

#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

typedef std::map<int, std::string> strMap;
typedef std::pair<int, std::string> strPair;

enum methods {
	ERROR,
	GET,
	POST,
	DELETE
};

enum attributes {
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
	CACHE_CONTROL
};

#define PORT 8080
#define IP "0.0.0.0"
#define IMG 1
#define HTML 2
#define END -1;

void trimString(std::string& string, const char* charset);

#endif
