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
		static int			exec_pass(Server *server, Message *msg);
		static int			exec_ping(Message *msg);
		static int			exec_nick(Message *msg, Server *serv);
		static int			exec_user(Message *msg);
		static int			exec_join(Server *server, Message *msg);
		static int			exec_cap(Message *msg);
		static int			exec_invite(Server *server, Message *msg);
		static int			exec_kick(Server *server, Message *msg);
		static int			exec_privmsg(Server *server, Message *msg);
		static int			exec_topic(Server *server, Message *msg);
		static int			nickname_exists(std::string name, Server *serv);
		// static int			channel_exists(std::string channel_name, Server *server, Channel*& myChannel);
		// static int			client_belong_to_channel(Server *server, std::string channel_name, std::string nickname);
		static int			allow_to_invite(Server *server, std::string channel_name, std::string nickname);
		static void			invite(Server *server, std::string channel_name, std::string nickname);
		static void			kick(Server *server, std::string channel_name, std::string nickname);
		static int			allow_to_set_topic(Server *server, std::string channel_name, std::string nickname);
		static std::string	get_channel_topic(Server *server, std::string channel_name);
		static void			change_topic(Server *server, std::string channel_name, std::string topic_name);
		static void			clean_topic(Server *server, std::string channel_name);
};

#endif