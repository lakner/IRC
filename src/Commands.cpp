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
	std::cout << "MESSAGE: " << msg->get_command() << std::endl;
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
	else if (command == "PRIVMSG" && msg->get_sender()->is_authd())
	{
		return(exec_privmsg(server, msg));
	}
	else if (command == "JOIN" && msg->get_sender()->is_authd())
		return(exec_join(server, msg));
	else if (command == "INVITE" && msg->get_sender()->is_authd())
		return(exec_invite(server, msg));
	else if (command == "KICK" && msg->get_sender()->is_authd())
		return(exec_kick(server, msg));
	else if (command == "TOPIC" && msg->get_sender()->is_authd())
		return(exec_topic(server, msg)); //change
	else 
		return(server->send_to_all_clients(msg)); // maybe first send into the clients writebuffer
}

int	Commands::exec_topic(Server *server, Message *msg)
{
	std::stringstream ss (msg->get_payload());
	std::string channel_name, new_channel_topic;
	Client *client = msg->get_sender();
	(void)server;

	ss >> channel_name >> new_channel_topic;
	if (channel_name.empty())
	{
		msg->send_to(client, std::string(HOSTNAME) + std::string(ERR_NEEDMOREPARAMS));
		return (461);
	}

	int	status = client_belong_to_channel(server, channel_name, client->get_nickname());
	int	allow_change_topic = allow_to_set_topic(server, channel_name, client->get_nickname());

	if (status == 0)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOTONCHANNEL));
		return (442);
	}
	else if (status == -1)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHCHANNEL));
		return (403);
	}

	if (new_channel_topic.empty())
		msg->send_to(client, std::string(HOSTNAME) + get_channel_topic(server, channel_name)); //not sure yet what to return
	else
	{
		if (!allow_change_topic)
		{
			msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_CHANOPRIVSNEEDED));
			return (482);
		}
		else
		{
			if (new_channel_topic == ":")
			{
				clean_topic(server, channel_name);
				msg->send_to(msg->get_sender(), std::string(HOSTNAME) + "topic name has been cleaned\n");
			}
			else
			{
				change_topic(server, channel_name, new_channel_topic);
				msg->send_to(msg->get_sender(), std::string(HOSTNAME) + "topic name has been changed to: " + new_channel_topic);
			}
		}
	}
	return (0);
}

int	Commands::exec_kick(Server *server, Message *msg)
{
	std::stringstream ss (msg->get_payload());
	std::string channel_name, nickname;
	Client *client = msg->get_sender();

	ss >> nickname >> channel_name;
	if (nickname.empty() || channel_name.empty())
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NEEDMOREPARAMS));
		return (461);
	}

	int status = client_belong_to_channel(server, channel_name, client->get_nickname());
	if (status == 0)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOTONCHANNEL));
		return (442);
	}
	else if (status == -1)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHCHANNEL));
		return (403);
	}
	else if (status == -2)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHNICK));
		return (401);
	}

	if (!allow_to_invite(server, channel_name, client->get_nickname()))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_CHANOPRIVSNEEDED));
		return (482);
	}

	status = client_belong_to_channel(server, channel_name, nickname);
	if (status == -2)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHNICK));
		return (401);
	}
	else if (status == 0)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_USERNOTINCHANNEL));
		return (441);
	}

	kick(server, channel_name, nickname);
	return (0);
}

int	Commands::exec_invite(Server *server, Message *msg)
{
	std::stringstream ss (msg->get_payload());
	std::string channel_name, nickname;
	Client *client = msg->get_sender();

	ss >> nickname >> channel_name;
	if (nickname.empty() || channel_name.empty())
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NEEDMOREPARAMS));
		return (461);
	}

	int status = client_belong_to_channel(server, channel_name, client->get_nickname());
	if (status == 0)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOTONCHANNEL));
		return (442);
	}
	else if (status == -1)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHCHANNEL));
		return (403);
	}
	else if (status == -2)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHNICK));
		return (401);
	}

	if (!allow_to_invite(server, channel_name, client->get_nickname()))
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_CHANOPRIVSNEEDED));
		return (482);
	}

	status = client_belong_to_channel(server, channel_name, nickname);
	if (status == -2)
	{
		msg->send_to(msg->get_sender(), std::string(HOSTNAME) + std::string(ERR_NOSUCHNICK));
		return (401);
	}
	else if (status == 1)
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
	else if (nickname_exists(n_name, serv))
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
int Commands::exec_join(Server *server, Message *msg)
{
	(void) server;
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
	// std::cout << "JOIN channels: ";
	// // output the vector:
	// std::copy(vchannels.begin(), vchannels.end(), std::ostream_iterator<std::string>(std::cout, " "));
	// std::cout << std::endl;

	// TODO: Actually join the channels here
	std::map<std::string, Channel>& channels = server->get_channels();
	
	for (unsigned int i = 0; i < vchannels.size(); i++)
	{
		int ret;
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
		
		if (ret != 0)
			msg->send_from_server(msg->get_sender(), "JOIN: Error joining channel, wrong password?");
		else
		{
			msg->send_from_server(msg->get_sender(), "JOIN: Successfully joined channel " + vchannels[i] + "\n");
			std::cout << msg->get_sender() << " joined channel " << vchannels[i] << std::endl;
		}
	}
	return (0);
}

int Commands::nickname_exists(std::string name, Server *serv)
{
	std::map<const int, Client>& clientMap = serv->get_clients();
	std::map<const int, Client>::iterator it = clientMap.begin();

	for (; it != clientMap.end(); it++)
	{
		Client cl = it->second;

		if (cl.get_nickname() == name)
			return 1; // Nickname found
	}
	return 0; // Nickname not found
}

int	Commands::exec_privmsg(Server *server, Message *msg)
{
	std::string	s_recipient;
	std::stringstream ss(msg->get_raw_content());
	std::string payload;
	ss >> s_recipient >> s_recipient; // Read the second word (skipping leading whitespace)
	Client *recpnt = NULL;

	(void) server;

	std::map<const int, Client>::iterator it;
	for (it = Server::get_clients().begin(); it != Server::get_clients().end(); it++)
	{
		// Check for ":" in the recipient here and throw an error?
		Client* client = &(it->second);
		if (client->get_nickname() == s_recipient) 
		{
			recpnt = client;
			break; 
		}
	}

	if (!recpnt)
	{
		msg->send_from_server(msg->get_sender(), "PRIVMSG: recipient not found.\n");
		return -1;
	}
		// With stringstream we only get the first word, 
	// but we need the entire portion of the command following a ':'
	if (msg->get_raw_content().find(":") == std::string::npos)
	{
		msg->send_from_server(msg->get_sender(), "PRIVMSG: message not found.\n");
		return -1;
	}
	else
	{
		payload = msg->get_raw_content().substr(msg->get_raw_content().find(":") + 1, msg->get_raw_content().size());
		std::cout << "PRIVMSG: Recipient is: "<< s_recipient << " with _recpnt " << recpnt << std::endl;
		std::cout << "PRIVMSG: Payload is: " << payload << std::endl;
	}

	if (!msg->get_sender()->is_authd())
	{
		send(msg->get_sender()->get_client_fd(), "Client not authorized! Try connecting with: PASS password\r\n", 59, 0);
		return (0);
	}

	if (send(recpnt->get_client_fd(), (msg->get_sender())->get_nickname().c_str(), msg->get_sender()->get_nickname().size(), 0) == -1 
			|| send(recpnt->get_client_fd(), " client sent: '", 15, 0) == -1
			|| send(recpnt->get_client_fd(), (payload + std::string("'\r\n")).c_str(), payload.size() + 3, 0) == -1)
	{
		std::cout << "Error sending with send()." << std::endl;
		throw "Error sending.";
		return(-1);
	}
	return 0;
}

int	Commands::client_belong_to_channel(
		Server *server, std::string channel_name, std::string nickname)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		if (!nickname_exists(nickname, server))
			return (-2);
		std::map<std::string, Client*>::iterator it;

		for (it = ch.get_users().begin(); it != ch.get_users().end(); it++) {
			Client client = *it->second;
			if (client.get_nickname() == nickname)
				return 1; // Nickname found
		}
		return (0);
	}
	return (-1); //channel doesn't exist
}

int	Commands::allow_to_invite(
		Server *server, std::string channel_name, std::string nickname)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		if (!ch.get_mode().o)
			return (1);

		std::map<std::string, Client*>& operatorMap = ch.get_operators();
		std::map<std::string, Client*>::iterator it = operatorMap.begin();

		for (; it != operatorMap.end(); it++) {
			Client *client = it->second;

			if (client->get_nickname() == nickname)
				return (1); // Nickname found
		}
		return (0);
	}
	return (-1); //channel doesn't exist
}

int	Commands::allow_to_set_topic( //change
		Server *server, std::string channel_name, std::string nickname)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		if (!ch.get_mode().t)
			return (1);

		std::map<std::string, Client*>& operatorMap = ch.get_operators();
		std::map<std::string, Client*>::iterator it = operatorMap.begin();

		for (; it != operatorMap.end(); it++) {
			Client *client = it->second;

			if (client->get_nickname() == nickname)
				return (1); // Nickname found
		}
		return (0);
	}
	return (-1); //channel doesn't exist
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

//kick a user from a channel
void	Commands::kick(Server *server, std::string channel_name, std::string nickname)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		std::map<const int, Client>& clientMap = server->get_clients();
		std::map<const int, Client>::iterator it = clientMap.begin();

		for (; it != clientMap.end(); it++) {
			Client cl = it->second;

			if (cl.get_nickname() == nickname)
			{
				ch.remove_user(&cl, ch.get_password());
				ch.remove_operator(&cl, ch.get_password());
			}
		}
	}
}

std::string	Commands::get_channel_topic(Server *server, std::string channel_name)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		std::cout << "CHANNEL TOPIC NAME: " << ch.get_topic() << std::endl;
		return (ch.get_topic());
	}
	return ("");
}

void	Commands::change_topic(Server *server, std::string channel_name, std::string topic_name)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		std::cout << "CHANGE OF TOPIC IMPLEMENT.. " << ch.get_channel_name() << std::endl;
		ch.set_topic(topic_name);
	}
}

void	Commands::clean_topic(Server *server, std::string channel_name)
{
	if (server->channel_exists(channel_name))
	{
		Channel &ch = server->get_channel(channel_name);
		ch.set_topic("");
	}
}