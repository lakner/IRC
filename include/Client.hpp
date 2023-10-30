#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <string>

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

	private:
		int						_client_fd;
		std::string				_nickname;
		std::string				_read_buffer;
		std::string				_write_buffer;

};

#endif