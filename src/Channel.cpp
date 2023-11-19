#include "Channel.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Numeric.hpp"

Channel::Channel(): _channel_name(""), _password(""), _userlimit(9999), _invite_only(false), _topic_change(false)
{

}

Channel::Channel(std::string name, std::string pass)
	:_channel_name(name), _password(pass) , _userlimit(9999), _invite_only(false), _topic_change(false)
{

}

Channel::~Channel()
{

}

//if client already exist in a channel just notify
std::string	Channel::add_user(Client *client, std::string password)
{
	if (password == this->_password)
	{
		// first user automatically is an operator
		if (_client_list.empty())
			add_operator(client);
		//_client_list.insert(std::pair<std::string, Client*>(client->get_nickname(), client));
		if (_channelusers == _userlimit)
			return (ERR_CHANNELISFULL);
		_client_list[client->get_nickname()] = client;
		_channelusers++;
		notify_user_joined(client);
		send_topic(client);
		send_user_list(client);
		return ("");
	}
	return (ERR_BADCHANNELKEY);
}

//removes a client form the client list of the channel
int		Channel::remove_user(Client *client)
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

int		Channel::add_operator(Client *client)
{
	_operator_list[client->get_nickname()] = client;
	notify_user_is_operator(client);
	return 0;
}

//removes a client form the operator list of the channel
int		Channel::remove_operator(Client *client)
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
	return -1;
}

void	Channel::notify_user_joined(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	std::string content = ":" + client->get_full_client_identifier();
	content += " JOIN " + _channel_name;
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		Message::send_to(it->second, content);
}

void	Channel::notify_user_exit(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	std::string content = ":" + client->get_full_client_identifier();
	content += " EXIT " + _channel_name;
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		Message::send_to(it->second, content);
}

void	Channel::notify_user_is_operator(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	std::string content = ":" + client->get_full_client_identifier();
	content += " MODE " + _channel_name + " +o " + client->get_nickname();
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		Message::send_to(it->second, content);
}

void	Channel::notify_mode_changed(Client *client)
{
	std::map<std::string, Client*>::iterator it;
	std::string content = ":" + client->get_full_client_identifier();
	content += " MODE " + _channel_name + " +o";
	for (it = _client_list.begin();	it != _client_list.end(); it++)
		Message::send_to(it->second, content);
}

void	Channel::send_topic(Client *client)
{
	std::string msg_content;
	if (_topic.empty())
		msg_content = std::string(RPL_NOTOPIC);
	else
		msg_content = std::string(RPL_TOPIC);
	msg_content += " " + client->get_nickname();
	msg_content += " " + _channel_name + " :" + _topic;
	Message::send_from_server(client, msg_content);
}	

void	Channel::send_user_list(Client *client)
{
	std::string msg_content = std::string(RPL_NAMREPLY); 
	msg_content += " " + client->get_nickname();
	msg_content += " = " + _channel_name + " :"; //add @ before operator name
	
	std::map<std::string, Client*>::iterator it;
	for (it = _client_list.begin(); it != _client_list.end(); it++)
		msg_content += it->first + " ";
	Message::send_from_server(client, msg_content);
	msg_content = std::string(RPL_ENDOFNAMES);
	msg_content += " " + client->get_nickname();
	msg_content += " " + _channel_name + " :End of /NAMES list";
	Message::send_from_server(client, msg_content);
}

int Channel::send_to_all_in_channel(std::string content)
{
	std::map<std::string, Client*>::iterator it;
	for (it = _client_list.begin(); it != _client_list.end(); it++)
	{
		std::cout << "send_to_all_in_channel: Iterating over client list. Current client: " << it->second->get_nickname() << std::endl;
		Message::send_to(it->second, content);
	}
	return 0;
}

int Channel::send_to_all_except(std::string content, Client& client)
{
	std::map<std::string, Client*>::iterator it;
	for (it = _client_list.begin(); it != _client_list.end(); it++)
	{
		std::cout << "send_to_all_except: Iterating over client list. Current client: " << it->second->get_nickname() << std::endl;
		if (it->second != &client)
			Message::send_to(it->second, content);
	}
	return 0;
}


bool	Channel::client_in_channel(Client& client)
{
	std::map<std::string, Client*>::iterator it = _client_list.begin();

	for(; it != _client_list.end(); it++)
	{
		if (&client == it->second)
			return true;
	}
	return false;
}

bool	Channel::allowed_to_set_topic(std::string nickname)
{
	if (!_topic_change)
		return (true);

	if (_operator_list.find(nickname) != _operator_list.end())
		return (true);

	return (false);
}

bool	Channel::is_operator(std::string nickname)
{
	std::map<std::string, Client*>::iterator it = _operator_list.begin();

	for (; it != _operator_list.end(); it++) {
		Client *client = it->second;

		if (client->get_nickname() == nickname)
			return (true);
	}
	return (false);
}

bool	Channel::allowed_to_invite(std::string nickname) //change
{
	if (_invite_only)
		return(is_operator(nickname));
	return (true);
}

bool	Channel::allowed_to_kick(std::string nickname) //change
{
	return(is_operator(nickname));
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


std::string	Channel::get_password( void )
{
	return (_password);
}

std::string	Channel::get_topic( void )
{
	return (_topic);
}

void	Channel::set_topic( std::string new_topic )
{
	this->_topic = new_topic;
}

bool	Channel::get_invite_only()
{
	return (_invite_only);
}


void	Channel::kick(std::string nickname)
{
	std::map<std::string, Client*>::iterator it = _client_list.begin();

	for (; it != _client_list.end(); it++) 
	{
		Client* cl = it->second;

		if (cl->get_nickname() == nickname)
		{
			remove_user(cl);
			remove_operator(cl);
		}
	}
}

// std::string Channel::add_mode_change(char mode, bool *sign, bool mode_stat)
// {
	
// 	std::string ret;

// 	if (mode_stat != *sign)
// 	{
// 		if (mode_stat == 0)
// 			ret = "-";
// 		else
// 			ret = "+";
// 		*sign = mode_stat;
// 	}
// 	ret += mode;
// 	return (ret);
// }


std::string	Channel::set_mode(char mode, bool mode_stat, std::stringstream *param, Server *server, Message *msg)
{
	std::string temp;
	Client		*sender = msg->get_sender();
	switch (mode)
	{
		case 'i':
			if (_invite_only != mode_stat)
			{
				_invite_only = mode_stat;
				return ("i");
			}
			break ;

		case 't':
			if (_topic_change != mode_stat)
			{
				_topic_change = mode_stat;
				return ("t");
			}
			break ;

		case 'k':
			*param >> temp;
			if (mode_stat && _password == temp)
				break;
			else if ((mode_stat && _password.empty()) || (!mode_stat && _password != temp))
			{
				_password = temp;
				msg->send_to(sender, ":" + sender->get_full_client_identifier() + " MODE " + get_channel_name() + " +k :" + _password);
			}
			else if (!mode_stat && _password == temp)
			{
				_password.clear();
				msg->send_to(sender, ":" + sender->get_full_client_identifier() + " MODE " + get_channel_name() + " -k :" + _password);
			}
			break ;

		case 'o':
			if (mode_stat)
			{
				*param >> temp;
				if (!is_operator(temp))
					add_operator(&server->get_client(temp));
				else
					std::cout << "error" << std::endl;
					//reply error
			}
			else
			{
				if (is_operator(temp))
					remove_operator(&server->get_client(temp));
				else
					std::cout << "error" << std::endl;
					//reply error
			}
			break ;

		case 'l': //maybe add unlimited with -l??
			if (mode_stat)
			{
				*param >> temp;
				try {
	    	    	int limit = std::stoi(temp);
					if (limit > 9999)
						_userlimit = 9999;
					else
						_userlimit = limit;
					msg->send_to(sender, ":" + sender->get_full_client_identifier() + " MODE " + get_channel_name() + " +l :" + std::to_string(_userlimit));
   			 	} catch (const std::invalid_argument& e) {
        			std::cerr << "Invalid argument: " << e.what() << std::endl; //no code for this case
    			} catch (const std::out_of_range& e) {
					_userlimit = 9999;
        			msg->send_to(sender, ":" + sender->get_full_client_identifier() + " MODE " + get_channel_name() + " +l :" + std::to_string(_userlimit));
    			}
			}
			else
			{
				_userlimit = 9999;
        		msg->send_to(sender, ":" + sender->get_full_client_identifier() + " MODE " + get_channel_name() + " :-l");
			}
			break ;
	}
	return ("");
}

std::string Channel::get_modes()
{
	std::string modes = "+";

	if (_invite_only)
		modes += "i";
	if (!_password.empty())
		modes += "k";
	if (_userlimit != 9999)
		modes += "l";
	if (_topic_change)
		modes += "t";
	std::cout << modes << std::endl;
	return (modes);
}