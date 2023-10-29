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

class Server
{
	public:
		Server();
		Server(char* port, char* password);
		~Server();
		int			prepare();
		int			run();
	
	private:
		const char*				_port;
		const std::string 		_password;
		struct addrinfo 		*_server_addrinfo;
		int						_server_sock_fd;
		std::vector< pollfd >	_pollfds;
		void					newClientConnection();
		std::vector< pollfd> _pollfds;
};

#endif