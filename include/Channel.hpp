#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include "Client.hpp"
#include <map>

class	Channel
{
	public:
		Channel(std::string channel_name);
		~Channel();
		
	private:
		std::string						_channel_name;
		std::map<std::string, Client>	_client_list;

};

#endif