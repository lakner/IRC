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
#include <arpa/inet.h>
#include <poll.h>
#include <map>
#include <exception>
#include "Client.hpp"
#include "Message.hpp"
#include "Commands.hpp"
#include "Channel.hpp"

class Client;

class Server
{
	public:
		Server();
		Server(char* port, char* password);
		~Server();
		int								prepare();
		int								run();
		Client&							get_client(int	client_fd);
		Client&							get_client(std::string client_name);
		const std::string				get_pass();
		int								send_to_all_clients(Message *msg);
		int								send_private(Message *msg);
		std::map<const int, Client>&	get_clients();
		std::map<std::string, Channel>&	get_channels();
		Channel&						get_channel(std::string name);
		void							add_channel(std::string name, std::string pass);
		int								channel_exists(std::string channel_name, Channel& myChannel);
		int								channel_exists(std::string channel_name);
		bool							nickname_exists(std::string nickname);	

		
	private:
		const char*						_port;
		const std::string 				_password;
		int								_server_sock_fd;
		std::vector< pollfd >			_pollfds;
		std::map<const int, Client>		_clients;
		std::map<std::string, Channel>	_channels;
		struct addrinfo*				create_listener_socket(struct addrinfo* addrinfo);
		void							new_client_connection();
		int								read_from_existing_client(int client_fd);
		void							add_client(int client_fd, std::string client_ipv4, std::string server_ipv4);
		std::string						read_client_ipv4_addr(struct sockaddr& client_addr);
		std::string						read_client_ipv4_addr(int socket_fd);
		std::string						read_server_ipv4_addr(int socket_fd);
		int								remove_client();
};

#endif
