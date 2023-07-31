#include <sstream>
#include <vector>
#include <dirent.h>
#include <unistd.h>

const std::string ParsingError(std::string error) throw()
{
	return ("parsing error: " + error + "\n");
}

const std::string intToString(int number)
{
	std::stringstream stringStream;
	stringStream << number;
	return (stringStream.str());
}

const std::vector<std::string> splitString(const std::string string, const char delimiter)
{
	std::stringstream 			ss(string);
	std::vector<std::string>	strings;
	std::string 				buffer;
	
	while(getline(ss, buffer, delimiter))
    	strings.push_back(buffer);
	return (strings);
}

void trimString(std::string& string, const char* charset)
{
	size_t start = string.find_first_not_of(charset);
	size_t end = string.find_last_not_of(charset);
	if (start != std::string::npos)
		string = string.substr(start, end - start + 1);
	else
		string = "";
}

bool isDigit(const std::string string)
{
	for (int i = 0; string[i]; ++i)
	{
		if (string[i] < '0' || string[i] > '9')
			return (false);
	}
	return (true);
}

bool isValidPath(const std::string string)
{
	if (access(string.c_str(), 0) == 0)
		return (true);
	return (false);
}

bool isValidPathDir(const std::string string)
{
	DIR *tmp;
	tmp = opendir(string.c_str());
	if (tmp)
	{
		closedir(tmp);
		return (true);
	}
	return (false);
}

size_t countChar(const std::string string, const char delimiter)
{
	size_t nbChar = 0;
	for (size_t i = 0; string[i]; ++i)
	{
		if (string[i] == delimiter)
			nbChar++;
	}
	return (nbChar);
}
