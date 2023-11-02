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
		std::string						_topic;
		std::map<std::string, Client*>	_client_list;
		void							notify_user_joined(Client *client);
		void							send_topic(Client *client);
		void							send_user_list(Client *client);

};

#endif