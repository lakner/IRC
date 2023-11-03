#include "Server.hpp"

// Server::Server() : _port("6667"), _password("password")
// {
// }

std::map<const int, Client> Server::_clients;

Server::~Server()
{
}

Server::Server(char* port, char* password) : _port(port), _password(password)
{
}


const std::string	Server::get_pass()
{
	return(_password);
}

int	Server::prepare()
{
	struct addrinfo *addrinfo, hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((getaddrinfo(NULL, this->_port, &hints, &addrinfo)) != 0)
	{
		std::cerr << "Error getaddrinfo()" << std::endl;
		exit(1);
	}

	if (create_listener_socket(addrinfo) == NULL)
	{
		freeaddrinfo(addrinfo);
		return -1;
	}
	freeaddrinfo(addrinfo);

	if (listen(this->_server_sock_fd, 10) == -1)
		return -1;
	return (this->_server_sock_fd);
}

struct addrinfo*	Server::create_listener_socket(struct addrinfo* addrinfo)
{
	struct addrinfo *p = NULL;
	int yes = 1;
	for(p = addrinfo; p != NULL; p = p->ai_next) 
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
		return p;
	}
	return p;
}

int	Server::run()
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
					new_client_connection();
				else
				{
					if (read_from_existing_client((*it).fd) <= 0)
					{
						std::map<const int, Client>::iterator it1 = _clients.find(it->fd);
						if (it1 != _clients.end())
							_clients.erase(it1);
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

void	Server::add_client(int client_fd, std::string ip_v4_addr)
{
	Client		new_client(client_fd, ip_v4_addr);

	//std::cout << "ADD CLIENT: " << new_client.get_client_fd() << std::endl;
	_clients.insert(std::pair<int, Client>(client_fd, new_client));
}

Client&		Server::get_client(int client_fd)
{
	std::map<const int, Client>::iterator it = _clients.find(client_fd);
	return (it->second);
}

std::map<const int, Client>&	Server::get_clients()
{
	return (_clients);
}

void	Server::new_client_connection()
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
	std::cout << "NEW CLIENT SOCKET: " << new_client_socket << std::endl;
	if (new_client_socket == -1)
	{
		std::cout << "accept() failed, terminating" << std::endl;
		throw "Error with accept().";
	}
	else
	{
		fcntl(new_client_socket, F_SETFL, O_NONBLOCK);
		add_client(new_client_socket, read_client_ipv4_address(client_addr));

		pfd.fd = new_client_socket;
		pfd.events = POLLIN; // expand events later
		pfd.revents = 0;
		this->_pollfds.push_back(pfd);
		std::cout << "Server: new connecton from client added" << std::endl;
	}
}

int	Server::read_from_existing_client(int client_fd)
{
	char	buf[256];
	int		nbytes = 0;

	std::cout << "Receiving data from " << client_fd << std::endl;
	Client &client = get_client(client_fd);
	memset(&buf, 0, sizeof(buf));
	nbytes = recv(client_fd, buf, sizeof (buf), 0);
	client.set_read_buffer(buf);
	std::cout << "read " << nbytes << " bytes in buf: " << buf;
	std::cout << "get_read_buffer: " << client.get_read_buffer() << std::endl;

	if (nbytes <= 0)
	{
		if (nbytes == 0) // Connection closed
			std::cout << "pollserver: socket " << client_fd << " hung up." << std::endl;
		else // some other error
			std::cout << "Error on receive" << std::endl;
		close(client_fd); // Bye!
		return (nbytes);
	}
	while (client.get_read_buffer().find("\r\n") != std::string::npos)
	{
		std::cout << "Found CRLF in message, continuing" << std::endl;
		int pos = client.get_read_buffer().find("\r\n");
		Message msg(&client, client.get_read_buffer().substr(0, pos + 2));
		std::cout << "Read buffer contains: " << client.get_read_buffer() << std::endl
				  << "-----END OF READ BUFFER-----" << std::endl;
		client.clear_read_buffer();
		if (msg.parse() == 0)
			Commands::execute(this, &msg);
	}
	std::cout << "Done processing read buffer: " << client.get_read_buffer() << std::endl;
	return(nbytes);
}

int	Server::send_private(Message *msg)
{
	if (!msg->get_sender()->is_authd())
	{
		send(msg->get_sender()->get_client_fd(), "Client not authorized! Try connecting with: PASS password\r\n", 59, 0);
		return (0);
	}

	if (!msg->get_rcpnt())
		return (0);
	if (send(msg->get_rcpnt()->get_client_fd(), (msg->get_sender())->get_nickname().c_str(), msg->get_sender()->get_nickname().size(), 0) == -1 
			|| send(msg->get_rcpnt()->get_client_fd(), " client sent: '", 15, 0) == -1
			|| send(msg->get_rcpnt()->get_client_fd(), (msg->get_payload() + std::string("'\r\n")).c_str(), msg->get_payload().size() + 3, 0) == -1)
	{
		std::cout << "Error sending with send()." << std::endl;
		throw "Error sending.";
		return(-1);
	}
	return (0);
}

int	Server::send_to_all_clients(Message *msg)
{
	if (!msg->get_sender()->is_authd())
	{
		send(msg->get_sender()->get_client_fd(), "Not authorized as a valid user!! Try to connect with: PASS password\r\n", 69, 0);
		return (0);
	}
	// We got some good data from a client
	for(std::map<const int, Client>::iterator it = _clients.begin();
		it != _clients.end(); it++)
	{
		// don't send back to the server
		if (this->_server_sock_fd == it->first || !it->second.is_authd())
			continue;

		if (send(it->first, (msg->get_sender())->get_nickname().c_str(), msg->get_sender()->get_nickname().size(), 0) == -1 
			|| send(it->first, " client send:  ", 15, 0) == -1
			|| send(it->first, (msg->get_payload() + std::string("\r\n")).c_str(), msg->get_payload().size() + 2, 0) == -1)
		{
			std::cout << "Error sending with send()." << std::endl;
			throw "Error sending.";
			return(-1);
		}
	}
	return(0);
}

std::map<std::string, Channel>*	Server::get_channels()
{
	return(&_channels);
}

void Server::add_channel(std::string name, std::string pass)
{
	if (_channels.find(name) == _channels.end())
	{
		//Channel channel(name, pass);
		_channels[name] = Channel(name, pass);
	}
	else
	{
		std::cerr << "Server::add_channel: Trying to add a channel that already exists." << std::endl;
	}
}

std::string	Server::read_client_ipv4_address(struct sockaddr& client_addr)
{
	struct sockaddr_in *ip_v4_addr = (struct sockaddr_in*) &client_addr;
	struct in_addr ip_addr = ip_v4_addr->sin_addr;
	char ip_str[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &ip_addr, ip_str, INET_ADDRSTRLEN);
	std::cout << "New client IP address: " << ip_str << std::endl;
	return(std::string(ip_str));
}