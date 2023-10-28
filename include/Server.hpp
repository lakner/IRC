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
		//Server(unsigned int port, std::string password);
		Server(char* port, char* password);
		~Server();
		int			prepare();
		int			run();
		//int			start();

	
	private:
		//unsigned int					_port;
		const char*						_port;
		const std::string 				_password;
		struct addrinfo 				*_server_addrinfo;
		int								_server_sock_fd;
		//int							_client_sock_fd;
		std::vector< pollfd> _pollfds;


		//struct addrinfo		_server_addr_info;
		// struct sockaddr_in	_server_addr;
		// struct sockaddr_in	_client_addr;
		// std::vector< sockaddr_storage >	_client_addresses;

		// socklen_t						_addr_len;

};

#endif