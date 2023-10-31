#include "Message.hpp"

Message::Message()
{

}

Message::Message(Client *sender, std::string content) : _sender(sender),
														_raw_content(content)
{
	_recpnt = NULL;
	_command = "";
	parse();
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
	// strip the "\r\n"
	_raw_content = _raw_content.substr(0, _raw_content.size() - "\r\n".size());
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
		std::string	recipient;
   		std::stringstream ss(_raw_content);
    	ss >> recipient >> recipient; // Read the second word (skipping leading whitespace)

		std::map<const int, Client>::iterator it;

		for (it = Server::get_clients().begin(); it != Server::get_clients().end(); it++) //segfaulting at odd fds
		{
    		std::cout << "1111" << std::endl;
			Client& client = it->second;
    		if (client.get_nickname() == recipient) 
			{
				std::cout << "111" << std::endl;
        		_recpnt = &client; 
        		break; 
    		}
			std::cout << "1121" << std::endl;
		}
		ss >> _payload;
		std::cout << "Recipient is: "<< recipient << " with _recpnt " << _recpnt << std::endl;
		std::cout << "Payload is: " << _payload << std::endl;
		
		if (!_recpnt)
			send_to(_sender, "PRIVMSG: recipient not found.");
		else if (! _payload.size())
			send_to(_sender, "PRIVMSG: message not found.");
		else
			_payload = _payload.substr(1, _payload.size() - 1);
		return ;
	}

	if (!_command.empty() && _raw_content.find(_command) != std::string::npos)
		// You can get nc to send \r\n with the -c switch, like this:
		// nc 127.0.0.1 6667 -c
		_payload = _raw_content.substr(_command.size() + 1, _raw_content.size() - (_command.size() + 3)); // +3 for KVIrc +2 for nc, because of \r\n instead of \n
	else
		_payload = _raw_content;
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