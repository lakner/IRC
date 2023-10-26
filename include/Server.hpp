#ifndef SERVER_H
#define SERVER_H

#pragma once
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

class Server
{
	public:
		Server();
		Server(unsigned int port, std::string password);
		~Server();
		int start();
	
	private:
		unsigned int		_port;
		std::string 		_password;
		int					_fd_socket;
		int					_fd_new_socket;
		socklen_t			_clilen;

		struct sockaddr_in _server_addr;
		struct sockaddr_in _client_addr;

};

#endif