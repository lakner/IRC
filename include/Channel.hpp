#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <map>

class	Client;
class	Server;
class	Message;

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
		Channel(std::string channel_name, std::string password); // Server* server);
		~Channel();
		int								add_user(Client *client, std::string password);
		int								send_to_all_in_channel(std::string msg);
		int								send_to_all_except(std::string content, Client& client);
		int								remove_user(Client *client);
		int								add_operator(Client *client);
		int								remove_operator(Client *client);
		bool							client_in_channel(Client &client);
		std::string						get_channel_name( void );
		std::map<std::string, Client*>&	get_users();
		std::map<std::string, Client*>&	get_operators();
		Mode							get_mode();
		std::string						get_password();
		std::string						get_topic();
		void							set_topic( std::string new_topic );
		bool							is_operator(std::string nickname);
		bool							allowed_to_set_topic(std::string nickname);
		bool							allowed_to_invite_kick(std::string nickname);
		void							kick(std::string nickname);
		
	private:
		std::string						_channel_name;
		std::string						_password;
		std::string						_topic;
		Mode							_mode;
		std::map<std::string, Client*>	_client_list;
		std::map<std::string, Client*>	_operator_list;
		void							notify_user_joined(Client *client);
		void							notify_user_exit(Client *client);
		void							notify_user_is_operator(Client *client);
		void							notify_mode_changed(Client *client);
		void							send_topic(Client *client);
		void							send_user_list(Client *client);
};

#endif