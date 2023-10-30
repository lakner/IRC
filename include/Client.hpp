#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>
#include "Server.hpp"

class	Client
{
	public:
		Client(int client_fd);
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
	private:
		int						_client_fd;
		std::string				_nickname;
		std::string				_read_buffer;
		std::string				_write_buffer;
		bool					_authenticated;

};

#endif