#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <string>
#include <map>

class	Client;
class	Server;
class	Message;

class	Channel
{
	public:
		Channel();
		Channel(std::string channel_name, std::string password); // Server* server);
		~Channel();
		std::string						add_user(Client *client, std::string password);
		int								send_to_all_in_channel(std::string msg);
		int								send_to_all_except(std::string content, Client& client);
		int								remove_user(Client *client);
		int								add_operator(Client *client);
		int								add_invited(Client *client);
		int								remove_operator(Client *client);
		int								remove_invited(Client *client);
		bool							client_in_channel(Client &client);
		std::string						get_channel_name( void );
		std::map<std::string, Client*>&	get_users();
		std::map<std::string, Client*>&	get_operators();
		std::string						get_modes();
		std::string						set_mode(char mode, bool mode_stat, std::stringstream *param, Server *server, Message *msg);
		std::string						get_password();
		std::string						get_topic();
		int								get_userlimit();
		bool							get_invite_only();
		void							set_topic( std::string new_topic );
		bool							is_operator(std::string nickname);
		bool							is_invited(std::string nickname);
		bool							allowed_to_set_topic(std::string nickname);
		void							kick(std::string nickname);
		void							invite(Client *client);

	private:
		int								_channelusers;
		std::string						_channel_name;
		std::string						_password;
		int								_userlimit;
		std::string						_mode;
		std::string						_topic;
		bool							_invite_only;
		bool							_topic_change;
		std::map<std::string, Client*>	_client_list;
		std::map<std::string, Client*>	_operator_list;
		std::map<std::string, Client*> 	_invited_users;
		void							notify_user_joined(Client *client);
		void							notify_user_exit(Client *client);
		//void							notify_user_is_operator(Client *client);


		//void							notify_mode_changed(Client *client);
		void							send_topic(Client *client);
		void							send_user_list(Client *client);
		// std::string 					add_mode_change(char mode, bool *sign, bool mode_stat);
};

#endif