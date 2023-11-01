#include "Commands.hpp"
#include "Message.hpp"
#include "Server.hpp"
#include "Channel.hpp"
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
	std::string command = msg->get_command();
	if (command == "CAP")							// change to switch case
		return (exec_cap());
	else if (command == "PASS")
		return (exec_pass(server, msg));
	else if (command == "NICK" && msg->get_sender()->is_authd())
		return (msg->get_sender()->set_nickname(msg->get_payload()), 0);
	else if (command == "USER" && msg->get_sender()->is_authd())
		return (msg->get_sender()->set_username(msg->get_payload()), 0);
	else if (command == "PRIVMSG" && msg->get_sender()->is_authd())
		return (server->send_private(msg));
	else if (command == "JOIN" && msg->get_sender()->is_authd())
		return(exec_join(server, msg));
	else 
		return(server->send_to_all_clients(msg)); // maybe first send into the clients writebuffer
}

int	Commands::exec_cap()
{
	// we ignore CAP messages
	return 0;
}

int Commands::exec_pass(Server *server, Message *msg)
{
	std::cout << "exec_pass: " << "msg->get_payload(): " << msg->get_payload() << " server->get_pass(): " << server->get_pass() << std::endl;
	if (msg->get_payload() == server->get_pass())
	{
		msg->get_sender()->authenticate(true);
		send(msg->get_sender()->get_client_fd(), "Client authorized successfully!!!\r\n", 35, 0); //use sentClient() when created
		return 0;
	}
	else
		return 1;
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
	std::map<std::string, Channel>* channels = server->get_channels();
	
	for (unsigned int i = 0; i < vchannels.size(); i++)
	{
		int ret;
		// new channel
		if (channels->find(vchannels[i]) == channels->end())
		{
			if (i < vpasswords.size()) // create channel with password
				server->add_channel(vchannels[i], vpasswords[i]);
			else // no password provided
				server->add_channel(vchannels[i], "");
		}

		// join the channel
		if (i < vpasswords.size())
			ret = (*channels)[vchannels[i]].add_user(msg->get_sender(), vpasswords[i]);
		else
			ret = (*channels)[vchannels[i]].add_user(msg->get_sender(), "");
		
		if (ret != 0)
			msg->send_to(msg->get_sender(), "JOIN: Error joining channel, wrong password?");
		else
		{
			msg->send_to(msg->get_sender(), "JOIN: Successfully joined channel " + vchannels[i] + "\n");
			std::cout << msg->get_sender() << " joined channel " << vchannels[i] << std::endl;
		}
	}
	return (0);
}

