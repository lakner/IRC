#include "Server.hpp"

using std::string;

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


std::map<const int, Client>&	Server::get_clients()
{
	return (_clients);
}


std::map<std::string, Channel>&	Server::get_channels()
{
	return(_channels);
}


Client&		Server::get_client(int client_fd)
{
	std::map<const int, Client>::iterator it = _clients.find(client_fd);
	return (it->second);
}


Client&		Server::get_client(std::string client_name)
{
	std::map<const int, Client>::iterator it = _clients.begin();

	for (; it != _clients.end(); it++)
	{
		if ((it->second).get_nickname() == client_name)
			return (it->second);
	}
	return (it->second);
}

Channel&	Server::get_channel(std::string name)
{
	std::map<std::string, Channel>::iterator it = _channels.begin();

	for (; it != _channels.end(); it++)
	{
		Channel &ch = it->second;
		if (it->second.get_channel_name() == name)
			return(ch);
	}
	return it->second;
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
		
		int send_buffer_size;
		socklen_t optlen = sizeof(send_buffer_size);
		getsockopt(this->_server_sock_fd, SOL_SOCKET, SO_SNDBUF, &send_buffer_size, &optlen);
		std::cout << "OPTLEN send_buffer_size = " << send_buffer_size << std::endl;
		
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
	pfd.events = POLLIN | POLLOUT;
	_pollfds.push_back(pfd);

	while (1)
	{
		int poll_count = poll(&(_pollfds[0]), _pollfds.size(), -1);
		if (poll_count == -1)
		{
			throw std::runtime_error("Polling error");
		}
		for(std::vector< pollfd >::iterator it = _pollfds.begin(); it != _pollfds.end();)
		{
			if (it->revents & POLLIN)
			{
				if (it->fd == this->_server_sock_fd)
					new_client_connection();
				else
				{
					if (read_from_existing_client(it->fd) <= 0)
					{
						std::map<const int, Client>::iterator it1 = _clients.find(it->fd);
						if (it1 != _clients.end())
							_clients.erase(it1);
						it->revents = 0;
						it = this->_pollfds.erase(it);
						continue;
					}
				}
			}
			if(it->revents & POLLOUT)
				get_client(it->fd).send_all_in_write_buffer();
			it++;
		}
	}
}


void	Server::new_client_connection()
{
	struct pollfd	pfd;
	struct sockaddr client_addr;
	socklen_t		client_addr_len = sizeof(client_addr);
	int				client_sock;

	std::cout << "Handling connection from a new client" << std::endl;
	std::memset(&pfd, 0, sizeof(pfd));
	client_sock = accept(this->_server_sock_fd, &client_addr, &client_addr_len);
	if (client_sock == -1)
	{
		std::cout << "accept() failed, terminating" << std::endl;
		throw "Error with accept().";
	}
	else
	{
		fcntl(client_sock, F_SETFL, O_NONBLOCK);
		std::string client_ip = read_client_ipv4_addr(client_addr);
		std::string server_ip = read_server_ipv4_addr(client_sock);
		add_client(client_sock, client_ip, server_ip);
		pfd.fd = client_sock;
		pfd.events = POLLIN | POLLOUT;
		pfd.revents = 0;
		this->_pollfds.push_back(pfd);
		std::cout << "Server: new connecton from client added" << std::endl;
	}
}

string	Server::read_server_ipv4_addr(int socket_fd)
{
	char server_ip[16];
	struct sockaddr_in sock;
	socklen_t len = sizeof(sock);
	getsockname(socket_fd, (struct sockaddr *) &sock, &len);
	inet_ntop(AF_INET, &sock.sin_addr, server_ip, sizeof(server_ip));
	std::cout << "Server ip address: " << server_ip << std::endl;
	return string(server_ip);
}

string	Server::read_client_ipv4_addr(struct sockaddr& client_addr)
{
	struct in_addr ip_addr = ((struct sockaddr_in*) &client_addr)->sin_addr;
	char ip_str[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &ip_addr, ip_str, INET_ADDRSTRLEN);
	std::cout << "New client IP address: " << ip_str << std::endl;
	return string(ip_str);
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


void	Server::add_client	(int client_fd, 
							std::string client_ip_v4_addr,
							std::string server_ipv4_addr)
{
	Client		new_client(client_fd, client_ip_v4_addr, server_ipv4_addr);
	_clients.insert(std::pair<int, Client>(client_fd, new_client));
}


void Server::add_channel(std::string name, std::string pass)
{
	if (_channels.find(name) == _channels.end())
		_channels[name] = Channel(name, pass);
	else
		std::cerr << "Server::add_channel: Trying to add a channel that already exists." << std::endl;
}


int	Server::channel_exists(std::string channel_name)
{
	std::map<std::string, Channel>::iterator it = _channels.begin();

	for (; it != _channels.end(); it++) {
		Channel &cl = it->second;

		if (cl.get_channel_name() == channel_name)
			return 1; // Nickname found
	}
	return 0; // Nickname not found
}


bool	Server::nickname_exists(std::string nickname)
{
	std::map<const int, Client>::iterator it = _clients.begin();
	
	for(; it != _clients.end(); it++)
	{
		Client& client = it->second;
		if (client.get_nickname() == nickname)
			return true;
	}
	return false;
}