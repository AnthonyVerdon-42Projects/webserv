#include "parsing.hpp"

Parsing::Parsing()
{
	_nbLine = 0;
	_inServerBlock = false;
	_inLocationBlock = false;
	_inMethodBlock = false;
	resetBlockArg(SERVER_BLOCK);
	resetBlockArg(LOCATION_BLOCK);
	resetBlockArg(METHOD_BLOCK);
}

Parsing::~Parsing()
{
}
