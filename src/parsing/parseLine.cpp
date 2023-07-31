#include "parsing.hpp"

std::map<std::string, function> Parsing::initFunctionMap(int block)
{
	std::map<std::string, function> functionMap;
	if (block == SERVER_BLOCK)
	{
		functionMap["listen"] = &Parsing::listenAttribute;
		functionMap["server_name"] = &Parsing::serverNameAttribute;
		functionMap["errpage"] = &Parsing::errpageAttribute;
		functionMap["max_filesize_upload"] = &Parsing::maxFilesizeUploadAttribute;
		functionMap["location"] = &Parsing::locationAttribute;
	}
	else if (block == LOCATION_BLOCK)
	{
		functionMap["methods"] = &Parsing::methodAttribute;
		functionMap["return"] = &Parsing::returnAttribute;
		functionMap["root"] = &Parsing::rootAttribute;
		functionMap["directory_listing"] = &Parsing::directoryListingAttribute;
		functionMap["index"] = &Parsing::indexAttribute;
		functionMap["CGI"] = &Parsing::CGIAttribute;
		functionMap["accept_uploaded_file"] = &Parsing::Parsing::acceptUploadedFileAttribute;
		functionMap["save_uploaded_file"] = &Parsing::saveUploadedFileAttribute;
	}
	return (functionMap);
}

void Parsing::resetBlockArg(int block)
{
	if (block == SERVER_BLOCK)
	{
		for (int i = 0; i < 5; ++i)
			_argumentUsedServer[i] = false;
	}
	else if (block == LOCATION_BLOCK)
	{
		for (int i = 0; i < 8; ++i)
			_argumentUsedLocation[i] = false;
	}
	else if (block == METHOD_BLOCK)
	{
		for (int i = 0; i < 3; ++i)
			_argumentUsedMethod[i] = false;
	}
}

void	Parsing::parseLineMethodBlock()
{
	std::vector<std::string> lineSplit = splitString(_line, ' ');
	if (lineSplit.size() != 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	
	std::string arguments[3] = {"GET", "POST", "DELETE"};
	for (int i = 0; i < 3; ++i)
	{
		if (arguments[i] == lineSplit[0])
		{
			if (_argumentUsedMethod[i])
				throw(ParsingError("line " + intToString(_nbLine) + REDECLARATION));
			_argumentUsedMethod[i] = true;
			if (lineSplit[1] == "ON")
				_location.methodsAllowed[i] = true;
			else if (lineSplit[1] == "OFF")
				_location.methodsAllowed[i] = false;
			else
				throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VALUE));
			return;
		}
	}
	throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VARIABLE));
}

void	Parsing::parseLineLocationBlock()
{
	std::vector<std::string> lineSplit = splitString(_line, ' ');
	
	const std::map<std::string, function> functionMap = initFunctionMap(LOCATION_BLOCK);
	int i = 0;
	std::map<std::string, function>::const_iterator it;
	
	for (it = functionMap.begin(); it != functionMap.end(); ++it)
	{
		if (it->first == lineSplit[0])
		{
			if (_argumentUsedLocation[i])
				throw(ParsingError("line " + intToString(_nbLine) + REDECLARATION));
			if (lineSplit[0] != "CGI")
				_argumentUsedLocation[i] = true;
			(this->*it->second)(lineSplit);
			return ;
		}
		i++;
	}
	throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VARIABLE));
}

void	Parsing::parseLineServerBlock()
{
	std::vector<std::string> lineSplit = splitString(_line, ' ');
	
	if (lineSplit.size() < 2)
		throw(ParsingError("line " + intToString(_nbLine) + WRONG_NB_ARG));
	
	const std::map<std::string, function>  functionMap = initFunctionMap(SERVER_BLOCK);
	int i = 0;
	std::map<std::string, function>::const_iterator it;
	for (it = functionMap.begin(); it != functionMap.end(); ++it)
	{
		if (it->first == lineSplit[0])
		{
			if (_argumentUsedServer[i])
				throw(ParsingError("line " + intToString(_nbLine) + REDECLARATION));
			_argumentUsedServer[i] = true;
			(this->*it->second)(lineSplit);
			return ;
		}
		i++;
	}
	throw(ParsingError("line " + intToString(_nbLine) + INCORRECT_VARIABLE));
}
