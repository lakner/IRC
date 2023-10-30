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
//#include "Channel.hpp"

class Server
{
	public:
		Server();
		Server(char* port, char* password);
		~Server();
		int			prepare();
		int			run();
		Client&		get_client(int	client_fd);
	
	private:
		const char*				_port;
		const std::string 		_password;
		int						_server_sock_fd;
		std::vector< pollfd >	_pollfds;
		struct addrinfo*		createListenerSocket(struct addrinfo* addrinfo);
		void					newClientConnection();
		int						readFromExistingClient(int client_fd);
		void					add_client(int client_fd);
		std::map<const int, Client>		_clients;
		//std::map<std::string, Channel>	_channels;
		void					sendToAllClients();
};

#endif