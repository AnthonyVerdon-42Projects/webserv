#include "parsing.hpp"

void	Parsing::initializeLocation()
{ 
	for (int i = 0; i < 3; ++i)
		_location.methodsAllowed[i] = true;
	_location.redirectionCode = 0;
	_location.redirectionPath = "includes/defaultPages/done.html";
	_location.root = _location.locationPath;
	_location.directoryListing = true;
	_location.index = "includes/defaultPages/index.html";
	_location.acceptUploadedFile = true;
	_location.uploadedFilePath = "includes/uploads/";
	for (int i = 0; i < 4; ++i)
		_location.lines[i] = 0;
	for (int i = 0; i < 4; ++i)
		_location.isDefined[i] = false;
}

void	Parsing::initializeServer()
{
	_server.host = "0.0.0.0";
	_server.port = 8080;
	_server.server_name = "_server";
	_server.isDefaultServer = false;
	_server.errpage = "includes/defaultPages/400";
	_server.maxFileSizeUpload = 0;
}
