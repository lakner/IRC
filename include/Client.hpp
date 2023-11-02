#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "Server.hpp"

class	Client
{
	public:
		Client(int client_fd, std::string ip4v_addr);
		~Client();

		void					set_read_buffer(char *buffer);
		std::string				get_read_buffer(void);
		void					set_write_buffer(std::string buffer);
		std::string				get_write_buffer(void);
		void					clear_read_buffer(void);
		void					clear_write_buffer(void);
		bool					is_authd();
		void					authenticate(bool valid);
		int						get_client_fd();
		std::string				get_nickname();
		void					set_nickname(std::string name);
		std::string				get_username();
		void					set_username(std::string name);
		std::string				get_full_client_identifier();
		void					set_full_client_identifier(std::string ident);

	private:
		int						_client_fd;
		std::string				_ipv4_addr;
		std::string				_nickname;
		std::string				_username;
		std::string				_host;
		std::string				_full_client_identifier;
		std::string				_read_buffer;
		std::string				_write_buffer;
		bool					_authenticated;

};

#endif