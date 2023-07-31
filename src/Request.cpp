#include "Request.hpp"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

Request::Request(void)
	: _clientfd(-1), _method(""), _contentTooLarge(false)
{ _cgi.inCGI = false; }

Request::Request(int clientfd, const t_server& serverConfig)
	: _clientfd(clientfd), _method(""), _serverConfig(serverConfig), _contentTooLarge(false)
{ _cgi.inCGI = false; }

Request::Request(const Request& other)
	: _clientfd(other._clientfd), _method(other._method), _statusCode(other._statusCode),
		_boundary(other._boundary), _query(other._query), _chunkBuffer(other._chunkBuffer),
		_requestHeader(other._requestHeader), _cgiEnv(other._cgiEnv),
		_serverConfig(other._serverConfig),
		_location(other._location), _isDirectory(other._isDirectory),
		_validRequest(other._validRequest), _contentTooLarge(other._contentTooLarge),
		_chunkBufferFull(other._chunkBufferFull)
{  }

Request&	Request::operator=(const Request& other) {
	if (this != &other) {
		_clientfd = other._clientfd;
		_method = other._method;
		_statusCode = other._statusCode;
		_boundary = other._boundary;
		_query = other._query;
		_chunkBuffer = other._chunkBuffer;
		_requestHeader = other._requestHeader;
		_cgiEnv = other._cgiEnv;
		_cgi = other._cgi;
		_serverConfig = other._serverConfig;
		_location = other._location;
		_isDirectory = other._isDirectory;
		_validRequest = other._validRequest;
		_contentTooLarge = other._contentTooLarge;
		_chunkBufferFull = other._chunkBufferFull;
	}
	return *this;
}

void Request::sendErrorResponse(void) {
	std::ifstream file(_requestHeader[HEAD].c_str());

	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::ostringstream ss;
	ss << "HTTP/1.1 " << _statusCode << "\r\n";
	ss << "Content-type: text/html\r\n";
	ss << "Content-Length: " << fileSize << "\r\n\r\n";
	ss << file.rdbuf();
	int rv = send(_clientfd, ss.str().c_str(), ss.str().size(), 0);
	if (rv <= 0)
		throw (RequestException(rv));
}

void Request::respondToGetRequest(void) {
	DIR* directory = opendir(_requestHeader[HEAD].c_str());
	_isDirectory = false;
	_cgi.inCGI = false;
	if (directory == NULL) {
		if (requestCGI())
			return ;
		if (setStatusCode() == 400)
			return (sendErrorResponse());
		std::ifstream file(_requestHeader[HEAD].c_str(), std::ios::in | std::ios::binary);

		file.seekg(0, std::ios::end);
		std::streampos fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		std::ostringstream ss;
		ss << "HTTP/1.1 " << _statusCode << "\r\n";
		ss << "Content-type: " << _requestHeader[ACCEPT] << "\r\n";
		ss << "Content-Length: " << fileSize << "\r\n\r\n";
		ss << file.rdbuf();

		int rv = send(_clientfd, ss.str().c_str(), ss.str().size(), 0);
		if (rv <= 0)
			throw (RequestException(rv));
		ss.str("");
		ss.clear();
		file.close();
	}
	else {
		_isDirectory = true;
		if (setStatusCode() == 400)
			return (sendErrorResponse());
		directoryListing(directory, _requestHeader[HEAD]);
	}
}

void Request::respondToPostRequest(void) {
	int statusCode = setStatusCode();
	if (statusCode == 400)
		return (sendErrorResponse());

	_cgi.inCGI = false;
	if (requestCGI())
		return ;
	std::ostringstream ss;

	_requestHeader[LOCATION] = "/";
	ss << "HTTP/1.1 " << _statusCode << "\r\n";
	ss << "Content-type: " << _requestHeader[ACCEPT] << "\r\n";
	if (_statusCode == "302 Redirect")
		ss << "Location: " << _requestHeader[LOCATION] << "\r\n";
	ss << "Content-length: 0" << "\r\n\r\n";
	int rv = send(_clientfd, ss.str().c_str(), ss.str().size(), 0);
	if (rv <= 0)
		throw (RequestException(rv));

	if (statusCode == 200) {
		std::ofstream file((_location->root + _location->uploadedFilePath + _requestHeader[HEAD]).c_str());
		file << _requestHeader[BODY];
		file.close();
	}
}

void Request::respondToDeleteRequest(void) {
	setStatusCode();
	std::string fileName = _requestHeader[HEAD];
	std::cout << fileName << std::endl;
	std::remove(fileName.c_str());
	std::ostringstream ss;
	ss << "HTTP/1.1 " << _statusCode << "\r\n";
	ss << "Content-type: " << _requestHeader[ACCEPT] + "\r\n";
	ss << "Content-Length: 0\r\n\r\n";
	int rv = send(_clientfd, ss.str().c_str(), ss.str().size(), 0);
	if (rv <= 0)
		throw (RequestException(rv));
}

const std::string Request::getMethod(std::string buffer) {
	if (buffer.find("GET") != std::string::npos)
		return ("GET");
	else if (buffer.find("POST") != std::string::npos)
		return ("POST");
	else if (buffer.find("DELETE") != std::string::npos)
		return ("DELETE");
	return ("FORBIDDEN");
}

bool Request::readRequest(std::string const &rawRequest) {
	static bool headerRead = false;
	if (headerRead == false) {
		_method = getMethod(rawRequest);
		if (_method != "FORBIDDEN")
			parseHeader(rawRequest);
		if (_method == "FORBIDDEN" ||
			(_method == "POST" && _boundary.empty())) {
			headerRead = false;
			return (true);
		}
		else if (_method == "POST" && (static_cast<std::size_t>(std::atoll(_requestHeader[CONTENT_LENGTH].c_str())) > _serverConfig.maxFileSizeUpload)) {
			headerRead = false;
			_contentTooLarge = true;
			return (true);
		}
		std::string tmpHeader = rawRequest.substr(0, rawRequest.find("\r\n\r\n"));
		std::string tmpBodyHeader = rawRequest.substr(rawRequest.find("\r\n\r\n") + 4, std::string::npos);
		tmpBodyHeader = tmpBodyHeader.substr(0, tmpBodyHeader.find("\r\n\r\n"));
		std::string tmpBoundary = tmpBodyHeader.substr(0, tmpBodyHeader.find("\r"));
		if (!(_requestHeader[TRANSFER_ENCODING] == "chunked" && _method == "POST")) {
			long contentLength = std::atol(_requestHeader[CONTENT_LENGTH].c_str()) + tmpHeader.length() + 4 + tmpBodyHeader.length() + tmpBoundary.length() + 2;
			if (_method == "GET" || _method == "DELETE" || (_method == "POST" && contentLength < BUFFER_SIZE)) {
				headerRead = false;
				return (true);
			}
		}
		headerRead = true;
	}
	else {
		if (parseBody(rawRequest) == true) {
			headerRead = false;
			return (true);
		}
	}
	return (false);
}

Request::~Request(void) {
}

int Request::getClientfd(void) const {
	return (_clientfd);
}

t_cgi& Request::getCGI(void) {
	return (_cgi);
}

const std::string& Request::getStatusCode(void) const {
	return (_statusCode);
}

Request::RequestException::RequestException(int rv): _rv(rv){}

char const *Request::RequestException::what(void) const throw() {
	return strerror(errno);
}

int Request::RequestException::rv(void) const throw() {
	return (_rv);
}
