#include "Server.hpp"

Server::Server() : _port(6667), _password("password")
{

}

Server::~Server()
{

}

Server::Server(unsigned int port, std::string password)
{
	this->_port = port;
	this->_password = password;
}

int Server::start()
{
	int bytes_read;
	char buf[256];

	this->_fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd_socket < 0)
		throw("Unable to create socket.");
	std::memset(this->_server_addr.sin_zero, 
				0, 
				sizeof(this->_server_addr.sin_zero));
	this->_server_addr.sin_family = AF_INET;
	this->_server_addr.sin_addr.s_addr = INADDR_ANY;
	this->_server_addr.sin_port = htons(this->_port);
	if (bind(this->_fd_socket, 
			(struct sockaddr *) &this->_server_addr, 
			sizeof(this->_server_addr))
		< 0)
		throw("Error on bind().");
	listen (this->_fd_socket, 16);
	this->_clilen = sizeof(this->_client_addr);
	this->_fd_new_socket = accept(_fd_socket, 
								(struct sockaddr *) &_server_addr, 
								&this->_clilen);
	if (this->_fd_new_socket < 0)
		throw("Error on accept().");
	std::memset(buf, 0, 256);
	// can set flags here instead of 0
	bytes_read = recv(this->_fd_new_socket, buf, 255, 0);
	if (bytes_read < 0)
		throw("Error on read() from socket");
	std::cout <<"Buffer contents: " << buf << std::endl;
	// can set flags here instead of 0
	write(this->_fd_new_socket, "Message received", 17);

	close(this->_fd_socket);
	close(this->_fd_new_socket);
	return 0;
}