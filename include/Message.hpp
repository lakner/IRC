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
		Message();
		Message(Client *sender, std::string content);
		~Message();
		int			parse();
		Client*		get_sender();
		std::string	get_command();
		std::string	get_payload();
		std::string	get_raw_content();
		Client*		get_rcpnt();
		int			sendmsg();
		int			send_to(Client *new_recpnt);
		int			send_to(Client *new_recpnt, std::string content);
		int			send_from_server(Client *new_recpnt, std::string content);
		// std::string	get_raw_content();

	private:
		Client		*_sender;
		Client		*_recpnt;	//if defined use this, if not sent to all Clients
		std::string	_raw_content;
		std::string	_command;
		std::string	_payload;
		// Channel	*_channel;
};

#endif