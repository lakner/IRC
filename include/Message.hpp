#ifndef MESSAGE_H
#define MESSAGE_H

#pragma once

#include <string>
#include "Server.hpp"

class Client;

class Message
{
	public:
		Message();
		Message(Client *sender, std::string content);
		~Message();
		void		parse();
		Client*		get_sender();
		std::string	get_command();
		std::string	get_payload();
		Client*		get_rcpnt();

	private:
		Client		*_sender;
		Client		*_recpnt;
		std::string	_raw_content;
		std::string	_command;
		std::string	_payload;
		// Channel	*_channel;
};

#endif