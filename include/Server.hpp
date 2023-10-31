#ifndef SERVER_H
#define SERVER_H

#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <map>
#include "Client.hpp"
#include "Message.hpp"
#include "Commands.hpp"
//#include "Channel.hpp"

class Client;

class Server
{
	public:
		Server();
		Server(char* port, char* password);
		~Server();
		int									prepare();
		int									run();
		Client&								get_client(int	client_fd);
		const std::string					get_pass();
		int									send_to_all_clients(Message *msg);
		int 								send_private(Message *msg);
		static std::map<const int, Client>&	get_clients();
		
	private:
		const char*							_port;
		const std::string 					_password;
		int									_server_sock_fd;
		std::vector< pollfd >				_pollfds;
		struct addrinfo*					create_listener_socket(struct addrinfo* addrinfo);
		void								new_client_connection();
		int									read_from_existing_client(int client_fd);
		void								add_client(int client_fd);
		static std::map<const int, Client>	_clients;

		//std::map<std::string, Channel>	_channels;
};

#endif
