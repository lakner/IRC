#include "Commands.hpp"
#include "Message.hpp"
#include "Server.hpp"

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
	if (msg->get_payload() == server->get_pass())
	{
		msg->get_sender()->authenticate(true);
		send(msg->get_sender()->get_client_fd(), "Client authorized successfully!!!\n", 34, 0); //use sentClient() when created
		return 0;
	}
	else
		return 1;
}

