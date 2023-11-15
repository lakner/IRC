#include "Commands.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Numeric.hpp"
#include <vector>
#include <algorithm>


Commands::Commands()
{

}

Commands::~Commands()
{

}

int	Commands::execute(Server *server, Message *msg)
{
	std::cout << "execute: COMMAND:" << msg->get_command() << std::endl;
	std::string command = msg->get_command();
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
	if (!msg->get_sender()->is_authd())
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
	return (-1);
}

int Commands::exec_who(Server *server, Message *msg)
{
	std::string			channel_name;
	std::string 		response = std::string(HOSTNAME);
	std::stringstream	ss(msg->get_payload());
	
	ss >> channel_name;

	if (channel_name.empty())
	{
		response += std::string(HOSTNAME) + " " + ERR_NEEDMOREPARAMS;
		return (msg->send_to(msg->get_sender(), response));
	}
	else if(!server->channel_exists(channel_name))
	{
		response += std::string(HOSTNAME) + " " + ERR_NOSUCHCHANNEL;
		return (msg->send_to(msg->get_sender(), response));
	}

	Channel& ch = server->get_channel(channel_name);

	response += std::string(RPL_WHOREPLY) + " " + channel_name;
	std::map<std::string, Client*> users = ch.get_users();
	std::map<std::string, Client*>::iterator it = users.begin();
	for (; it != users.end(); it++)
	{
		response += " ";
		if (ch.is_operator(it->first))
			response += "@";
		response += it->first;
	}
	response += " :1";
	msg->send_to(msg->get_sender(), response);
	response = std::string(HOSTNAME) + " " + RPL_ENDOFWHO + msg->get_sender()->get_nickname();
	response += " " + channel_name + " :End of WHO list";
	msg->send_to(msg->get_sender(), response);
	return 0;
}

int	Commands::exec_topic(Server *server, Message *msg)
{
	std::string			payload = msg->get_payload();
	std::stringstream	ss (payload);
	std::string			channel_name;
	std::string			new_channel_topic = "";
	Client&				client = *(msg->get_sender());
	std::string 		response = "";

	ss >> channel_name;
	
	
	// topic can contain spaces, we can't get it with stringstream
	// we'll just get the stuff after the first ':'
	if (payload.find(":") != std::string::npos)
		new_channel_topic = payload.substr(payload.find(":") + 1); 

	// we don't need a topic, but the channel name needs to be defined
	if (channel_name.empty())
	{
		response += std::string(HOSTNAME) + " " + ERR_NEEDMOREPARAMS;
		return (msg->send_to(msg->get_sender(), response));
	}
	// channel needs to exist
	else if(!server->channel_exists(channel_name))
	{
		response += std::string(HOSTNAME) + " " + ERR_NOSUCHCHANNEL;
		return (msg->send_to(msg->get_sender(), response));
	}

	Channel& ch = server->get_channel(channel_name);
	if (!ch.client_in_channel(client))		// client needs to be in channel
		response += std::string(HOSTNAME) + " " + std::string(ERR_NOTONCHANNEL) + " " + ch.get_channel_name() + " :You're not on that channel";
	else if (!ch.allowed_to_set_topic(client.get_nickname()))
		response += std::string(HOSTNAME) + " " + std::string(ERR_CHANOPRIVSNEEDED) + " " + ch.get_channel_name() + " :You're not a channel operator";
	else if (new_channel_topic.empty())		// no second parameter or it does not start with ':' - return the topic
	{	
		response += std::string(HOSTNAME) + " " + std::string(RPL_TOPIC) + " " + msg->get_sender()->get_nickname() + " " ;
		response += ch.get_channel_name();
		if (ch.get_topic() == "")
			response +=" :No topic is set."; 
		else
			response += " :" + ch.get_topic(); 
	}
	else 
	{
		ch.set_topic(new_channel_topic);
		response = ":" + msg->get_sender()->get_full_client_identifier() + " " + msg->get_raw_content();
	}
	return (msg->send_to(msg->get_sender(), response));		// we only really need to return an error if send fails
}

int	Commands::exec_kick(Server *server, Message *msg)
{
	std::stringstream	ss (msg->get_payload());
	std::string			sender_nick = msg->get_sender()->get_nickname();
	std::string			channel_name, nick_to_kick, reason = "";
	std::string			response = std::string(HOSTNAME) + " ";
	Client& client = *(msg->get_sender());

	ss >> channel_name >> nick_to_kick;
	if (channel_name == nick_to_kick)
	{
		if (msg->get_payload().find(":") != std::string::npos)
			nick_to_kick = msg->get_payload().substr(msg->get_payload().find(":") + 1);
	}
	else
	{
		if (msg->get_payload().find(":") != std::string::npos)
			reason = msg->get_payload().substr(msg->get_payload().find(":"));
	}
	if (nick_to_kick.empty() || channel_name.empty())
	{
		response += std::string(ERR_NEEDMOREPARAMS) + " " + sender_nick + " ";
		response += channel_name + " :No channel or user specified";
		return (msg->send_to(&client, response));
	}
	else if(!server->channel_exists(channel_name))
	{
		response += std::string(ERR_NOSUCHCHANNEL) + " " + sender_nick + " " + channel_name + " :No such channel";
		return (msg->send_to(&client, response));
	}

	Channel &ch = server->get_channel(channel_name);
	std::cout << "KICK: nick_to_kick: " << nick_to_kick << std::endl; 
	if (!server->nickname_exists(nick_to_kick))
	{
		response += std::string(ERR_NOSUCHNICK) + " " + sender_nick + " ";
		response += nick_to_kick + " " + channel_name + " :No such nick";
	}
	else if (!ch.client_in_channel(server->get_client(nick_to_kick)))
	{
		response += std::string(ERR_USERNOTINCHANNEL) + " " + sender_nick + " ";
		response += nick_to_kick + " " + channel_name + " :They aren't on that channel";
		// msg->send_to(&client, std::string(HOSTNAME) + std::string(ERR_NOTONCHANNEL));
		// return (442);
	}
	else if (!ch.allowed_to_invite_kick(client.get_nickname()))
	{
		response += std::string(ERR_CHANOPRIVSNEEDED) + " " + sender_nick + " " + nick_to_kick;
		response += " " + channel_name + " :You're not a channel operator";
		//msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_CHANOPRIVSNEEDED));
	}
	else // we can kick the user
	{
		response = ":" + client.get_full_client_identifier() + " KICK " + channel_name;
		if (reason.empty())
			response += " " + nick_to_kick + " :" + sender_nick;
		else
			response += " " + nick_to_kick + " " + reason;
		msg->send_to(&(server->get_client(nick_to_kick)), response);
		ch.kick(nick_to_kick);
	}
	return (msg->send_to(&client, response));
}

int	Commands::exec_invite(Server *server, Message *msg)
{
	std::stringstream ss (msg->get_payload());
	std::string channel_name, nickname;
	Client& client = *(msg->get_sender());

	ss >> nickname >> channel_name;
	if (nickname.empty() || channel_name.empty())
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NEEDMOREPARAMS));
		return (461);
	}

	if(!server->channel_exists(channel_name))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHCHANNEL));
		return (403);
	}

	Channel &ch = server->get_channel(channel_name);
	if (!ch.client_in_channel(client))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOTONCHANNEL));
		return (442);
	}

	if (!ch.allowed_to_invite_kick(client.get_nickname()))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_CHANOPRIVSNEEDED));
		return (482);
	}

	if (!server->nickname_exists(nickname))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHNICK));
		return (401);
	}
	
	Client& invited = server->get_client(nickname);

	if (ch.client_in_channel(invited))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_USERONCHANNEL));
		return (443);
	}

	invite(server, channel_name, nickname);
	return (0);
}

int	Commands::exec_ping(Message *msg)
{
	msg->send_to(msg->get_sender(), "PONG " + msg->get_payload());
	return 0;
}

int	Commands::exec_cap(Message *msg)
{
	// we ignore CAP messages
	msg->send_to(msg->get_sender(), std::string(HOSTNAME) + ":irc.unknown.net CAP * LS :account-notify account-tag away-notify batch cap-notify chghost draft/relaymsg echo-message extended-join inspircd.org/poison inspircd.org/standard-replies invite-notify labeled-response message-tags multi-prefix sasl server-time setname userhost-in-names ");
	return 0;
}

int Commands::exec_pass(Server *server, Message *msg)
{
	std::string payload = msg->get_payload();
	payload.erase(0, payload.find_first_not_of(": "));
	std::cout << "exec_pass: " << "payload: " << payload << " server->get_pass(): " << server->get_pass() << std::endl;
	if (payload == server->get_pass() && !msg->get_sender()->is_authd())
	{
		std::cout << "exec_pass: authenticating." << std::endl;
		msg->get_sender()->authenticate(1);
		return 0;
	}
	else if (payload.empty())
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NEEDMOREPARAMS));
		return 0;
	}
	else if (msg->get_sender()->is_authd() != 0)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_ALREADYREGISTRED) + " " + msg->get_sender()->get_nickname() + " :You may not reregister");
		return 0;
	}
	return 1;
}

//need to implement 
bool is_valid_nickname(std::string name)
{
	// check for valid Characters 
	if (name == name)
		return true;
	return true;
}

int Commands::exec_nick(Message *msg, Server *serv)
{
	std::string n_name = msg->get_payload();
	if(!is_valid_nickname(n_name))
	{
		msg->send_to(msg->get_sender(), ERR_ERRONEUSNICKNAME);
		return (atoi(ERR_ERRONEUSNICKNAME));
	}
	else if (serv->nickname_exists(n_name))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + ERR_NICKNAMEINUSE + " " + msg->get_sender()->get_nickname() + " " + n_name + " :Nickname is already in use.");
		return (atoi(ERR_NICKNAMEINUSE));
	}
	else if (n_name.empty())
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + ERR_NONICKNAMEGIVEN + " " + msg->get_sender()->get_nickname() + " :No nickname given");
		return (atoi(ERR_NONICKNAMEGIVEN));
	}
	else
	{
		std::string to_send = ":" + msg->get_sender()->get_full_client_identifier() + " NICK :" + n_name;
		msg->send_to(msg->get_sender(), to_send);
		msg->get_sender()->set_nickname(n_name);
		return 0;
	}
	return 1;
}

const char* Commands::UsernameAlreadyExists::what() const throw()
{
	return ("Username already exist!\n");
}

int Commands::exec_user(Message *msg)
{
	if (msg->get_sender()->get_nickname().empty())
		return -1;
	if (!msg->get_sender()->get_username().empty())
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_ALREADYREGISTRED) + " " + msg->get_sender()->get_nickname() + " :You may not reregister");
		return (atoi(ERR_ALREADYREGISTRED));
	}
	std::stringstream ss (msg->get_payload());
	std::string username, unused, realname;
	Client *client = msg->get_sender();

	ss >> username >> unused >> unused >> realname;
	std::cout << "USER: username: " << username << " unused: " << unused << " realname: " << realname << std::endl;
	client->set_username(username);

	msg->get_sender()->authenticate(2);
	std::string response = ":127.0.0.1 " + std::string(RPL_WELCOME) + " " + msg->get_sender()->get_nickname();
	response += " :Welcome to the Internet Relay Network ";
	response += client->get_full_client_identifier();
	msg->send_to(client, response);
	return 0;
}

// JOIN <channels> <passwords>
// both channels and passwords can be a comma-separated list
// instead of just one channel/ one password, but I don't know if we really need to implement that?

//add JOIN 0
int Commands::exec_join(Server *server, Message *msg)
{
	std::cout << "JOIN: Payload is: " << msg->get_payload() << std::endl;
	
	std::stringstream ss(msg->get_payload());
	std::string s_channels, s_passwords;

	ss >> s_channels >> s_passwords;
	std::cout << "JOIN: channels: " << s_channels << "\n\tPasswords are: " << s_passwords << std::endl;

	std::vector<std::string> vchannels, vpasswords;
	
	std::stringstream channelstr(s_channels);
	while (channelstr.good())
	{
		std::string ch;
		std::getline(channelstr, ch, ',');
		if (!(ch.rfind("#", 0) == 0))
		{
			msg->send_to(msg->get_sender(), "Channel names must start with '#'");
			return -1;
		}
		vchannels.push_back(ch);
	}
	std::stringstream passstr(s_passwords);
	while (passstr.good())
	{
		std::string pass;
		std::getline(passstr, pass, ',');
		vpasswords.push_back(pass);
	}

	std::map<std::string, Channel>& channels = server->get_channels();
	
	for (unsigned int i = 0; i < vchannels.size(); i++)
	{
		std::string ret;
		// new channel
		if (channels.find(vchannels[i]) == channels.end())
		{
			if (i < vpasswords.size()) // create channel with password
				server->add_channel(vchannels[i], vpasswords[i]);
			else // no password provided
				server->add_channel(vchannels[i], "");
		}

		// join the channel
		if (i < vpasswords.size())
			ret = channels[vchannels[i]].add_user(msg->get_sender(), vpasswords[i]);
		else
			ret = channels[vchannels[i]].add_user(msg->get_sender(), "");

		if (!ret.empty())
		{
			std::string error = ret + " " + msg->get_sender()->get_nickname() + " " + vchannels[i] + " ";
			if (ret == ERR_BADCHANNELKEY)
				error += ":Cannot join channel (incorrect channel key)";
			if (ret == ERR_INVITEONLYCHAN)
				error += ":Cannot join channel (invite only)";
			if (ret == ERR_CHANNELISFULL)
				error += ":Cannot join channel (channel is full)";
			msg->send_from_server(msg->get_sender(), ret + " " + msg->get_sender()->get_nickname() + " " + vchannels[i] + " ");
		}
		else
		{
		//	msg->send_from_server(msg->get_sender(), "JOIN: Successfully joined channel " + vchannels[i] + "\n");
			std::cout << msg->get_sender()->get_nickname() << " joined channel " << vchannels[i] << std::endl;
		}
	}
	return (0);
}

int	Commands::exec_privmsg(Server *server, Message *msg)
{
	std::string	s_recipient;
	std::stringstream ss(msg->get_raw_content());
	std::string payload;
	ss >> s_recipient >> s_recipient; // Read the second word (skipping leading whitespace)

	if (msg->get_raw_content().find(":") == std::string::npos)
	{
		msg->send_from_server(msg->get_sender(), "PRIVMSG: message not found.\n");
		return -1;
	}

	payload = msg->get_raw_content().substr(msg->get_raw_content().find(":"), msg->get_raw_content().size());
	std::cout << "PRIVMSG: Payload is: " << payload << std::endl;
	if (server->nickname_exists(s_recipient))
	{
		Client& recpnt = server->get_client(s_recipient);
		std::cout << "PRIVMSG: Recipient is: "<< s_recipient << std::endl;
		std::string message = ":" + msg->get_sender()->get_full_client_identifier() + " ";
		message += msg->get_raw_content();
		msg->send_to(&recpnt, message);
	}
	else if(server->channel_exists(s_recipient))
	{
		std::cout << "PRIVMSG: sending to channel '" << s_recipient << "'." << std::endl;
		Channel &channel = server->get_channel(s_recipient);
		std::string msg_content = ":" + msg->get_sender()->get_full_client_identifier();
		msg_content += " PRIVMSG " + s_recipient + " " + payload;
		channel.send_to_all_except(msg_content, *(msg->get_sender()));
	}
	return 0;
}


//invite a user to a channel
void	Commands::invite(Server *server, std::string channel_name, std::string nickname)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		static std::map<const int, Client>& clientMap = server->get_clients();
		std::map<const int, Client>::iterator it = clientMap.begin();

		for (; it != clientMap.end(); it++) {
			Client *cl = &it->second;

			if (cl->get_nickname() == nickname)
				ch.add_user(cl, ch.get_password());
		}
	}
}
