#include "Client.hpp"
#include <iostream>

Client::Client(int client_fd) : _client_fd(client_fd) { std::cout << _client_fd << std::endl;}

Client::~Client()
{

}

void	Client::set_read_buffer(char *buffer)
{
	_read_buffer += std::string(buffer);
}

std::string	Client::get_read_buffer(void)
{
	return (_read_buffer);
}

void	Client::set_write_buffer(std::string buffer)
{
	_write_buffer += buffer;
}

std::string	Client::get_write_buffer(void)
{
	return (_write_buffer);
}

void	Client::clear_write_buffer(void)
{
	_write_buffer.clear();
}

void	Client::clear_read_buffer(void)
{
	_read_buffer.clear();
}
