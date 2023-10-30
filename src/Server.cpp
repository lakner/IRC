#include "Server.hpp"

// Server::Server() : _port("6667"), _password("password")
// {
// }

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
	for(p = this->_server_addrinfo; p != NULL; p = p->ai_next)
	{
		this->_server_sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (this->_server_sock_fd < 0)
			continue;
		fcntl(this->_server_sock_fd, F_SETFL, O_NONBLOCK);
		setsockopt(this->_server_sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		
		if (bind(this->_server_sock_fd, p->ai_addr, p->ai_addrlen) < 0)
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
	struct pollfd			pfd;

	_pollfds.reserve(1000);	
	std::memset(&pfd, 0, sizeof(pfd));
	pfd.fd = this->_server_sock_fd;
	pfd.events = POLLIN;
	_pollfds.push_back(pfd);

	while (1)
	{
		int poll_count = poll(&(_pollfds[0]), _pollfds.size(), -1);
		if (poll_count == -1)
		{
			throw std::runtime_error("Polling error");
		}
		for(std::vector< pollfd >::iterator it = _pollfds.begin();
			it != _pollfds.end(); )
		{
			if ((*it).revents & POLLIN)
			{
				if ((*it).fd == this->_server_sock_fd)
					newClientConnection();
				else
				{
					if (readFromExistingClient((*it).fd) <= 0)
					{
						(*it).revents = 0;
						it = this->_pollfds.erase(it);
						continue;
					}
				}
			}
			it++;
		}
	}
}

void	Server::add_client(int client_fd)
{
	Client		new_client(client_fd);

	_clients.insert(std::pair<int, Client>(client_fd, new_client));
}

Client&		Server::get_client(int client_fd)
{
	std::map<const int, Client>::iterator it = _clients.find(client_fd);
    return (it->second);
}

void	Server::newClientConnection()
{
	struct pollfd	pfd;
	struct sockaddr client_addr;
	socklen_t		client_addr_len;
	int				new_client_socket;

	std::cout << "Handling connection from a new client" << std::endl;
	std::memset(&pfd, 0, sizeof(pfd));
	client_addr_len = sizeof(client_addr);
	new_client_socket = accept(this->_server_sock_fd,
				&client_addr,
				&client_addr_len);
	if (new_client_socket == -1)
	{
		std::cout << "accept() failed, terminating" << std::endl;
		throw "Error with accept().";
	}
	else
	{
		fcntl(new_client_socket, F_SETFL, O_NONBLOCK);
		add_client(new_client_socket);
		pfd.fd = new_client_socket;
		pfd.events = POLLIN; // expand events later
		pfd.revents = 0;
		this->_pollfds.push_back(pfd);
		std::cout << "Server: new connecton from client added" << std::endl;
	}
}

int	Server::readFromExistingClient(int client_fd)
{
	char	buf[256];

	memset(&buf, 0, sizeof(buf));
	std::cout << "Receiving data from " << client_fd << std::endl;
	int nbytes = recv(client_fd, buf, sizeof (buf), 0);
	if (nbytes <= 0)
	{
		// Got error or connection closed by client
		if (nbytes == 0) // Connection closed
			std::cout << "pollserver: socket " << client_fd << " hung up." << std::endl;
		else // some other error
			std::cout << "Error on receive" << std::endl;
		close(client_fd); // Bye!
	}
	else
	{
		// We got some good data from a client
		Client&		client = get_client(client_fd);
		client.set_read_buffer(buf);
		// process_data() before copying to writebuffer
		client.set_write_buffer(client.get_read_buffer());
		client.clear_read_buffer();
		std::cout << "Received:  " << client.get_write_buffer() << std::endl;

		if (send(client_fd, "Server recieved:   ", 19, 0) == -1 || send(client_fd, client.get_write_buffer().c_str(), client.get_write_buffer().size(), 0) == -1)
		{
			std::cout << "Error sending with send()." << std::endl;
			throw "Error sending.";
		}
		client.clear_write_buffer();
		// for(std::vector< pollfd >::iterator it = _pollfds.begin();
		// 	it != _pollfds.end(); it++)
		// {
		// 	// don't send back to the server
		// 	if (this->_server_sock_fd == (*it).fd)
		// 		continue;
		// 	//std::cout << "Sending... " << get_write_buffer(void).size() << " bytes in buf: " << get_write_buffer(void) << " to " << (*it).fd << std::endl;
		// 	if (send((*it).fd, buf, nbytes, 0) == -1)
		// 	{
		// 		std::cout << "Error sending with send()." << std::endl;
		// 		throw "Error sending.";
		// 	}
		// 	client.clear_write_buffer();
		// }
	}
	return (nbytes);
}
