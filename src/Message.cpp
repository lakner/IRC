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


int	Message::parse()
{
	// strip the "\r\n"
	_raw_content = _raw_content.substr(0, _raw_content.size() - std::string("\r\n").size());
	std::cout << "Message::parse:: _raw_content after stripping CRLF: " << _raw_content << std::endl;
	if (_raw_content.rfind("CAP", 0) == 0)
		_command = "CAP";
	else if (_raw_content.rfind("PASS", 0) == 0)
		_command = "PASS";
	else if (_raw_content.rfind("NICK", 0) == 0)
		_command = "NICK";
	else if (_raw_content.rfind("USER", 0) == 0)
		_command = "USER";
	else if (_raw_content.rfind("PRIVMSG", 0) == 0)
	{
		_command = "PRIVMSG";;
		return(parse_privmsg());
	}

	if (!_command.empty() && _raw_content.find(_command) != std::string::npos)
	{
		// You can get nc to send \r\n with the -c switch, like this:
		// nc 127.0.0.1 6667 -c
		// removed the +3/+2 because we're already stripping "\r\n" at the top
		_payload = _raw_content.substr(_command.size(), _raw_content.size());
		// remove leading whitespaces
		_payload = _payload.substr(_payload.find_first_not_of(" \n\r\t\f\v"));
	}
	else
		_payload = _raw_content;
	return 0;
}

int	Message::sendmsg()
{
	// no authorization check here ,this is meant for the server sending messages
	if (send(_recpnt->get_client_fd(), (_payload + "\r\n").c_str(), _payload.length(), 0) == -1)
	{
		std::cout << "Error sending with send()." << std::endl;
		throw "Error sending.";
		return(-1);
	}
	return (0);	
}

int	Message::send_to(Client *new_recpnt)
{
	if (send(new_recpnt->get_client_fd(), (_payload + "\r\n").c_str(), _payload.size(), 0) == -1)
	{
		std::cout << "Error sending with send()." << std::endl;
		throw "Error sending.";
		return(-1);
	}
	return (0);
}

int	Message::send_to(Client *new_recpnt, std::string content)
{
	if (send(new_recpnt->get_client_fd(), (content + "\r\n").c_str(), content.size(), 0) == -1)
	{
		std::cout << "Error sending with send()." << std::endl;
		throw "Error sending.";
		return(-1);
	}
	return (0);
}

int	Message::parse_privmsg()
{
	std::string	recipient;
	std::stringstream ss(_raw_content);
	ss >> recipient >> recipient; // Read the second word (skipping leading whitespace)

	std::map<const int, Client>::iterator it;

	for (it = Server::get_clients().begin(); it != Server::get_clients().end(); it++)
	{
		// Check for ":" in the recipient here and throw an error?
		Client& client = it->second;
		if (client.get_nickname() == recipient) 
		{
			_recpnt = &client; 
			break; 
		}
	}

	if (!_recpnt)
	{
		send_to(_sender, "PRIVMSG: recipient not found.\n");
		return -1;
	}
		// With stringstream we only get the first word, 
	// but we need the entire portion of the command following a ':'
	if (_raw_content.find(":") == std::string::npos)
	{
		send_to(_sender, "PRIVMSG: message not found.\n");
		return -1;
	}
	else
	{
		_payload = _raw_content.substr(_raw_content.find(":") + 1, _raw_content.size());
		std::cout << "PRIVMSG: Recipient is: "<< recipient << " with _recpnt " << _recpnt << std::endl;
		std::cout << "PRIVMSG: Payload is: " << _payload << std::endl;
	}
	return 0;

}