#ifndef MESSAGE_H
#define MESSAGE_H

#pragma once

#include <string>
#include "Server.hpp"
#include <sstream>
#include "Client.hpp"
#include <map>
#include "Server.hpp"

class Client;

class Message
{
	public:
		Message(Client *sender, std::string content);
		~Message();
		int			parse();
		Client*		get_sender();
		std::string	get_command();
		std::string	get_payload();
		std::string	get_raw_content();
		Client*		get_rcpnt();
		int			sendmsg();
		static int	send_to(Client *new_recpnt, std::string content);
		static int	send_from_server(Client *new_recpnt, std::string content);

	private:
		Client		*_sender;
		std::string	_raw_content;
		Client		*_recpnt;	//if defined use this, if not sent to all Clients
		std::string	_command;
		std::string	_payload;
		// Channel	*_channel;
};

#endif