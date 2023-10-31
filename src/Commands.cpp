#include "Commands.hpp"
#include "Message.hpp"
#include "Server.hpp"
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
	std::string channels, passwords;

	ss >> channels >> passwords;
	std::cout << "JOIN: channels: " << channels << "\n\tPasswords are: " << passwords << std::endl;

	std::vector<std::string> vchannels, vpasswords;
	
	std::stringstream channelstr(channels);
	while (channelstream.good())
	{
		std::string ch;
		std::getline(channelstr, ch, ',');
		if (!s.rfind("#", 0) == 0)
		{
			msg->sendto(msg->get_sender->get_client_fd(), "Channel names must start with '#'");
			return -1;
		}
		vchannels.push_back(ch);
	}
	std::stringstream passstr(channels);
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
	return (0);
}

