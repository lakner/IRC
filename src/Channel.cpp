#include "Channel.hpp"
#include "Server.hpp"

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

int Channel::add_user(Client *client, std::string password)
{
	if (password == this->_password)
	{
		_client_list[client->get_nickname()] = client;
		return 0;
	}
	return -1;
}