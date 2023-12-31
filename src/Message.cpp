#include "Message.hpp"
#include "Numeric.hpp"

using std::string;

Message::Message(Client *sender, string content) : _sender(sender),
														_raw_content(content),
														_recpnt(NULL),
														_command("")
{
}

Message::~Message()
{

}

Client*	Message::get_sender()
{
	return(_sender);
}

Client*	Message::get_rcpnt()
{
	return(_recpnt);
}

string	Message::get_command()
{
	return(_command);
}

string	Message::get_payload()
{
	return(_payload);
}

string	Message::get_raw_content()
{
	return(_raw_content);
}

int	Message::parse()
{
	// strip the "\r\n"
	if (_raw_content.find("\r\n") != string::npos)
		_raw_content = _raw_content.substr(0, _raw_content.size() - string("\r\n").size());
	std::cout << "Message::parse:: _raw_content after stripping CRLF: " << _raw_content << std::endl;
	if (_raw_content.rfind("CAP", 0) == 0)
		_command = "CAP";
	else if (_raw_content.rfind("PING", 0) == 0)
		_command = "PING";
	else if (_raw_content.rfind("PASS", 0) == 0)
		_command = "PASS";
	else if (_raw_content.rfind("NICK", 0) == 0)
		_command = "NICK";
	else if (_raw_content.rfind("USER", 0) == 0)
		_command = "USER";
	else if (_raw_content.rfind("INVITE", 0) == 0)
		_command = "INVITE";
	else if (_raw_content.rfind("invite", 0) == 0)
		_command = "invite";
	else if (_raw_content.rfind("KICK", 0) == 0)
		_command = "KICK";
	else if (_raw_content.rfind("PRIVMSG", 0) == 0)
		_command = "PRIVMSG";
	else if (_raw_content.rfind("JOIN", 0) == 0)
		_command = "JOIN";
	else if (_raw_content.rfind("TOPIC", 0) == 0)
		_command = "TOPIC";
	else if (_raw_content.rfind("WHO", 0) == 0)
		_command = "WHO";
	else if (_raw_content.rfind("MODE", 0) == 0)
		_command = "MODE";
	else
	{
		std::stringstream ss(_raw_content);
		string first_word;

		ss >> first_word;
		send_to(_sender, _sender->get_server_string() + " " + ERR_UNKNOWNCOMMAND + " " + get_sender()->get_nickname() + " " + first_word + " :Unknown command");
		return -1;
	}
	if (!_command.empty() && _raw_content.find(_command) != string::npos)
	{
		_payload = _raw_content.substr(_command.size(), string::npos);
		// remove leading whitespaces
		if (!_payload.empty() && _payload.find_first_not_of(" \n\r\t\f\v") < _payload.size())
			_payload = _payload.substr(_payload.find_first_not_of(" \n\r\t\f\v"));
	}
	else
		_payload = _raw_content;
	return 0;
}

int	Message::sendmsg()
{
	send_to(_recpnt, _payload);
	return (0);	
}


int	Message::send_to(Client *new_recpnt, string content)
{
	content += "\r\n";
	new_recpnt->append_write_buffer(content);
	std::cout << "SENDING BACK TO CLIENT, put in write buffer: " << content << std::endl;
	return (0);
}

int	Message::send_from_server(Client *new_recpnt, string content)
{
	content = new_recpnt->get_server_string() + content;
	return(send_to(new_recpnt, content));
}

