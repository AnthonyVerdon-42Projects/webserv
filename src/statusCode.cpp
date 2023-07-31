#include "Request.hpp"

static std::string handleDirectoryCode(strMap& _requestHeader) {
	std::string dirName = _requestHeader[HEAD];

	if (dirName[dirName.length() - 1] != '/') {
		_requestHeader[LOCATION] = "/" + dirName + "/";
		return ("308 Permanent Redirect");
	}
	else {
		_requestHeader[LOCATION] = "";
		return ("200 OK");
	}
}

int Request::setStatusCode(void) {
	std::ifstream file;
	
	file.open(_requestHeader[HEAD].c_str());
	if (_method == "GET" && _location->methodsAllowed[GET] == false) {
		_statusCode = "405 Method Not Allowed";
		_requestHeader[HEAD] = "includes/defaultPages/405";
		return (400);
	}
	else if (_method == "POST" && _location->methodsAllowed[POST] == false) {
		_statusCode = "405 Method Not Allowed";
		_requestHeader[HEAD] = "includes/defaultPages/405";
		return (400);
	}
	else if (_method == "DELETE" && _location->methodsAllowed[DELETE] == false) {
		_statusCode = "405 Method Not Allowed";
		_requestHeader[HEAD] = "includes/defaultPages/405";
		return (400);
	}
	else if (_method == "POST" && static_cast<std::size_t>(std::atoll(_requestHeader[CONTENT_LENGTH].c_str())) > _serverConfig.maxFileSizeUpload) {
		_statusCode = "413 Content Too Large";
		_requestHeader[HEAD] = "includes/defaultPages/413";
		return (400);
	}
	else if (_method == "POST" && _location->acceptUploadedFile == false) {
		_statusCode = "403 Forbidden";
		_requestHeader[HEAD] = "includes/defaultPages/403";
		return (400);
	}
	else if (!file && _method != "POST") {
		_statusCode = "404 Not Found";
		_requestHeader[HEAD] = _serverConfig.errpage;
		return (400);
	}
	else if (_method == "GET" && _isDirectory == true) {
		_statusCode = handleDirectoryCode(_requestHeader);
		if (_location->directoryListing == false) {
			_statusCode = "403 Forbidden";
			_requestHeader[HEAD] = "includes/defaultPages/403";
			return (400);
		}
	}
	else if (_validRequest == false || _method == "FORBIDDEN") {
		_statusCode = "400 Bad Request";
		_requestHeader[HEAD] = "includes/defaultPages/400";
		_validRequest = true;
		return (400);
	}
	else if (_method == "POST" && _cgi.inCGI == false)
		_statusCode = "302 Redirect";
	else if (_method == "POST")
		_statusCode = "201 Created";
	else if (_method == "DELETE")
		_statusCode = "204 No Content";
	else if (_method == "GET" )
		_statusCode = "200 OK";
	return (200);
}
