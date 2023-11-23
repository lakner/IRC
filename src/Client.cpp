#include "Client.hpp"
#include "Commands.hpp"
#include <iostream>

using std::string;

Client::Client() :
		_client_fd(-1), 
		_client_ipv4_addr(""),
		_server_ipv4_addr(""),
		_server_string(""),
		_nickname(""),
		_authenticated(0)
{
}

Client::Client(int client_fd, string client_ipv4_addr, string server_ipv4_addr) :
		_client_fd(client_fd), 
		_client_ipv4_addr(client_ipv4_addr),
		_server_ipv4_addr(server_ipv4_addr),
		_server_string(":" + _server_ipv4_addr + " "),
		_nickname(""),
		_authenticated(0) 
{
	// std::cout << "New client: " << _client_fd << " ip_addr: " << ipv4_addr << std::endl;
}

Client::~Client()
{

}

void	Client::set_read_buffer(char *buffer)
{
	_read_buffer += string(buffer);
}


string	Client::get_read_buffer(void)
{
	return (_read_buffer);
}

void	Client::set_write_buffer(string buffer)
{
	_write_buffer = buffer;
}

void	Client::append_write_buffer(string buffer)
{
	_write_buffer += buffer;
}

string	Client::get_write_buffer(void)
{
	return (_write_buffer);
}

void	Client::clear_write_buffer(void)
{
	_write_buffer.clear();
}

void	Client::clear_read_buffer(void)
{
	string::size_type pos = _read_buffer.find("\r\n");
	if (pos != string::npos)
	{
		_read_buffer = _read_buffer.substr(pos + 2, _read_buffer.size());
		std::cout << "Client::clear_read_buffer, new read buffer: " << _read_buffer << std::endl;
	}
	else 
		_read_buffer.clear();
}

int		Client::is_authd()
{
	return _authenticated;
}

void	Client::authenticate(int status)
{
	_authenticated = status;
}

int		Client::get_client_fd()
{
	return (_client_fd);
}

string		Client::get_nickname()
{
	return (_nickname);
}

void		Client::set_nickname(string name)
{
	_nickname = name;
	_full_client_identifier =
		_nickname + "!" + _username + "@" + _client_ipv4_addr;
}

string		Client::get_username()
{
	return (_username);
}

void		Client::set_username(string name)
{
	_username = name;
	_full_client_identifier =
		_nickname + "!~" + _username + "@" + _client_ipv4_addr;
}

string		Client::get_full_client_identifier()
{
	return(_full_client_identifier);
}

int	Client::send_all_in_write_buffer()
{
	string to_send = _write_buffer;
	if (to_send.empty())
		return 0;
	int numbytes = send(_client_fd, to_send.c_str(), to_send.size(), 0);
	if (numbytes == -1)
		return numbytes;
	if (static_cast<size_t>(numbytes) < to_send.size())
		set_write_buffer(to_send.substr(numbytes));
	else
		clear_write_buffer();
	return 0;
}

string Client::get_server_string()
{
	return(_server_string);
}

void Client::send_to_all_in_my_channels(Server *server, string response)
{
	std::map<string, Channel>& channels = server->get_channels();
	std::map<string, Channel>::iterator it = channels.begin();

	for (; it != channels.end(); it++) 
	{
		if (it->second.client_in_channel(server->get_client(_nickname)))
			it->second.send_to_all_in_channel(response);
	}
}