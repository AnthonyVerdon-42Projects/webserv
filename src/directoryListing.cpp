#include "Request.hpp"

static void multipleRepace(std::string &str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

static void addElement(std::string &list, std::string element)
{
	list += "<li><span><a href=\"";
	list += element;
	list += "\">";
	list += element;
	list += "</a></span></li>\n";
}

void Request::directoryListing(DIR* directory, const std::string& dirName) {
	std::string tmpHTML;
	std::ifstream htmlFile("includes/defaultPages/directoryListing.html");
	if (htmlFile.is_open()) {
		std::string line;
		while (getline(htmlFile, line)) {
			tmpHTML += line + "\n";
		}
		htmlFile.close();
	}
	else {
		std::cerr << "Error: directoryListing.html not found" << std::endl;
		tmpHTML = "<ul><!--DIRLIST--></ul>";
	}

	std::string dirList;
	struct dirent* file;
	addElement(dirList, "/");
	addElement(dirList, "..");
	while (true) {
		file = readdir(directory);
		if (file == NULL)
			break;
		if (file->d_name[0] == '.')
			continue ;
		addElement(dirList, file->d_name);
	}

	multipleRepace(tmpHTML, "<!--DIRLIST-->", dirList);
	multipleRepace(tmpHTML, "<!--DIRNAME-->", dirName);
	size_t fileSize = tmpHTML.length();

	std::ostringstream ss;
	ss << "HTTP/1.1 " << _statusCode << "\r\n";
	if (_requestHeader[LOCATION] != "")
		ss << "Location: " << _requestHeader[LOCATION] << "\r\n";
	ss << "Content-type: " + _requestHeader[ACCEPT] + "\r\n";
	ss << "Content-Length: " << fileSize << "\r\n\r\n";
	ss << tmpHTML;
	write(_clientfd, ss.str().c_str(), ss.str().size());
	ss.str("");
	ss.clear();
	closedir(directory);
}
