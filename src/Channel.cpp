#include "Channel.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Numeric.hpp"

//Channel::Channel(std::string channel_name) : _channel_name(channel_name){}

Channel::Channel(): _channel_name(""), _password("")
{

}

Channel::Channel(std::string name, std::string pass)
	:_channel_name(name), _password(pass)
{
	
}

Channel::~Channel()
{

}

int		Channel::add_user(Client *client, std::string password)
{
	if (password == this->_password)
	{
		_client_list[client->get_nickname()] = client;
		notify_user_joined(client);
		send_topic(client);
		send_user_list(client);
		return 0;
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

void	Channel::send_topic(Client *client)
{
	Message msg;
	std::string msg_content = std::string(RPL_TOPIC);
	msg_content += " " + client->get_nickname();
	msg_content += " " + _channel_name + " :" + _topic;
	msg.send_to(client, msg_content);
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
	msg.send_to(client, msg_content);
	msg_content = std::string(RPL_ENDOFNAMES);
	msg.send_to(client, msg_content);
}