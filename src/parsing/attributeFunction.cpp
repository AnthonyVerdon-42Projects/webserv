#include "parsing.hpp"
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>

//LOCATION BLOCK ATTRIBUTE
void Parsing::testLocationValue()
{
	if (_location.isDefined[ROOT] && !isValidPathDir(_location.root))
		throw(ParsingError("line " + intToString(_location.lines[ROOT]) + INCORRECT_VALUE));
	if (_location.isDefined[REDIRECTION] && !isValidPath(_location.root +_location.redirectionPath))
		throw(ParsingError("line " + intToString(_location.lines[REDIRECTION]) + INCORRECT_VALUE));
	if (_location.isDefined[INDEX] && !isValidPath(_location.root +_location.index))
		throw(ParsingError("line " + intToString(_location.lines[INDEX]) + INCORRECT_VALUE));
	if (_location.isDefined[UPLOAD] && !isValidPathDir(_location.root +_location.uploadedFilePath))
		throw(ParsingError("line " + intToString(_location.lines[UPLOAD]) + INCORRECT_VALUE));
}

void Parsing::CGIAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 3)
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
	if (access(lineSplit[1].c_str(), 0) != 0)
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
	_location.executableCGI.push_back(lineSplit[1]);
	_location.extensionCGI.push_back(lineSplit[2]);
}

void Parsing::methodAttribute(const std::vector<std::string> & lineSplit)
{
	(void)lineSplit;
}

void Parsing::returnAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 3)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	_location.redirectionCode = strtol(lineSplit[1].c_str(), NULL, 10);
	if (errno != 0)
		throw(ParsingError(strerror(errno)));
	else if (!isDigit(lineSplit[1]))
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
	_location.redirectionPath = lineSplit[2];
	_location.lines[REDIRECTION] = _nbLine;
	_location.isDefined[REDIRECTION] = true;
}

void Parsing::rootAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	std::string tmp;
	if (lineSplit[1][lineSplit[1].length() - 1] != '/')
		tmp = lineSplit[1] + "/";
	else
		tmp = lineSplit[1];
	_location.root = tmp;
	_location.lines[ROOT] = _nbLine;
	_location.isDefined[ROOT] = true;
}

void Parsing::directoryListingAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	if (lineSplit[1] == "ON")
		_location.directoryListing = true;
	else if (lineSplit[1] == "OFF")
		_location.directoryListing = false;
	else
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
}

void Parsing::indexAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	_location.index = lineSplit[1];
	_location.lines[INDEX] = _nbLine;
	_location.isDefined[INDEX] = true;
}
void Parsing::acceptUploadedFileAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	if (lineSplit[1] == "ON")
		_location.acceptUploadedFile = true;
	else if (lineSplit[1] == "OFF")
		_location.acceptUploadedFile = false;
	else
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
}

void Parsing::saveUploadedFileAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	std::string tmp;
	if (lineSplit[1][lineSplit[1].length() - 1] != '/')
		tmp = lineSplit[1] + "/";
	else
		tmp = lineSplit[1];
	_location.uploadedFilePath = tmp;
	_location.lines[UPLOAD] = _nbLine;
	_location.isDefined[UPLOAD] = true;
}

//SERVER BLOCK ATTRIBUTE

void Parsing::listenAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 3)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	std::vector<std::string> portSplit = splitString(lineSplit[2], '.');
	if (portSplit.size() != 4 || countChar(lineSplit[2], '.') != 3)
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
	for (size_t j = 0; j < 4; ++j)
	{
		if (!isDigit(portSplit[j]))
			throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
	}
	_server.host = lineSplit[2];
	if (!isDigit(lineSplit[1]))
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
	_server.port = strtol(lineSplit[1].c_str(), NULL, 10);
	if (errno != 0)
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
}

void Parsing::serverNameAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	_server.server_name = lineSplit[1];
}

void Parsing::errpageAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	if (!isValidPath(lineSplit[1]))
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
	_server.errpage = lineSplit[1];
}

void Parsing::maxFilesizeUploadAttribute(const std::vector<std::string> & lineSplit)
{
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	_server.maxFileSizeUpload = strtol(lineSplit[1].c_str(), NULL, 10);
	if (errno != 0)
		throw(ParsingError(strerror(errno)));
	else if (!isDigit(lineSplit[1]))
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
}

void Parsing::locationAttribute(const std::vector<std::string> & lineSplit)
{
	(void)lineSplit;
	if (lineSplit.size() != 3)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
}
