#ifndef COMMANDS_H
#define COMMANDS_H

#pragma once
#include <string>

class Server;
class Message;
class Channel;

class Commands
{
	public:
		~Commands();
		static int	execute(Server *server, Message *msg);

		class UsernameAlreadyExists : public std::exception {
			public:
				const char* what() const throw();
		};

	private:
		Commands();
		static int	exec_pass(Server *server, Message *msg);
		static int	exec_ping(Message *msg);
		static int	exec_nick(Message *msg, Server *serv);
		static int	exec_user(Message *msg);
		static int	exec_join(Server *server, Message *msg);
		static int	exec_cap(Message *msg);
};

int	nickname_exists(std::string name, Server *serv);
#endif