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
		_payload = _payload.substr(1, _payload.size() - 1);
		return ;
	}

	if (!_command.empty() && _raw_content.find(_command) != std::string::npos)
		_payload = _raw_content.substr(_command.size() + 1, _raw_content.size() - (_command.size() + 2)); // +3 for KVIrc +2 for nc, because of \r\n instead of \n
	else
		_payload = _raw_content;
}
