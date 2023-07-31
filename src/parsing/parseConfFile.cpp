#include "parsing.hpp"
#include <fstream>
#include <cstring>

#include <iostream>
void debug(t_location & location)
{
	std::cout << "LOCATION PATH:" + location.locationPath + "\n";
	std::cout << "ROOT:" + location.root + "\n";
	std::cout << "INDEX:" + location.index + "\n";
	std::cout << "DIRECTORY LISTING:" << location.directoryListing << "\n";
	std::cout << "REDIRECTION PATH:" << location.redirectionPath << "\n";
	std::cout << "REDIRECTION CODE:" << location.redirectionCode << "\n";
	std::cout << "ACCEPT UPLOADED FILE:" << location.acceptUploadedFile << "\n";
	std::cout << "UPLOADED FILE PATH:" + location.uploadedFilePath + "\n";
	std::cout << "GET:" << location.methodsAllowed[0] << "\n";
	std::cout << "POST:" << location.methodsAllowed[1] << "\n";
	std::cout << "DELETE:" << location.methodsAllowed[2] << "\n\n";
}

void	Parsing::checkDifferentServer()
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		for (size_t j = i + 1; j < _servers.size(); ++j)
		{
			if (_servers[i].port == _servers[j].port
				&& _servers[i].host == _servers[j].host)
				throw (ParsingError(SAME_ADDRESS));
		}
	}
}

void	Parsing::noBlock()
{
	if (_line.length() > 0 && _line != "server {")
		throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VARIABLE));
	_inServerBlock = true;
	initializeServer();
}

void Parsing::serverBlock()
{
	std::vector<std::string> lineSplit = splitString(_line, ' ');
	if (lineSplit.size() > 0 && lineSplit[0] == "location")
	{
		if (lineSplit.size() != 3)
			throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
		if (lineSplit[2] != "{")
			throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VARIABLE));
		if (!isValidPath(lineSplit[1]))
			throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
		std::string tmp;
		if (lineSplit[1][lineSplit[1].length() - 1] != '/')
			tmp = lineSplit[1] + "/";
		else
			tmp = lineSplit[1];
		_location.locationPath = tmp;
		initializeLocation();
		_inLocationBlock = true;
	}
	else if (_line == "}")
	{
		_servers.push_back(_server);
		_inServerBlock = false;
		resetBlockArg(SERVER_BLOCK);
	}
	else
		parseLineServerBlock();
}

void Parsing::locationBlock()
{
	std::vector<std::string> lineSplit = splitString(_line, ' ');
	if (lineSplit.size() > 0 && lineSplit[0] == "methods")
	{
		_inMethodBlock = true;
		if (lineSplit.size() != 2)
			throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
		if (lineSplit[1] != "{")
			throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VARIABLE));
	}
	else if (_line == "}")
	{
		testLocationValue();
		_server.locations.push_back(_location);
		_inLocationBlock = false;
		resetBlockArg(LOCATION_BLOCK);
	}
	else
		parseLineLocationBlock();
}

void Parsing::methodBlock()
{
	if (_line == "}")
	{
		_inMethodBlock = false;
		resetBlockArg(METHOD_BLOCK);
	}
	else
		parseLineMethodBlock();
}

const std::vector<t_server>& Parsing::readConfFile(std::ifstream & confFile)
{
	while (std::getline(confFile, _line))
	{
		_nbLine++;
		trimString(_line, WHITESPACE);
		_line = _line.substr(0, _line.find("##"));
		if (_line.length() == 0)
			continue;
		else if (!_inServerBlock && !_inLocationBlock && !_inMethodBlock)
			noBlock();
		else if (_inServerBlock && !_inLocationBlock && !_inMethodBlock)
			serverBlock();
		else if (_inServerBlock && _inLocationBlock && !_inMethodBlock)
			locationBlock();
		else if (_inServerBlock && _inLocationBlock && _inMethodBlock)
			methodBlock();
	}
	confFile.close();
	checkDifferentServer();
	return (_servers);
}

const std::vector<t_server>& Parsing::parseConfFile(const std::string &path)
{
	size_t extension = path.find(".conf");
	if (extension != std::string::npos && extension + 5 != path.length())
		throw (ParsingError(WRONG_EXTENSION));
	std::ifstream confFile(path.c_str(), std::ios_base::in);
	if (!confFile.is_open())
		throw (ParsingError(FILE_NOT_OPEN));
	return (readConfFile(confFile));
}
