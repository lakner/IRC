#include "Server.hpp"
#include <iostream>
#include <string>
#include <exception>
#include <cstdlib>

using std::string;

int	output_err(string errormessage)
{
	std::cerr << "Error: " << errormessage << std::endl;
	return 1;
}

int main(int argc, char** argv)
{
	if (argc != 3)
		return output_err("Wrong number of arguments.");
	try
	{
		Server ircserver = Server(argv[1], argv[2]);
		if (ircserver.prepare() == -1)
		{
			throw std::runtime_error("Error preparing the socket");
		}
		else
		{
			std::cout << "Prepared successfully." << std::endl;
		}
		ircserver.run();
	}
	catch(std::exception e)
	{
		output_err(e.what());
		exit(1);
	}
}