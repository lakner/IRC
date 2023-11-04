#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <map>

class	Client;
class	Server;

typedef	struct s_mode
{
	int	i;
	int	t;
	int	k;
	int	o;
	int	l;
}			Mode;

class	Channel
{
	public:
		Channel();
		Channel(std::string channel_name, std::string password);
		~Channel();
		int add_user(Client *client, std::string password);
		int remove_user(Client *client, std::string password);
		int add_operator(Client *client, std::string password);
		int remove_operator(Client *client, std::string password);
		std::string	get_channel_name( void );
		static std::map<std::string, Client*>&		get_users();
		static std::map<std::string, Client*>&		get_operators();
		Mode								get_mode();
		std::string							get_password();
		
	private:
		std::string						_channel_name;
		std::string						_password;
		std::string						_topic;
		static std::map<std::string, Client*>	_client_list;
		static std::map<std::string, Client*>	_operator_list;
		Mode							_mode;
		void							notify_user_joined(Client *client);
		void							notify_user_exit(Client *client);
		void							notify_user_is_operator(Client *client);
		void							notify_mode_changed(Client *client);
		void							send_topic(Client *client);
		void							send_user_list(Client *client);
};

#endif