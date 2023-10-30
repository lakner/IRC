#include "Commands.hpp"
#include "Message.hpp"

Commands::Commands()
{

}

Commands::~Commands()
{

}

int	Commands::execute(Server *server, Message *msg)
{
	std::string command = msg->get_command();
	if (command == "CAP")
		return(exec_cap());
	else if (command == "PASS")
		return(exec_pass(server, msg));
	else
		return(server->sendToAllClients());
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
		return 0;
	}
	else
		return 1;
}

