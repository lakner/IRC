#include "Channel.hpp"
#include "Server.hpp"
#include "Message.hpp"
#include "Numeric.hpp"

using std::string;

Channel::Channel(): _channelusers(0), _channel_name(""), _password(""), _userlimit(9999), _invite_only(false), _topic_change(false)
{

}

Channel::Channel(string name, string pass)
	: _channelusers(0),_channel_name(name), _password(pass) , _userlimit(9999), _invite_only(false), _topic_change(false)
{

}

Channel::~Channel()
{

}

//if client already exist in a channel just notify
string	Channel::add_user(Client *client, string password)
{
	if (password == this->_password)
	{
		// first user automatically is an operator
		if (_client_list.empty())
			add_operator(client);
		std::cout << "LIMIT OF USERS: " << _userlimit << " CURRENT USERS: " << _channelusers << std::endl;
		//_client_list.insert(std::pair<string, Client*>(client->get_nickname(), client));
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
	string keyToRemove = client->get_nickname();
	std::map<string, Client*>::iterator it = _client_list.find(keyToRemove);
	if (it != _client_list.end()) {
		_client_list.erase(it);
	} else {
		return (-1);
	}
	notify_user_exit(client);
	// send_topic(client);
	send_user_list(client);
	return 0;
}

int		Channel::add_operator(Client *client)
{
	_operator_list[client->get_nickname()] = client;
	return 0;
}

//removes a client form the operator list of the channel
int		Channel::remove_operator(Client *client)
{
	string keyToRemove = client->get_nickname();
	std::map<string, Client*>::iterator it = _operator_list.find(keyToRemove);

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
	string content = ":" + client->get_full_client_identifier();
	content += " JOIN " + _channel_name;
	send_to_all_in_channel(content);
}

void	Channel::notify_user_exit(Client *client)
{
	string content = ":" + client->get_full_client_identifier();
	content += " EXIT " + _channel_name;
	send_to_all_in_channel(content);
}

// void	Channel::notify_user_is_operator(Client *client)
// {
// 	string content = ":" + client->get_full_client_identifier();
// 	content += " MODE " + _channel_name + " +o " + client->get_nickname();
// 	send_to_all_in_channel(content);
// }

// void	Channel::notify_mode_changed(Client *client)
// {
// 	std::map<string, Client*>::iterator it;
// 	string content = ":" + client->get_full_client_identifier();
// 	content += " MODE " + _channel_name + " +o";
// 	for (it = _client_list.begin();	it != _client_list.end(); it++)
// 		Message::send_to(it->second, content);
// }

void	Channel::send_topic(Client *client)
{
	string msg_content;
	if (_topic.empty())
		msg_content = string(RPL_NOTOPIC);
	else
		msg_content = string(RPL_TOPIC);
	msg_content += " " + client->get_nickname();
	msg_content += " " + _channel_name + " :" + _topic;
	Message::send_from_server(client, msg_content);
}	

void	Channel::send_user_list(Client *client)
{
	string msg_content = string(RPL_NAMREPLY); 
	msg_content += " " + client->get_nickname();
	msg_content += " = " + _channel_name + " :"; //add @ before operator name
	
	std::map<string, Client*>::iterator it;
	for (it = _client_list.begin(); it != _client_list.end(); it++)
	{
		if (is_operator(it->first))
			msg_content += "@";
		msg_content += it->first + " ";
	}
	Message::send_from_server(client, msg_content);
	msg_content = string(RPL_ENDOFNAMES);
	msg_content += " " + client->get_nickname();
	msg_content += " " + _channel_name + " :End of /NAMES list";
	
	Message::send_from_server(client, msg_content);
}

int Channel::send_to_all_in_channel(string content)
{
	std::map<string, Client*>::iterator it;
	for (it = _client_list.begin(); it != _client_list.end(); it++)
	{
		std::cout << "send_to_all_in_channel: Iterating over client list. Current client: " << it->second->get_nickname() << std::endl;
		Message::send_to(it->second, content);
	}
	return 0;
}

int Channel::send_to_all_except(string content, Client& client)
{
	std::map<string, Client*>::iterator it;
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
	std::map<string, Client*>::iterator it = _client_list.begin();

	for(; it != _client_list.end(); it++)
	{
		if (&client == it->second)
			return true;
	}
	return false;
}

bool	Channel::allowed_to_set_topic(string nickname)
{
	if (!_topic_change)
		return (true);

	if (_operator_list.find(nickname) != _operator_list.end())
		return (true);

	return (false);
}

bool	Channel::is_operator(string nickname)
{
	std::map<string, Client*>::iterator it = _operator_list.begin();

	for (; it != _operator_list.end(); it++) {
		Client *client = it->second;

		if (client->get_nickname() == nickname)
			return (true);
	}
	return (false);
}

// bool	Channel::allowed_to_invite(string nickname) //change
// {
// 	if (_invite_only)
// 		return(is_operator(nickname));
// 	return (true);
// }

// bool	Channel::allowed_to_kick(string nickname) //change
// {
// 	return(is_operator(nickname));
// }

string	Channel::get_channel_name( void )
{
	return (_channel_name);
}

std::map<string, Client*>&	Channel::get_users( void )
{
	return (_client_list);
}

std::map<string, Client*>&	Channel::get_operators( void )
{
	return (_operator_list);
}


string	Channel::get_password( void )
{
	return (_password);
}

string	Channel::get_topic( void )
{
	return (_topic);
}

void	Channel::set_topic( string new_topic )
{
	this->_topic = new_topic;
}

bool	Channel::get_invite_only()
{
	return (_invite_only);
}


void	Channel::kick(string nickname)
{
	std::map<string, Client*>::iterator it = _client_list.begin();

	for (; it != _client_list.end(); it++) 
	{
		Client* cl = it->second;

		if (cl->get_nickname() == nickname)
		{
			remove_user(cl);
			remove_operator(cl);
			break ;
		}
	}
}


void	Channel::invite(Client *client)
{
	add_user(client, _password);
}

// string Channel::add_mode_change(char mode, bool *sign, bool mode_stat)
// {
	
// 	string ret;

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


string	Channel::set_mode(char mode, bool mode_stat, std::stringstream *param, Server *server, Message *msg)
{
	string temp;
	Client		*sender = msg->get_sender();
	string mode_message = ":" + sender->get_full_client_identifier() + " MODE " + _channel_name;

	if (mode == 'k' || mode == 'l' || mode == 'o')
	{
		*param >> temp;
		if (temp.empty())
		{
			msg->send_from_server(sender, string(ERR_NEEDMOREPARAMS) + " " + sender->get_nickname() + " " + mode + ":");
			return ("");
		}
	}
	
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
			if ((mode_stat && _password.empty()))
			{
				_password = temp;
				send_to_all_in_channel(mode_message + " +k :" + _password);
			}
			else if (!mode_stat && _password == temp)
			{
				_password.clear();
				send_to_all_in_channel(mode_message + " -k :" + _password);
			}
			break ;

		case 'o':
			if (!client_in_channel(server->get_client(temp)))
				msg->send_from_server(sender, string(ERR_NOSUCHNICK) + " " + sender->get_nickname() + " " + temp + " :No such nick");
			else if (mode_stat && !is_operator(temp))
			{
					add_operator(&server->get_client(temp));
					send_to_all_in_channel(mode_message + " +o :" + temp);
			}
			else if (is_operator(temp) && !mode_stat)
			{
					remove_operator(&server->get_client(temp));
					send_to_all_in_channel(mode_message + " -o :" + temp);
			}
			break ;

		case 'l':
			if (mode_stat)
			{
				try {
    				int limit = std::stoi(temp);
   					_userlimit = (limit > 9999) ? 9999 : limit;
				} catch (const std::invalid_argument& e) {
				    break;
				} catch (const std::out_of_range& e) {
   					_userlimit = 9999;
				}
				send_to_all_in_channel(mode_message + get_channel_name() + " +l :" + std::to_string(_userlimit));
			}
			else
			{
				_userlimit = 9999;
        		send_to_all_in_channel(mode_message + get_channel_name() + " :-l");
			}
			break ;
	}
	return ("");
}

string Channel::get_modes()
{
	string modes = "+";

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
