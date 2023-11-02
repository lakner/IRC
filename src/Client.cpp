#include "Client.hpp"
#include "Commands.hpp"
#include <iostream>

Client::Client(int client_fd, std::string ipv4_addr) :
		_client_fd(client_fd), 
		_ipv4_addr(ipv4_addr),
		_nickname(std::to_string(client_fd)),
		_authenticated(false) { std::cout << _client_fd << std::endl;}

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
	std::string::size_type pos = _read_buffer.find("\r\n");
	if (pos != std::string::npos)
	{
		_read_buffer = _read_buffer.substr(pos + 2, _read_buffer.size());
		std::cout << "Client::clear_read_buffer, new read buffer: " << _read_buffer << std::endl;
	}
	else 
		_read_buffer.clear();
}

bool	Client::is_authd()
{
	return _authenticated;
}

void	Client::authenticate(bool valid)
{
	_authenticated = valid;
}

int		Client::get_client_fd()
{
	return (_client_fd);
}

std::string		Client::get_nickname()
{
	return (_nickname);
}

void		Client::set_nickname(std::string name)
{
	_nickname = name;
	_full_client_identifier =
		_nickname + "!" + _username + "@" + _ipv4_addr;
}

std::string		Client::get_username()
{
	return (_username);
}

void		Client::set_username(std::string name)
{
	_username = name;
	_full_client_identifier =
		_nickname + "!" + _username + "@" + _ipv4_addr;
}

std::string		Client::get_full_client_identifier()
{
	return(_full_client_identifier);
}