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
		static int	exec_invite(Server *server, Message *msg);
		static int	exec_kick(Server *server, Message *msg);
		static int	exec_privmsg(Server *server, Message *msg);
		static int	exec_topic(Server *server, Message *msg);
		static int	exec_who(Server *server, Message *msg);
		static int	exec_mode(Server *server, Message *msg);
		//static void	invite(Server *server, std::string channel_name, std::string nickname);
		static bool	is_valid_nickname(std::string name);
		static bool	is_valid_username(std::string name);
		static bool	is_valid_channel_name(std::string channel_name);
};

#endif