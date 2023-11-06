#include "Channel.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Numeric.hpp"

Channel::Channel(): _channel_name(""), _password("")
{
	Mode	initial_mode;

	initial_mode.i = 0;
	initial_mode.k = 0;
	initial_mode.l = 0;
	initial_mode.o = 0;
	initial_mode.t = 0;
	_mode = initial_mode;
}

Channel::Channel(std::string name, std::string pass)
	:_channel_name(name), _password(pass)
{
	Mode	initial_mode;

	initial_mode.i = 0;
	initial_mode.k = 0;
	initial_mode.l = 0;
	initial_mode.o = 0;
	initial_mode.t = 0;
	_mode = initial_mode;
}

Channel::~Channel()
{

}

//if client already exist in a channel just notify
int		Channel::add_user(Client *client, std::string password)
{
	if (password == this->_password)
	{
		//_client_list.insert(std::pair<std::string, Client*>(client->get_nickname(), client));
		_client_list[client->get_nickname()] = client;
		notify_user_joined(client);
		send_topic(client);
		send_user_list(client);
		return 0;
	}
	return -1;
}

//removes a client form the client list of the channel
int		Channel::remove_user(Client *client, std::string password)
{
	if (password == this->_password)
	{
		std::string keyToRemove = client->get_nickname();
		std::map<std::string, Client*>::iterator it = _client_list.find(keyToRemove);

		if (it != _client_list.end()) {
			_client_list.erase(it);
		} else {
			return (-1);
		}
		notify_user_exit(client);
		send_topic(client);
		send_user_list(client);
		return 0;
	}
	return -1;
}

int		Channel::add_operator(Client *client, std::string password)
{
	if (password == this->_password)
	{
		_operator_list[client->get_nickname()] = client;
		notify_user_is_operator(client);
		return 0;
	}
	return -1;
}

//removes a client form the operator list of the channel
int		Channel::remove_operator(Client *client, std::string password)
{
	if (password == this->_password)
	{
		std::string keyToRemove = client->get_nickname();
		std::map<std::string, Client*>::iterator it = _operator_list.find(keyToRemove);

		if (it != _operator_list.end())
		{
			_operator_list.erase(it);
			return 0;
		} else {
			return (-1);
		}
	}
	return -1;
}

void	Channel::notify_user_joined(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	Message msg;
	std::string content = ":" + client->get_full_client_identifier();
	content += " JOIN " + _channel_name;
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		msg.send_to(it->second, content);
}

void	Channel::notify_user_exit(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	Message msg;
	std::string content = ":" + client->get_full_client_identifier();
	content += " EXIT " + _channel_name;
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		msg.send_to(it->second, content);
}

void	Channel::notify_user_is_operator(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	Message msg;
	std::string content = ":" + client->get_full_client_identifier();
	content += " MODE " + _channel_name + " +o " + client->get_nickname();
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		msg.send_to(it->second, content);
}

void	Channel::notify_mode_changed(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	Message msg;
	std::string content = ":" + client->get_full_client_identifier();
	content += " MODE " + _channel_name + " +o";
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		msg.send_to(it->second, content);
}

void	Channel::send_topic(Client *client)
{
	Message msg;
	std::string msg_content = std::string(RPL_TOPIC);
	msg_content += " " + client->get_nickname();
	msg_content += " " + _channel_name + " :" + _topic;
	msg.send_from_server(client, msg_content);
}	

void	Channel::send_user_list(Client *client)
{
	Message msg;
	std::string msg_content = std::string(RPL_NAMREPLY); 
	msg_content += " " + client->get_nickname();
	msg_content += " " + _channel_name + " :";
	
	std::map<std::string, Client*>::iterator it;
	for (it = _client_list.begin(); it != _client_list.end(); it++)
		msg_content += it->first + " ";
	msg.send_from_server(client, msg_content);
	msg_content = std::string(RPL_ENDOFNAMES);
	msg.send_from_server(client, msg_content);
}

int Channel::send_to_all_in_channel(Message *msg)
{
	std::map<std::string, Client*>::iterator it;
	for (it = _client_list.begin(); it != _client_list.end(); it++)
		msg->send_to(it->second, msg->get_payload());
	return 0;
}

std::string	Channel::get_channel_name( void )
{
	return (_channel_name);
}

std::map<std::string, Client*>&	Channel::get_users( void )
{
	return (_client_list);
}

std::map<std::string, Client*>&	Channel::get_operators( void )
{
	return (_operator_list);
}

Mode Channel::get_mode( void )
{
	return (_mode);
}

std::string	Channel::get_password( void )
{
	return (_password);
}