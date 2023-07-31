#include <iostream>
#include <csignal>
#include <cstdlib>
#include <vector>

#include "Server.hpp"
#include "parsing/parsing.hpp"

void signal_handler(int signum)
{
	(void)signum;
	throw std::runtime_error("SIGINT received");
}

int	main(int argc, char **argv)
{
	signal(SIGINT, signal_handler);

	std::vector<t_server> serverConfigFile;
	Parsing parser;
	try
	{
		if (argc == 2)
			serverConfigFile = parser.parseConfFile(argv[1]);
		else
			serverConfigFile = parser.parseConfFile("conf/real.conf");
	}
	catch(const std::string exception)
	{
		std::cerr << exception;
		return EXIT_FAILURE;
	}
	Server server;
	try
	{
		for (std::vector<t_server>::iterator it = serverConfigFile.begin(); it != serverConfigFile.end(); ++it) {
			server.addAddress(*it);
		}
		server.start();
	}
	catch(const std::exception & e)
	{
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
