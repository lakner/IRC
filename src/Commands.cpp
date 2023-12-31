#include "Commands.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Numeric.hpp"
#include <vector>
#include <algorithm>

using std::string;

Commands::Commands()
{

}

Commands::~Commands()
{

}

int	Commands::execute(Server *server, Message *msg)
{
	std::cout << "execute: COMMAND:" << msg->get_command() << std::endl;
	string command = msg->get_command();
	if (command == "CAP")							// change to switch case
		return (exec_cap(msg));
	else if (command == "PING")
		return (exec_ping(msg));
	else if (command == "PASS")
		return (exec_pass(server, msg));
	else if (command == "NICK" && msg->get_sender()->is_authd())
		return (exec_nick(msg, server));
	else if (command == "USER" && msg->get_sender()->is_authd() == 1)
		return (exec_user(msg));
	if (msg->get_sender()->is_authd() != 2)
		return (-1);
	else if (command == "PRIVMSG")
		return(exec_privmsg(server, msg));
	else if (command == "JOIN")
		return(exec_join(server, msg));
	else if ((command == "INVITE" || command == "invite"))
		return(exec_invite(server, msg));
	else if (command == "KICK")
		return(exec_kick(server, msg));
	else if (command == "TOPIC")
		return(exec_topic(server, msg));
	else if (command == "WHO")
		return(exec_who(server, msg));
	else if (command == "MODE")
		return (exec_mode(server, msg));
	return (-1);
}

int Commands::exec_mode(Server *server, Message *msg)
{
	string				channel_name, modes, modes_set;
	std::stringstream	ss(msg->get_payload());
	bool				mode_state = 1; // 1(+/set) 0(-/unset)
	Client				*sender = msg->get_sender();
	string				sender_nick = sender->get_nickname();

	ss >> channel_name >> modes;

	if(!server->channel_exists(channel_name))
		return (msg->send_from_server(sender, string(ERR_NOSUCHCHANNEL) + " " + sender_nick + " " + channel_name + " :No such channel"));

	Channel &ch = server->get_channel(channel_name);
	if (modes.empty())
	{
		std::cout << "response" << std::endl;
		string response = string(RPL_CHANNELMODEIS) + " " + sender_nick + " " + channel_name + " :" + ch.get_modes();
		if (!ch.get_password().empty())
			response += " " + ch.get_password();
		if (ch.get_userlimit() != 9999)
		{
			std::ostringstream ss_userlimit;
			ss_userlimit << ch.get_userlimit();
			response += " " + ss_userlimit.str();
		}
		return(msg->send_from_server(sender, response));
	}
	else if (modes == "b")
		return(msg->send_from_server(sender, string(RPL_ENDOFBANLIST) + " " + sender_nick + " "
				+ channel_name + " :End of channel ban list"));
	else if (!ch.is_operator(sender_nick))
		return(msg->send_from_server(sender, string(ERR_CHANOPRIVSNEEDED) + " " + sender_nick + " " + channel_name + " :You must be a channel half-operator"));
	for (string::size_type i = 0; i < modes.length(); i++)
	{
		if (modes[i] == '+')
		{
			if (mode_state == 0 || i == 0)
				modes_set += "+";
			if (modes_set.size() == 2)
				modes_set = "+";
			mode_state = 1;
			continue ;
		}
		if ( modes[i] == '-')
		{
			if (mode_state == 1 || i == 0)
				modes_set += "-";
			if (modes_set.size() == 2)
				modes_set = "-";
			mode_state = 0;
			continue ;
		}
		modes_set += ch.set_mode(modes[i], mode_state, &ss, server, msg);
	}
	if (modes_set.size() > 1)
		ch.send_to_all_in_channel(":" + sender->get_full_client_identifier() + " MODE " + channel_name + " :" + modes_set);
	return (0);
}

int Commands::exec_who(Server *server, Message *msg)
{
	string				channel_name;
	Client*				sender = msg->get_sender();
	string 				response = sender->get_server_string();
	std::stringstream	ss(msg->get_payload());
	
	ss >> channel_name;

	if (channel_name.empty())
	{
		response += sender->get_server_string() + " " + ERR_NEEDMOREPARAMS;
		return (msg->send_to(sender, response));
	}
	else if(!server->channel_exists(channel_name))
	{
		response += sender->get_server_string() + " " + ERR_NOSUCHCHANNEL;
		return (msg->send_to(sender, response));
	}

	Channel& ch = server->get_channel(channel_name);

	response += string(RPL_WHOREPLY) + " " + channel_name;
	std::map<string, Client*> users = ch.get_users();
	std::map<string, Client*>::iterator it = users.begin();
	for (; it != users.end(); it++)
	{
		response += " ";
		if (ch.is_operator(it->first))
			response += "@";
		response += it->first;
	}
	response += " :1";
	msg->send_to(sender, response);
	response = sender->get_server_string() + " " + RPL_ENDOFWHO + " " + sender->get_nickname();
	response += " " + channel_name + " :End of WHO list";
	msg->send_to(sender, response);
	return 0;
}

int	Commands::exec_topic(Server *server, Message *msg)
{
	string			payload = msg->get_payload();
	std::stringstream	ss (payload);
	string			channel_name;
	string			new_channel_topic = "";
	Client			*sender = msg->get_sender();
	string			response = "";

	ss >> channel_name;
	
	if (payload.find(":") != string::npos)
		new_channel_topic = payload.substr(payload.find(":") + 1); 

	// we don't need a topic, but the channel name needs to be defined
	if (channel_name.empty())
	{
		response += sender->get_server_string() + " " + ERR_NEEDMOREPARAMS;
		return (msg->send_to(sender, response));
	}
	// channel needs to exist
	else if(!server->channel_exists(channel_name))
	{
		response += sender->get_server_string() + " " + ERR_NOSUCHCHANNEL + " " + sender->get_nickname() + " " + channel_name + " :No such channel";
		return (msg->send_to(sender, response));
	}
	
	Channel& ch = server->get_channel(channel_name);
	if (!ch.client_in_channel(*sender))		// client needs to be in channel
		response += sender->get_server_string() + " " + string(ERR_NOTONCHANNEL) + " " + ch.get_channel_name() + " :You're not on that channel";
	else if (!ch.allowed_to_set_topic(sender->get_nickname()))
		response += sender->get_server_string() + " " + string(ERR_CHANOPRIVSNEEDED) + " " + sender->get_nickname() + " " + ch.get_channel_name() + " :You must be a channel half-operator";
	else if (new_channel_topic.empty())		// no second parameter or it does not start with ':' - return the topic
	{	
		response += sender->get_server_string() + " " + string(RPL_TOPIC) + " " + sender->get_nickname() + " " ;
		response += ch.get_channel_name();
		if (ch.get_topic() == "")
			response +=" :No topic is set."; 
		else
			response += " :" + ch.get_topic(); 
	}
	else 
	{
		ch.set_topic(new_channel_topic);
		response = ":" + sender->get_full_client_identifier() + " " + msg->get_raw_content();
	}
	return (msg->send_to(sender, response));		// we only really need to return an error if send fails
}

int	Commands::exec_kick(Server *server, Message *msg)
{
	std::stringstream	ss (msg->get_payload());
	Client				*sender = msg->get_sender();
	string				sender_nick = sender->get_nickname();
	string				channel_name, nick_to_kick, reason = "";
	string				response = sender->get_server_string() + " ";

	ss >> channel_name >> nick_to_kick;
	if (channel_name == nick_to_kick)
	{
		if (msg->get_payload().find(":") != string::npos)
			nick_to_kick = msg->get_payload().substr(msg->get_payload().find(":") + 1);
	}
	else
	{
		if (msg->get_payload().find(":") != string::npos)
			reason = msg->get_payload().substr(msg->get_payload().find(":"));
	}
	if (nick_to_kick.empty() || channel_name.empty())
	{
		response += string(ERR_NEEDMOREPARAMS) + " " + sender_nick + " ";
		response += channel_name + " :No channel or user specified";
		return (msg->send_to(sender, response));
	}
	else if(!server->channel_exists(channel_name))
	{
		response += string(ERR_NOSUCHCHANNEL) + " " + sender_nick + " " + channel_name + " :No such channel";
		return (msg->send_to(sender, response));
	}

	Channel &ch = server->get_channel(channel_name);
	std::cout << "KICK: nick_to_kick: " << nick_to_kick << std::endl; 
	if (!server->nickname_exists(nick_to_kick))
	{
		response += string(ERR_NOSUCHNICK) + " " + sender_nick + " ";
		response += nick_to_kick + " " + channel_name + " :No such nick";
	}
	else if (!ch.client_in_channel(server->get_client(nick_to_kick)))
	{
		response += string(ERR_USERNOTINCHANNEL) + " " + sender_nick + " ";
		response += nick_to_kick + " " + channel_name + " :They are not on that channel";
	}
	else if (!ch.client_in_channel(*sender))
	{
		response += string(ERR_NOTONCHANNEL) + " " + sender_nick + " ";
		response += nick_to_kick + " " + channel_name + " :You're not on that channel";
	}
	else if (!ch.is_operator(sender_nick))
	{
		response += string(ERR_CHANOPRIVSNEEDED) + " " + sender_nick;
		response += " " + channel_name + " :You must be a channel half-operator";
	}
	else
	{
		response = ":" + sender->get_full_client_identifier() + " KICK " + channel_name;
		if (reason.empty())
			response += " " + nick_to_kick + " :" + sender_nick;
		else
			response += " " + nick_to_kick + " " + reason;
		ch.send_to_all_in_channel(response);
		ch.kick(nick_to_kick);
		return (0);
	}
	return (msg->send_to(sender, response));
}


int	Commands::exec_invite(Server *server, Message *msg)
{
	std::stringstream	ss (msg->get_payload());
	string 				channel_name, nickname, extra_arguments;
	Client				*sender = msg->get_sender();
	string				response = sender->get_server_string() + " ";
	string				sender_nick = sender->get_nickname();
	Client& 			client = *(msg->get_sender());
	int					channel_exist = 1; //the channel existed before the invitation

	ss >> nickname >> channel_name >> extra_arguments;
	if (!extra_arguments.empty())
	{
		response += "NOTICE " + sender_nick + " :*** Invalid duration for invite";
		return (msg->send_to(&client, response));
	}
	if (nickname.empty() || channel_name.empty())
	{
		response += string(ERR_NEEDMOREPARAMS) + " " + sender_nick + " ";
		response += channel_name + " :No channel or user specified: ";
		return (msg->send_to(&client, response));
	}
	else if(!server->channel_exists(channel_name))
	{
		if (is_valid_channel_name(channel_name))
		{
			server->add_channel(channel_name, "");
			channel_exist = 0;
		}
	}

	Channel &ch = server->get_channel(channel_name);
	std::cout << "INVITE: nick_to_invite: " << nickname << std::endl;
	if (!server->nickname_exists(nickname))
	{
		response += string(ERR_NOSUCHNICK) + " " + sender_nick + " ";
		response += nickname + " " + channel_name + " :No such nick";
	}
	else if (ch.client_in_channel((server->get_client(nickname))))
	{
		response += string(ERR_USERONCHANNEL) + " " + sender_nick + " " + nickname;
		response += " " + channel_name + " :is already in channel";
	}
	else if (!ch.client_in_channel(*sender) && channel_exist)
	{
		response += string(ERR_NOTONCHANNEL) + " " + sender_nick + " " + nickname;
		response += " " + channel_name + " :You're not on that channel";
	}
	else if (ch.get_invite_only() && !ch.is_operator(sender_nick))
	{
		response += string(ERR_CHANOPRIVSNEEDED) + " " + sender_nick;
		response += " " + channel_name + " :You must be a channel half-operator";
	}
	else
	{
		response += string(RPL_INVITING) + " " + sender_nick + " " + nickname + " :" + channel_name;
		//ch.invite(&(server->get_client(nickname)));
		msg->send_to(&(server->get_client(nickname)), ":" + client.get_full_client_identifier() \
														+ " INVITE " + nickname + " :" + channel_name);
		ch.add_invited(&server->get_client(nickname));
	}
	return (msg->send_to(sender, response));
}

int	Commands::exec_ping(Message *msg)
{
	msg->send_to(msg->get_sender(), "PONG " + msg->get_payload());
	return 0;
}

int	Commands::exec_cap(Message *msg)
{
	Client	*sender = msg->get_sender();
	
	msg->send_from_server(sender, "CAP * LS :account-notify account-tag away-notify batch cap-notify chghost draft/relaymsg echo-message extended-join inspircd.org/poison inspircd.org/standard-replies invite-notify labeled-response message-tags multi-prefix sasl server-time setname userhost-in-names ");
	return 0;
}

int Commands::exec_pass(Server *server, Message *msg)
{
	Client		*sender = msg->get_sender();
	string		payload = msg->get_payload();

	payload.erase(0, payload.find_first_not_of(": "));
	std::cout << "exec_pass: " << "payload: " << payload << " server->get_pass(): " << server->get_pass() << std::endl;
	if (payload == server->get_pass() && !sender->is_authd())
	{
		std::cout << "exec_pass: authenticating." << std::endl;
		sender->authenticate(1);
		return 0;
	}
	else if (payload.empty())
	{
		msg->send_to(sender, sender->get_server_string() + string(ERR_NEEDMOREPARAMS));
		return 0;
	}
	else if (sender->is_authd() != 0)
	{
		msg->send_to(sender, sender->get_server_string() + string(ERR_ALREADYREGISTRED) + " " + msg->get_sender()->get_nickname() + " :You may not reregister");
		return 0;
	}
	return 1;
}


bool Commands::is_valid_channel_name(string name)
{
	if (name.length() > 50 || !name.length())
		return false;
	if (string("&#+!").find(name[0]) == string::npos)
		return false;
	if (name.find_first_of(" \a,\r\n") != string::npos)
		return false;
	return true;
}

bool Commands::is_valid_nickname(string name)
{
	if (name.length() > 32 || !name.length())
		return false;
	if (isdigit(name[0]))
		return false;
	string valid = "<-[]\\^{}_|";
	valid += "abdcefghijklmnopqrstuvwxyz";
	valid += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	valid += "0123456789";
	if (name.find_first_not_of(valid) != string::npos)
		return false;
	return true;
}

bool Commands::is_valid_username(string name)
{
	if (name.length() > 32 || !name.length())
		return false;
	if (name.find_first_of("\r\n @") != string::npos)
		return false;
	return true;
}

int Commands::exec_nick(Message *msg, Server *serv)
{
	string		n_name = msg->get_payload();
	string		response;
	string 		sender = msg->get_sender()->get_nickname();
	std::cout << "NICK: '" << n_name << "'" << std::endl;

	if (n_name.empty())
		response = " " + string(ERR_NONICKNAMEGIVEN) + " " + sender + " :No nickname given";
	else if(!is_valid_nickname(n_name))
		response = " " + string(ERR_ERRONEUSNICKNAME) + " " + sender + " " + n_name + " :Erronous nickname";
	else if (serv->nickname_exists(n_name))
		response = " " + string(ERR_NICKNAMEINUSE) + " " + sender + " " + n_name + " :Nickname is already in use.";
	else
	{
		response = ":" + msg->get_sender()->get_full_client_identifier() + " NICK :" + n_name;
		msg->get_sender()->set_nickname(n_name);
		msg->get_sender()->send_to_all_in_my_channels(serv, response);
		msg->send_to(msg->get_sender(), response); // send to all channelmembers of all channels where sender is in
		
		return 0;
	}
	msg->send_from_server(msg->get_sender(), response);
	return 0;
}

const char* Commands::UsernameAlreadyExists::what() const throw()
{
	return ("Username already exist!\n");
}

int Commands::exec_user(Message *msg)
{
	Client *sender = msg->get_sender();

	if (sender->get_nickname().empty())
		return -1;
	if (!sender->get_username().empty())
	{
		msg->send_to(sender, sender->get_server_string() + string(ERR_ALREADYREGISTRED) + " " + sender->get_nickname() + " :You may not reregister");
		return (atoi(ERR_ALREADYREGISTRED));
	}

	std::stringstream ss (msg->get_payload());
	string username, unused, realname;

	ss >> username >> unused >> unused >> realname;
	std::cout << "USER: username: " << username << " unused: " << unused << " realname: " << realname << std::endl;
	sender->set_username(username);

	msg->get_sender()->authenticate(2);
	string response = ":127.0.0.1 " + string(RPL_WELCOME) + " " + msg->get_sender()->get_nickname();
	response += " :Welcome to the Internet Relay Network ";
	response += sender->get_full_client_identifier();
	msg->send_to(sender, response);
	return 0;
}

// JOIN <channels> <passwords>
// both channels and passwords can be a comma-separated list
// instead of just one channel/ one password, but I don't know if we really need to implement that?

//add JOIN 0
int Commands::exec_join(Server *server, Message *msg)
{
	std::cout << "JOIN: Payload is: " << msg->get_payload() << std::endl;
	
	std::stringstream	ss(msg->get_payload());
	string 				s_channels, s_passwords;
	string 				error;
	Client				*sender = msg->get_sender();

	ss >> s_channels >> s_passwords;
	std::cout << "JOIN: channels: " << s_channels << "\n\tPasswords are: " << s_passwords << std::endl;

	std::vector<string> vchannels, vpasswords;
	
	std::stringstream channelstr(s_channels);
	while (channelstr.good())
	{
		string ch;
		std::getline(channelstr, ch, ',');
		if (!is_valid_channel_name(ch))
		{
			msg->send_from_server(sender, " " + string(ERR_NOSUCHCHANNEL) + " " + ch + " :No such channel");
			return 0;
		}
		vchannels.push_back(ch);
	}
	std::stringstream passstr(s_passwords);
	while (passstr.good())
	{
		string pass;
		std::getline(passstr, pass, ',');
		vpasswords.push_back(pass);
	}

	std::map<string, Channel>& channels = server->get_channels();
	for (unsigned int i = 0; i < vchannels.size(); i++)
	{
		string ret;
		// new channel
		if (channels.find(vchannels[i]) == channels.end())
		{
			if (i < vpasswords.size()) // create channel with password
				server->add_channel(vchannels[i], vpasswords[i]);
			else // no password provided
				server->add_channel(vchannels[i], "");
		}

		// join the channel
		if (channels[vchannels[i]].client_in_channel(*sender))
		{
			error = std::string(ERR_USERONCHANNEL) + " " + sender->get_nickname();
			error += " " + sender->get_nickname() + " " + vchannels[i] + " :is already in channel";
			msg->send_from_server(sender, error);
			return 0;
		}
		if (i < vpasswords.size() && !channels[vchannels[i]].get_invite_only())
			ret = channels[vchannels[i]].add_user(sender, vpasswords[i]);
		else if (!channels[vchannels[i]].get_invite_only() || channels[vchannels[i]].is_invited(sender->get_nickname()))
		{
			if (channels[vchannels[i]].is_invited(sender->get_nickname()))
				channels[vchannels[i]].remove_invited(sender);
			ret = channels[vchannels[i]].add_user(sender, "");
		}
		else
			ret = ERR_INVITEONLYCHAN;

		if (!ret.empty())
		{
			error = ret + " " + sender->get_nickname() + " " + vchannels[i] + " ";
			if (ret == ERR_BADCHANNELKEY)
				error += ":Cannot join channel (incorrect channel key)";
			if (ret == ERR_INVITEONLYCHAN)
				error += ":Cannot join channel (invite only)";
			if (ret == ERR_CHANNELISFULL)
				error += ":Cannot join channel (channel is full)";
			msg->send_from_server(sender, ret + " " + msg->get_sender()->get_nickname() + " " + vchannels[i] + " " + error);
		}
		else
		{
		//	msg->send_from_server(msg->get_sender(), "JOIN: Successfully joined channel " + vchannels[i] + "\n");
			std::cout << sender->get_nickname() << " joined channel " << vchannels[i] << std::endl;
		}
	}
	return (0);
}

int	Commands::exec_privmsg(Server *server, Message *msg)
{
	string			s_recipient;
	std::stringstream 	ss(msg->get_raw_content());
	string 		payload;
	Client				*sender = msg->get_sender();
	ss >> s_recipient >> s_recipient; // Read the second word (skipping leading whitespace)

	if (msg->get_raw_content().find(":") == string::npos)
	{
		msg->send_from_server(sender, "PRIVMSG: message not found.\n");
		return -1;
	}

	payload = msg->get_raw_content().substr(msg->get_raw_content().find(":"), msg->get_raw_content().size());
	std::cout << "PRIVMSG: Payload is: " << payload << std::endl;
	if (server->nickname_exists(s_recipient))
	{
		Client& recpnt = server->get_client(s_recipient);
		std::cout << "PRIVMSG: Recipient is: "<< s_recipient << std::endl;
		string message = ":" + msg->get_sender()->get_full_client_identifier() + " ";
		message += msg->get_raw_content();
		msg->send_to(&recpnt, message);
	}
	else if(server->channel_exists(s_recipient))
	{
		std::cout << "PRIVMSG: sending to channel '" << s_recipient << "'." << std::endl;
		Channel &channel = server->get_channel(s_recipient);
		string msg_content = ":" + msg->get_sender()->get_full_client_identifier();
		msg_content += " PRIVMSG " + s_recipient + " " + payload;
		channel.send_to_all_except(msg_content, *(msg->get_sender()));
	}
	return 0;
}
