#include "Server.hpp"

Server::Server() : _port("6667"), _password("password")
{
}

Server::~Server()
{
}

Server::Server(char* port, char* password) : _port(port), _password(password)
{
}

// Server::Server(unsigned int port, std::string password)
// {
// 	this->_port = port;
// 	this->_password = password;
// }

// we don't need the stuff below at all, this is for clients.
// int Server::prepare()
// {
// 	struct addrinfo hints;
	
// 	std::memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_INET;		// IPV4 only
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_flags = AI_PASSIVE;
// 	// ...getaddrinfo crap
// }

int Server::prepare()
{
	struct addrinfo hints;
	struct addrinfo *p;
	int yes = 1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((getaddrinfo(NULL, this->_port, &hints, &this->_server_addrinfo)) != 0)
	{
		std::cerr << "Error getaddrinfo()" << std::endl;
		exit(1);
	}

	// create listener socket
	for(p = _server_addrinfo; p != NULL; p = p->ai_next) 
	{
		this->_server_sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
			// 	this->_server_addr.sin_family = AF_INET;
// 	this->_server_addr.sin_addr.s_addr = INADDR_ANY;
// 	this->_server_addr.sin_port = htons(this->_port);
		if (this->_server_sock_fd < 0)
			continue;
		setsockopt(this->_server_sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		
		if (bind(this->_server_sock_fd, p->ai_addr, p->ai_addrlen)
			< 0)
		{
			close(this->_server_sock_fd);
			continue;
		}
		break;
	}
	freeaddrinfo(this->_server_addrinfo);
	if (p == NULL)
		return -1;

	if (listen(this->_server_sock_fd, 10) == -1)
		return -1;
	return (this->_server_sock_fd);
}

int Server::run()
{
	int						newfd;
	socklen_t				addrlen;
	struct pollfd			pfd;
	char					buf[256];

	_pollfds.reserve(1000);	
	std::memset(&pfd, 0, sizeof(pfd));
	std::memset(&buf, 0, sizeof(buf));
	pfd.fd = this->_server_sock_fd;
	pfd.events = POLLIN;
	_pollfds.push_back(pfd);

	while (1)
	{
		// std::cout << "pollfds: " << _pollfds[0].fd << std::endl;
		// std::cout << "_server_sock_fd: " << _server_sock_fd << std::endl;
		// std::cout << "Size(_pollfds): " << _pollfds.size() << std::endl;
		int poll_count = poll(&(_pollfds[0]), _pollfds.size(), -1);
		if (poll_count == -1)
		{
			throw std::runtime_error("Polling error");
		}
		for(std::vector< pollfd >::iterator it = _pollfds.begin();
			it != _pollfds.end(); )
		{
			// poll_count --;
			// if (!poll_count)
			// 	break;
			// std::cout << "Iterating!" << std::endl;
			// std::cout << "checkin revents" << std::endl;
			// std::cout << "revents: " << (*it).revents << std::endl;
			if ((*it).revents & POLLIN)
			{
				//std::cout << "RECEIVED POLLIN: " << std::endl << "(*it).fd = " << (*it).fd << std::endl << "_server_sock_fd = " << _server_sock_fd << std::endl; 
				if ((*it).fd == this->_server_sock_fd)
				{
				// 	// handle new connection
					std::cout << "handle new connection" << std::endl;
					addrlen = sizeof(pfd);
					newfd = accept(this->_server_sock_fd,
								(struct sockaddr*) &pfd,
								&addrlen);
					if (newfd == -1)
					{
						std::cout << "accept() failed, terminating" << std::endl;
						throw "Error with accept().";
					}
					else
					{
						pfd.fd = newfd;
						pfd.events = POLLIN;
						pfd.revents = 0;
						 _pollfds.push_back(pfd);
						std::cout << "Server: new connecton snippet" << std::endl;
					}
				}
				else
				{
					std::cout << "Receiving data from " << (*it).fd << std::endl;
				// 	// we're not the listening fd -> existing client sending stuff
				// 	// If not the listener, we're just a regular client
					int nbytes = recv((*it).fd, buf, sizeof buf, 0);
					if (nbytes <= 0) 
					{
				 		// Got error or connection closed by client
						if (nbytes == 0) 
						{
						// Connection closed
							std::cout << "pollserver: socket " << (*it).fd << " hung up." << std::endl;
						}
						else 
						{
							std::cout << "Error on receive" << std::endl;
						}				
						close((*it).fd); // Bye!
						(*it).revents = 0;
						it = this->_pollfds.erase(it);
						std::cout << "successfully erased pollfd" << std::endl;
						continue;
					}	
					else 
					{
						// We got some good data from a client				
						std::cout << "receiving..."<< std::endl;
						std::cout << buf << std::endl;
						for(std::vector< pollfd >::iterator it = _pollfds.begin();
							it != _pollfds.end(); it++)
						{
							// don't send back to the server
							if (this->_server_sock_fd == (*it).fd)
								continue;
							std::cout << "Sending... "<< nbytes << " in buf: " << buf << " to " << (*it).fd << std::endl;
							if (send((*it).fd, buf, nbytes, 0) == -1) 
							{
								std::cout << "Error sending with send()." << std::endl;
								throw "Error sending.";
							}
						}
					}
				}
			}
			it++;
		}
	}
}
