#include "Server.hpp"
#include <iostream>
#include <string>
#include <exception>


int	output_err(std::string errormessage)
{
	std::cerr << "Error: " << errormessage << std::endl;
	return 1;
}

int main(int argc, char** argv)
{
	std::cout << "Hello world" << std::endl;
	if (argc != 3)
		return output_err("Wrong number of arguments.");
	try
	{
		unsigned int port = std::stoi(argv[1]);
		Server ircserver = Server(port, argv[2]);
		ircserver.start();
	}
	catch(std::exception e)
	{
		output_err(e.what());
	}
}