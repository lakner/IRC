#ifndef COMMANDS_H
#define COMMANDS_H

#pragma once
#include <string>

class Server;
class Message;

class Commands
{
public:
	~Commands();
	static int	execute(Server *server, Message *msg);

private:
    Commands();
	static int	exec_pass(Server *server, Message *msg);
	static int	exec_cap();


};

#endif