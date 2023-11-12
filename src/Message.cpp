#include "Message.hpp"
#include "Numeric.hpp"

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

std::string	Message::get_raw_content()
{
	return(_raw_content);
}

int	Message::parse()
{
	// strip the "\r\n"
	if (_raw_content.find("\r\n") != std::string::npos)
		_raw_content = _raw_content.substr(0, _raw_content.size() - std::string("\r\n").size());
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
	else if (_raw_content.rfind("KICK", 0) == 0)
		_command = "KICK";
	else if (_raw_content.rfind("PRIVMSG", 0) == 0)
		_command = "PRIVMSG";
	else if (_raw_content.rfind("JOIN", 0) == 0)
		_command = "JOIN";
	else if (_raw_content.rfind("TOPIC", 0) == 0)
		_command = "TOPIC";
	else
	{
		std::stringstream ss(_raw_content);
		std::string first_word;

		ss >> first_word;
		send_to(get_sender(), std::string(HOSTNAME) + " " + ERR_UNKNOWNCOMMAND + " " + get_sender()->get_nickname() + " " + first_word + " :Unknown command");
		return -1;
	}
	if (!_command.empty() && _raw_content.find(_command) != std::string::npos)
	{
		// You can get nc to send \r\n with the -c switch, like this:
		// nc 127.0.0.1 6667 -c
		// removed the +3/+2 because we're already stripping "\r\n" at the top
		_payload = _raw_content.substr(_command.size(), std::string::npos);
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


int	Message::send_to(Client *new_recpnt, std::string content)
{
	// char hostname[64] = ":127.0.0.1";
	// //gethostname(hostname, sizeof(hostname));
	// content = std::string(":") + hostname + " " + content;
	content += "\r\n";
	new_recpnt->append_write_buffer(content);
	// if (send(new_recpnt->get_client_fd(), (content + "\r\n").c_str(), content.size() + 2, 0) == -1)
	// {
	// 	std::cout << "Error sending with send()." << std::endl;
	// 	throw "Error sending.";
	// 	return(-1);
	// }
	std::cout << "SENDING BACK TO CLIENT, put in write buffer: " << content << std::endl;
	return (0);
}

int	Message::send_from_server(Client *new_recpnt, std::string content)
{
	char hostname[64] = "127.0.0.1";
	// //gethostname(hostname, sizeof(hostname));
	content = std::string(":") + hostname + " " + content;
	return(send_to(new_recpnt, content));
}

