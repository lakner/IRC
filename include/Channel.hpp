#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <map>

class	Client;
class	Server;

class	Channel
{
	public:
		Channel();
		Channel(std::string channel_name, std::string password);
		~Channel();
		int add_user(Client *client, std::string password);
		
	private:
		std::string						_channel_name;
		std::string						_password;
		std::map<std::string, Client*>	_client_list;

};

#endif