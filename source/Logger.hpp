#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdio.h>

#include <string>

class Logger
{
public:

	static void Log(std::string message)
	{
		printf("%s", message.c_str());
	}

private:

};


#endif