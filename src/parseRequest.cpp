#include "Request.hpp"
#include <vector>
#include <algorithm>
#include <string>

/**
* This function will return a specific part of str
*
* @param sep The delimiter character
* @param str The reference string containing elements split with sep char
* @param pos The element index you want to get
* @return token The split element corresponding to the index
*
* Exemple: 
* getToken("This is a string", ' ', 2);
* will return "is", as it is the 2nd part of the string 
* with the delimiter ' ' (space)
*/
static std::string getToken(const std::string& str, char sep, int pos){
	std::string token;
	int cur_pos;

	cur_pos = 1;
	for (uint64_t i = 0; i < str.size(); ++i) {
		if (str[i] == sep)	{
			if (cur_pos == pos)
				break;
			token.clear();
			cur_pos++;
		}
		else
			token += str[i];
	}
	return (token);
}

/**
* This function will find the extension of the file
*
* @param fileName The file name
* @return extension The extension according to the file name
* 					default return is "text/html"
*
* Exemple: 
* getToken("img.jpeg");
* will return "jpeg", as it is the type of the file 
*/
static std::string getExtension(const std::string& fileName) {
	std::string extension;
	size_t dot = fileName.find(".");
	if (dot != std::string::npos) {
		extension = fileName.substr(dot + 1, fileName.find(",", dot));
	}
	else {
		extension = "text/html";
	}
	return (extension);
}

/**
* This function will get the specific parts of the attributes
* in the HTTP header
*
* It will add the specific value to a std::map<int, std::string> named
* _requestHeader at the right index, member attribute of the Request class
*
* @param line The line from the HTTP header, with the value of the attribute
* @param lineToken The token corresponding to the attribute of the current line
*
* Exemple: 
* getToken("Content-Type: text/html", CONTENT_TYPE);
* will add "text/html" to _requestHeader[CONTENT_TYPE]
*/
void Request::processLine(const std::string& line, int lineToken) {
	std::string str = getToken(line, ' ', 2);
	int pos = str.find('\r');
	std::string root = _location->root;
	std::string index = _location->index;
	if (pos > 0)
		str.erase(pos, 1);
	switch (lineToken) {
		case HEAD:
			{
				str.erase(0, 1);
				std::string rawRoot = root;
				rawRoot.erase(rawRoot.length() - 1, 1);
				if (line.find("HTTP/1.1") != std::string::npos)
					_validRequest = true;
				else
					_validRequest = false;
				if (str.find(rawRoot) == std::string::npos) {
					if (str.length() == 0)
						_requestHeader[HEAD] = root + index;
					else
						_requestHeader[HEAD] = root + str;
				}
				else
					_requestHeader[HEAD] = str;
			}
			break;
		case BOUNDARY:
			{
				std::string tmp = getToken(line, ' ', 3);
				_boundary = tmp.substr(tmp.find("=") + 1, tmp.length());
				trimString(_boundary, "-\r");
			}
			break;
		case ACCEPT:
			str = getExtension(_requestHeader[HEAD]);
			break;
		default:
			break;
	}
	if (lineToken != HEAD && lineToken != BOUNDARY)
		_requestHeader.insert(strPair(lineToken, str));
}

/**
* This function will find the location block depending on
* the location path in the HTTP header
*
* @param path The path of the requested file
* @param locations All the location blocks stored in the current server block (conf file)
* @return t_location* The address of the right location block
*/
static t_location* getLocation(const std::string& path, std::vector<t_location>& locations) {
	for (std::vector<t_location>::iterator it = locations.begin(); it != locations.end(); ++it) {
		if (path == it->locationPath)
			return (&(*it));
	}
	return (&(*(locations.begin())));
}

/**
* This function will return the token regarding to the attribute
* contained in the HTTP header
*
* @param line The line from the HTTP header, with the value of the attribute
* @return token The corresponding token
*/
int Request::getLineToken(const std::string& line) {
	if (line.find("POST") != std::string::npos || line.find("GET") != std::string::npos || line.find("DELETE") != std::string::npos) {
		std::string path = getToken(line, ' ', 2);
		if (path[0] == '/')
			path.erase(0, 1);
		size_t pos = path.rfind("/", path.length() - 1);
		if (pos != std::string::npos)
			path = path.substr(0, pos + 1);
		else if (closedir(opendir(_requestHeader[HEAD].c_str())) == -1)
			path = _requestHeader[HEAD] + "/";
		else
			path = "/";
		_location = getLocation(path, _serverConfig.locations);
		return (HEAD);
	}
	else if (line.find("Host:") != std::string::npos)
		return (HOST);
	else if (line.find("User-Agent:") != std::string::npos)
		return (USER_AGENT);
	else if (line.find("Accept:") != std::string::npos)
		return (ACCEPT);
	else if (line.find("Accept-Language:") != std::string::npos)
		return (ACCEPT_LANGUAGE);
	else if (line.find("Accept-Encoding:") != std::string::npos)
		return (ACCEPT_ENCODING);
	else if (line.find("Content-Type:") != std::string::npos) {
		if (line.find("boundary") != std::string::npos)
			return (BOUNDARY);
		return (CONTENT_TYPE);
	}
	else if (line.find("Content-Length:") != std::string::npos)
		return (CONTENT_LENGTH);
	else if (line.find("Transfer-Encoding:") != std::string::npos)
		return (TRANSFER_ENCODING);
	else
		return (-2);
}

/**
* This function will process the body of the HTTP request
*
* It will separate the body with the boundaries to keep
* the whole content of the file but it will also get the CONTENT_TYPE
* contained in the header of the specific boundary
*
* @param boundary The boundary from the HTTP header, delimiter of the content
* @param line The whole body after the HTTP header 
* @param requestHeader The map containing the header attributes values
* @param query The query string (member of Request) used for CGI
*/
static void processBody(std::string& boundary, const std::string& line, strMap& requestHeader, std::string& query) {
	int i = 1;
	std::string str = getToken(line, '\n', i);
	trimString(str, "-\r");
	if (!boundary.empty() && str == boundary) {
		++i;
		while (i < 4) {
			str = getToken(line, '\n', i);
			if (i == 2) {
				std::string tmp = getToken(str, '\"', 4);
				requestHeader[HEAD] = tmp;
			}
			else if (i == 3) {
				std::string tmp = getToken(str, ' ', 2);
				requestHeader[CONTENT_TYPE] = tmp;
			}
			++i;
		}
		std::string tmp = line.substr(line.find("\r\n\r\n"), line.length());
		tmp.erase(0, 4);
		tmp.erase(tmp.end() - 1);
		size_t pos = tmp.rfind("\n------------------");
		tmp.erase(pos, pos - tmp.length());
		tmp.erase(tmp.end() - 1);
		requestHeader[BODY].clear();
		requestHeader[BODY] = tmp;
	}
	else {
		query = line;
	}
}

/**
* This function will parse the first buffer from the client,
* containing the header and in some cases a body
*
* @param buffer The buffer to parse
*/
void Request::parseHeader(const std::string& buffer) {
	size_t i = 0;
	int lineToken;
	std::string line;

	while (buffer[i])
	{
		while (buffer[i] && buffer[i] != '\n')
		{
			line += buffer[i];
			++i;
		}
		if (line == "\r")
		{
			break ;
		}
		lineToken = getLineToken(line);
		processLine(line, lineToken);
		line.clear();
		++i;
	}
	line.clear();
	size_t pos = buffer.find("\r\n\r\n");
	line = buffer.substr(pos + 4, std::string::npos);
	_requestHeader[BODY] = line;
}

/**
* This function will parse a body sent by the client
* using a chunked method
*
* @param buffer The buffer to add to the body content
* @return True if the whole body has been received
*/
bool Request::parseChunkedBody(const std::string& buffer) {
	if (!_chunkBufferFull)
	{
		_chunkBuffer += buffer;
		if (buffer.find("\r\n0\r\n", 0) != std::string::npos)
			_chunkBufferFull = true;
		else
			return (false);
	}

	std::size_t pos = 0;
	while (true) {
		std::size_t limPos = _chunkBuffer.find("\r\n", pos);
		long size = std::strtol(_chunkBuffer.substr(pos, limPos - pos).c_str(), NULL, 16);
		if (size == 0) {
			_chunkBuffer.clear();
			_chunkBufferFull = false;
			return (true);
		}
		_requestHeader[BODY] += _chunkBuffer.substr(limPos + 2, size);
		pos = limPos + 2 + size + 2;
	}
}

/**
* This function will parse a body sent by the client
*
* @param buffer The buffer to add to the body content
* @return True if the whole body has been received
*/
bool Request::parseBody(const std::string& buffer) {
	if (_requestHeader[TRANSFER_ENCODING] == "chunked")
		return (parseChunkedBody(buffer));

	_requestHeader[BODY] += buffer;
	size_t pos = buffer.find(_boundary);
	if (pos != std::string::npos && pos > _boundary.length() + 30)
		return (true);
	return (false);
}

/**
* This function will parse the query string contained
* in some GET method headers
*
* It will split the string depending on the '?' character
*
* @param query A reference to the query value string attribute in Request
* @param name A reference to the file name attribute in Request
*
* Exemple: 
* getQuery(query, name);
* with query == "/cgi-bin/hello.py?name=hi"
* will be query == "name=hi" and name == "/cgi-bin/hello.py"
*/
static void getQuery(std::string& query, std::string& name) {
	size_t delimiter = name.find("?");
	if (delimiter != std::string::npos) {
		query = name.substr(delimiter + 1);
		name = name.substr(0, delimiter);
	}
}

/**
* This function will replace the %HEX_CODE by the corresponding
* char (cf. ascii table) in the file name of the request
*
* @param name A reference to the file name attribute in Request
*
* Exemple: 
* editName(name);
* with name == "/uploads/My%20picture" will be name == "My picture"
*/
static void editName(std::string& name) {
	std::string subName = name;
	while (true) {
		size_t pos = subName.find("%");
		if (pos != std::string::npos) {
			std::string tmp = subName.substr(pos + 1, std::string::npos);
			char* name_c = (char *)tmp.c_str();
			name_c[2] = '\0';
			long c = std::strtol(name_c, NULL, 16);
			name.replace(name.find("%"), 3, (const char*)&c);
			subName = tmp;
		}
		else if (name.find("+") != std::string::npos) {
			name.replace(name.find("+"), 1, " ");
		}
		else
			break ;
	}
}

/**
* This function will respond to the request with the matching
* function (GET, POST, DELETE or FORBIDDEN)
*/
void Request::respondToRequest(void) {
	editName(_requestHeader[HEAD]);
	getQuery(_query, _requestHeader[HEAD]);
	if (_method == "GET")
		respondToGetRequest();
	else if (_method == "POST") {
		if (_contentTooLarge == false)
			processBody(_boundary, _requestHeader[BODY], _requestHeader, _query);
		respondToPostRequest();
	}
	else if (_method == "DELETE")
		respondToDeleteRequest();
	else if (_method == "FORBIDDEN") {
		_requestHeader[HEAD] = "includes/defaultPages/400";
		_statusCode = "400 Bad Request";
		sendErrorResponse();
	}
	_contentTooLarge = false;
	_requestHeader.clear();
	_boundary.clear();
	_statusCode.clear();
}
