#include "Message.hpp"

Message::Message()
{

}

Message::Message(Client *sender, std::string content) : _sender(sender),
														_raw_content(content)
{
	_recpnt = NULL;
	_command = "";
}

Message::~Message()
{

}

Client*	Message::get_sender()
{
	return(_sender);
}

Client* Message::get_rcpnt()
{
	return(_recpnt);
}

std::string Message::get_command()
{
	return(_command);
}

std::string	Message::get_payload()
{
	return(_payload);
}


void	Message::parse()
{
	if (_raw_content.rfind("CAP", 0) == 0)
		_command = "CAP";
	else if (_raw_content.rfind("PASS"))
		_command = "PASS";

	if (_raw_content.find(_command) != std::string::npos)
		_payload = _raw_content.erase(_raw_content.find(_command), _command.length());
}
