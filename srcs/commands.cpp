//
// Created by rdas-nev on 5/18/23.
//

#include "../inc/ft_irc.hpp"

void Server::commands(std::string buffer, int client_fd)
{
	if (buffer[0] == '\n' || buffer[0] == '\r')
		return;
	if (buffer.substr(0, 4) == ("PASS"))
		pass(buffer, client_fd);
	else if (users[client_fd].getPassAuth() == '0') // If password is not authenticated
	{
		send(client_fd, "\nerror: introduce password first.\r\n", 35, 0);
		return;
	}
	else if (buffer.substr(0, 4) == ("NICK"))
		nick(buffer, client_fd);
	else if (buffer.substr(0, 4) == ("USER"))
		user(buffer, client_fd);
	else if (!users[client_fd].isAuthenticated()) // If user is not authenticated
	{
		send(client_fd, "\nerror: not authenticated.\n", 26, 0);
		return;
	}
	else if (buffer.substr(0, 5) == ("JOIN "))
		join(buffer, client_fd);
	else if (buffer.substr(0, 8) == ("PRIVMSG "))
		msg(buffer, client_fd);
	else if (buffer.substr(0, 7) == "NOTICE ")
		notice(buffer, client_fd);
	else if (buffer.substr(0, 5) == ("PART "))
		part(buffer, client_fd);
	else if (buffer.substr(0, 5) == ("OPER "))
		oper(buffer, client_fd);
	else if (buffer.substr(0, 5) == ("KICK "))
		kick(buffer, client_fd);
	else if (buffer.substr(0, 7) == ("INVITE "))
		invite(buffer, client_fd);
	else if (buffer.substr(0, 6) == ("TOPIC "))
		topic(buffer, client_fd);
	else if (buffer.substr(0, 5) == ("MODE "))
		mode(buffer, client_fd);
	else
		send(client_fd, "\nerror: command not found.\n", 27, 0);
}

void Server::pass(std::string buf, int fd)
{
	std::string pass = buf.substr(buf.find("PASS")+5);
	pass = pass.substr(0, pass.find('\n'));
	pass = pass.substr(0, pass.find('\r'));
	std::cout << BLUE"pass: " RESET<< pass << std::endl;
	if (users[fd].getPassAuth() == '1')
	{
		send(fd, "Password already set.\r\n", 24, 0);
		return;
	}
	if (pass == password)
	{
		users[fd].setAuthenticated('1', 0);
		send(fd, "Password correct.\r\n", 19, 0);
	}
	else
	{
		send(fd, "Invalid password, submit again please.\r\n", 40, 0);
	}
}

void Server::nick(std::string buf, int fd)
{
	if (buf[4] == '\n' || buf[4] == '\r' || buf[4] != ' ')
	{
		send(fd, "Invalid nickname.\r\n", 19, 0);
		return;
	}
	std::string nick = buf.substr(buf.find("NICK")+5);
	nick = nick.substr(0, nick.find('\n'));
	nick = nick.substr(0, nick.find('\r'));
	std::cout << BLUE"nick: " << nick << "|" RESET << std::endl;
	if (!validateNickUser(nick, fd, 0))
		return;
	users[fd].setNickName(nick);
	users[fd].setAuthenticated('1', 1);
	send(fd, "Your nickname has been set.\r\n", 29, 0);
	if (users[fd].isAuthenticated() && !users[fd].getAuthCount())
	{
		send(fd, "You are now authenticated.\r\n", 28, 0);
		users[fd].setAuthCount(1);
	}
}

void Server::user(std::string buf, int fd)
{
	if (users[fd].getUserAuth() == '1')
	{
		send(fd, "User already set.\r\n", 19, 0);
		return;
	}
	if (buf[4] == '\n' || buf[4] == '\r' || buf[4] != ' ')
	{
		send(fd, "Invalid username.\r\n", 19, 0);
		return;
	}
	std::string user = buf.substr(buf.find("USER")+5);
	user = user.substr(0, user.find('\n'));
	user = user.substr(0, user.find('\r'));
	std::cout << BLUE"user: " << user << "|" RESET << std::endl;
	if (!validateNickUser(user, fd, 1))
		return;
	users[fd].setUserName(user);
	users[fd].setAuthenticated('1', 2);
	send(fd, "Your username has been set.\r\n", 29, 0);
	if (users[fd].isAuthenticated() && !users[fd].getAuthCount())
	{
		send(fd, "You are now authenticated.\r\n", 28, 0);
		users[fd].setAuthCount(1);
	}

}

void Server::join(std::string buf, int client_fd)
{
	std::string channel_name;
	std::string input_key;

	if (buf.find('#') != std::string::npos)
	{
		channel_name = buf.substr(buf.find('#'));
		channel_name = channel_name.substr(0, channel_name.find('\n'));
		channel_name = channel_name.substr(0, channel_name.find('\r'));
		if (channel_name.find(' ') != std::string::npos)
		{
			input_key = (channel_name.substr(channel_name.find(' ') + 1));
			input_key = input_key.substr(0, input_key.find('\n'));
			input_key = input_key.substr(0, input_key.find('\r'));
			channel_name = channel_name.substr(0, channel_name.find(' '));
		}
		std::cout << BLUE"CHANNEL_NAME: " << channel_name << "|" RESET << std::endl;
	}
	else
	{
		send(client_fd, "error on command\r\n", 18, 0);
		return;
	}
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == channel_name) // CHANNEL ALREADY EXISTS --------------------------------------
		{
			std::cout << GREEN"Channel already exists\n" RESET;
			if (getChannels()[i].getUsers().find(users[client_fd].getNickName()) != getChannels()[i].getUsers().end())
			{
				std::cout << YELLOW"User already in channel\n" RESET;
				return;
			}
			if (getChannels()[i].getMemberLimit() && getChannels()[i].getUsers().size() == (size_t)getChannels()[i].getMemberLimit())
			{
				std::string rejectMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Channel " + getChannels()[i].getName() +" is full!\r\n";
				send(client_fd, rejectMessage.c_str(), rejectMessage.length(), 0);
				return;
			}
			if (getChannels()[i].getInviteOnly() && isInVector(getChannels()[i].getInviteList(), client_fd) == 0)
			{
				std::string rejectMessage = ":localhost 001 " + users[client_fd].getNickName() + " :You are not on the invite list for " + getChannels()[i].getName() +"!\r\n";
				send(client_fd, rejectMessage.c_str(), rejectMessage.length(), 0);
				return;
			}
			if (getChannels()[i].getKeyMode() && input_key != getChannels()[i].getKey())
			{
				std::cout << BLUE"input key: " << input_key << "\n" RESET;
				std::string rejectMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Wrong key for " + getChannels()[i].getName() +"!\r\n";
				send(client_fd, rejectMessage.c_str(), rejectMessage.length(), 0);
				return;
			}
			getChannels()[i].addMember(users[client_fd].getNickName(), client_fd);
			std::string welcomeMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Welcome to the IRC server\r\n";
			std::string topicMessage = ":localhost 332 " + users[client_fd].getNickName() + " " + channel_name + " :" + getChannels()[i].getTopic() + "\r\n";
			std::string namesReplyMessage = ":localhost 353 " + users[client_fd].getNickName() + " = " + channel_name + " :";
			for (std::map<std::string, int>::iterator j = getChannels()[i].getUsers().begin(); j != getChannels()[i].getUsers().end(); j++)
			{
				namesReplyMessage += j->first;
				if (i != getChannels()[i].getUsers().size() - 1)
					namesReplyMessage += " ";
			}
			namesReplyMessage += "\r\n";
			std::string endOfNamesMessage = ":localhost 366 "+ users[client_fd].getNickName() + channel_name + " :End of /NAMES list\r\n";
			std::string joinMessage = ":" + users[client_fd].getNickName() + " JOIN " + channel_name + "\r\n";
			send(client_fd, welcomeMessage.c_str(),welcomeMessage.length(), 0);
			send(client_fd, topicMessage.c_str(), topicMessage.length(),0);
			send(client_fd, namesReplyMessage.c_str(),namesReplyMessage.length(), 0);
			send(client_fd, endOfNamesMessage.c_str(), endOfNamesMessage.length(), 0);
			send(client_fd, joinMessage.c_str(), joinMessage.length(), 0);
            users[client_fd].addChannel(channel_name);
			return;
		}
	}
	std::cout << GREEN"Channel not found, creating new channel\n" RESET; // NEW CHANNEL -------------------------------------------
	getChannels().push_back(Channel(channel_name, "Welcome to " + channel_name + "!"));
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == channel_name)
		{
			getChannels()[i].addMember(users[client_fd].getNickName(), client_fd);
            getChannels()[i].addOp(client_fd);
			server_ops.push_back(client_fd);
			std::string welcomeMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Welcome to the IRC server\r\n";
			std::string topicMessage = ":localhost 332 " + users[client_fd].getNickName() + " " + channel_name + " :" + getChannels()[i].getTopic() + "\r\n";
			std::string namesReplyMessage = ":localhost 353 " + users[client_fd].getNickName() + " = " + channel_name + " :";
			for (std::map<std::string, int>::iterator j = getChannels()[i].getUsers().begin(); j != getChannels()[i].getUsers().end(); j++)
			{
				namesReplyMessage += j->first;
				if (i != getChannels()[i].getUsers().size() - 1)
					namesReplyMessage += " ";
			}
			namesReplyMessage += "\r\n";
			std::string endOfNamesMessage = ":localhost 366 "+ users[client_fd].getNickName() + channel_name + " :End of /NAMES list\r\n";
			std::string joinMessage = ":" + users[client_fd].getNickName() + " JOIN " + channel_name + "\r\n";
			send(client_fd, welcomeMessage.c_str(),welcomeMessage.length(), 0);
			send(client_fd, topicMessage.c_str(), topicMessage.length(),0);
			send(client_fd, namesReplyMessage.c_str(),namesReplyMessage.length(), 0);
			send(client_fd, endOfNamesMessage.c_str(), endOfNamesMessage.length(), 0);
			send(client_fd, joinMessage.c_str(), joinMessage.length(), 0);
            users[client_fd].addChannel(channel_name);
			return;
		}
	}
}

void Server::msg(std::string buf, int fd)
{
	std::string message = buf.substr(buf.find(':') + 1);
	message = message.substr(0, message.find('\n'));
	std::cout << BLUE"Message: " << message << "\n" RESET;

	if (buf[8] == '#') // Channel message
	{
		std::string channel_name = buf.substr(buf.find('#'));
		channel_name = channel_name.substr(0, channel_name.find(" "));
		for (size_t i = 0; i < getChannels().size(); i++)
		{
			if (getChannels()[i].getName() == channel_name)
			{
				for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin();
					 it != getChannels()[i].getUsers().end(); ++it)
				{
					if (it->second != fd)
					{
						std::cout << BLUE"Sending message to " << users[it->second].getNickName() << "\n" RESET;
						std::string msg = ":" + users[fd].getNickName() + " PRIVMSG " + channel_name + " :" + message + "\r\n";
						send(it->second, msg.c_str(), msg.length(), 0);
					}
				}
				return;
			}
		}
	}
	else // Private message
	{
		std::string receiver = buf.substr(8);
		receiver = receiver.substr(0, receiver.find(' '));
		for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->second.getNickName() == receiver)
			{
				std::cout << BLUE"Sending message to " << users[it->first].getNickName() << "\n" RESET;
				std::string msg = ":" + users[fd].getNickName() + "!" + users[fd].getUserName() + "@localhost" + " PRIVMSG " + users[it->first].getNickName() + " :" + message + "\n";
				send(it->first, msg.c_str(), msg.length(), 0);
				return;
			}
		}
	}
}

void Server::notice(std::string buf, int fd)
{
	std::string message = buf.substr(buf.find(':') + 1);
	message = message.substr(0, message.find('\n'));
	std::cout << BLUE"Notice: " << message << "\n" RESET;

	if (buf[7] == '#') // Channel message
	{
		std::string channel_name = buf.substr(buf.find('#'));
		channel_name = channel_name.substr(0, channel_name.find(' '));
		for (size_t i = 0; i < getChannels().size(); i++)
		{
			if (getChannels()[i].getName() == channel_name)
			{
				for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin();
					 it != getChannels()[i].getUsers().end(); ++it)
				{
					if (it->second != fd)
					{
						std::cout << "Sending notice to " << users[it->second].getNickName() << "\n";
						std::string msg = ":" + users[fd].getNickName() + " NOTICE " + channel_name + " :" + message + "\r\n";
						send(it->second, msg.c_str(), msg.length(), 0);
					}
				}
				return;
			}
		}
	}
	else // Private message
	{
		std::string receiver = buf.substr(7);
		receiver = receiver.substr(0, receiver.find(' '));
		std::cout << BLUE"Receiver: " << receiver << "\n" RESET;
		for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->second.getNickName() == receiver)
			{
				std::cout << BLUE"Sending notice to " << users[it->first].getNickName() << "\n" RESET;
				std::string msg = ":" + users[fd].getNickName() + " NOTICE " + users[it->first].getNickName() + " :" + message + "\n";
				send(it->first, msg.c_str(), msg.length(), 0);
				return;
			}
		}
	}
}

void Server::part(std::string buf, int fd)
{
	std::string channel_name = buf.substr(buf.find('#'));
	channel_name = channel_name.substr(0, channel_name.find(' '));
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == channel_name)
		{
			std::string partMessage = ":" + users[fd].getNickName() + " PART " + channel_name + "\r\n";
			send(fd, partMessage.c_str(), partMessage.length(), 0);

			std::string leaver = users[fd].getNickName();
			getChannels()[i].removeMember(leaver);
			for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin(); it != getChannels()[i].getUsers().end(); ++it)
			{
				if (it->second != fd)
				{
					std::cout << BLUE"Sending parting message to " << users[it->second].getNickName() << "\n" RESET;
					std::string msg = ":server PRIVMSG " + channel_name + " :" + leaver + " left the channel." + "\r\n";
					send(it->second, msg.c_str(), msg.length(), 0);
				}
			}
			return;
		}
	}
}

void Server::oper(std::string buf, int fd)
{
    std::string target = buf.substr(buf.find("OPER") + 5);
    std::string input_pass = target.substr(target.find(' ') + 1);
    input_pass = input_pass.substr(0, input_pass.find("\r\n"));
    target = target.substr(0, target.find(' '));

    std::cout << GREEN"target: " << target << "|\n" RESET;
    std::cout << BLUE"expected: " << users[fd].getUserName() << "|\n" RESET;

    std::cout << GREEN"input pass: " << input_pass << "|\n" RESET;
    std::cout << BLUE"expected: " << oper_auth << "|\n" RESET;

    if (target == users[fd].getUserName() && input_pass == oper_auth)
    {
        server_ops.push_back(fd);
        send(fd, "You are now a server operator.\r\n", 32, 0);
    }
    else
    {
        std::string msg = ":localhost 464 " + users[fd].getNickName() + " :Password incorrect\r\n";
        send(fd, msg.c_str(), 50, 0);
    }
}

void Server::kick(std::string buf, int fd)
{
    std::string channel_name = buf.substr(buf.find('#'));
    std::string target_user = channel_name.substr(channel_name.find(' ') + 1);
    target_user = target_user.substr(0, target_user.find("\r\n"));
    channel_name = channel_name.substr(0, channel_name.find(' '));

    for (size_t i = 0; i < getChannels().size(); i++)
    {
        if (getChannels()[i].getName() == channel_name)
        {
			if (!(isInVector(getChannels()[i].getOps(), fd)))
			{
				std::string msg = ":server PRIVMSG " + channel_name + " :You're not a channel operator!\r\n";
				send(fd, msg.c_str(), msg.length(), 0);
				return;
			}
			if (getChannels()[i].getUsers().find(target_user) != getChannels()[i].getUsers().end())
			{
				std::string kickMessage = ":" + users[fd].getNickName() + " KICK " + channel_name + " " + target_user + "\r\n";
				send(getChannels()[i].getUsers()[target_user], kickMessage.c_str(), kickMessage.length(), 0);

				std::string op = users[fd].getNickName();
				for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin(); it != getChannels()[i].getUsers().end(); ++it)
				{
					if (it->second != getChannels()[i].getUsers()[target_user])
					{
						std::cout << BLUE"Sending parting message to " GREEN << users[it->second].getNickName() << "\n" RESET;
						std::string msg = ":server PRIVMSG " + channel_name + " :" + target_user + " was kicked by " + op + "\r\n";
						send(it->second, msg.c_str(), msg.length(), 0);
					}
				}
				getChannels()[i].removeInvite(clientFd(target_user));
				getChannels()[i].removeMember(target_user);
				return;
			}
        }
    }
}

void Server::invite(std::string buf, int fd)
{
	std::string target_user = buf.substr(buf.find("INVITE") + 7);
	std::string channel_name = target_user.substr(target_user.find(' ') + 1);
	channel_name = channel_name.substr(0, channel_name.find("\r\n"));
	target_user = target_user.substr(0, target_user.find(' '));

	std::cout << BLUE"target user: " << target_user << "|\n" RESET;
	std::cout << BLUE"channel name: " << channel_name << "|\n" RESET;

	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == channel_name)
		{
			if (!(isInVector(getChannels()[i].getOps(), fd)))
			{
				std::string msg = ":server PRIVMSG " + channel_name + " :You're not a channel operator!\r\n";
				send(fd, msg.c_str(), msg.length(), 0);
				return;
			}
			if (clientFd(target_user) != -1)
			{
				std::string inviteMessage = ":" + users[fd].getNickName() + " INVITE " + target_user + " " + channel_name + "\r\n";
				send(clientFd(target_user), inviteMessage.c_str(), inviteMessage.length(), 0);
				getChannels()[i].getInviteList().push_back(clientFd(target_user));
				getChannels()[i].printVectorInt(getChannels()[i].getInviteList());
				return;
			}
		}
	}
}

void Server::topic(std::string buf, int fd)
{
	std::string channel_name = buf.substr(buf.find("TOPIC") + 6);
	std::string topic;
	if (channel_name.find(':') != std::string::npos)
	{
		channel_name = channel_name.substr(0, channel_name.find(':') - 1);
		topic = buf.substr(buf.find(':') + 1, buf.find("\r\n") - buf.find(':'));
	}
	else
	{
		channel_name = channel_name.substr(0, channel_name.find("\r\n"));
		topic = "";
	}
	size_t i;
	std::cout << BLUE"channel name: " << channel_name << "|\n" RESET;
	std::cout << BLUE"input topic: " << topic << "|\n" RESET;
	for (i = 0; i < getChannels().size(); i++)
		if (getChannels()[i].getName() == channel_name)
			break;
	std::cout << BLUE"Current topic: " << getChannels()[i].getTopic() << "\n" RESET;
	if (topic.empty() && i < getChannels().size())
	{
		std::string msg = ":server PRIVMSG " + channel_name + " :" + getChannels()[i].getTopic() + "\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
		return;
	}
	if ((isInVector(getChannels()[i].getOps(), fd) || !getChannels()[i].getTopicMode()) && !topic.empty())
	{
		getChannels()[i].changeTopic(topic);
		std::string msg = ":server PRIVMSG " + channel_name + " :Topic changed to -> " + getChannels()[i].getTopic() + "\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
	}
	else
	{
		std::string msg = ":server PRIVMSG " + channel_name + " :You're not a channel operator!\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
	}
}

void Server::mode(std::string buf, int fd)
{
	std::string target = buf.substr(buf.find("MODE") + 5);
	std::string mode;
	if (target.find(" +") != std::string::npos || target.find(" -") != std::string::npos)
	{
		if (target.find('+') != std::string::npos)
		{
			target = target.substr(0, target.find('+') - 1);
			mode = buf.substr(buf.find('+'), buf.find("\r\n") - buf.find('+'));
		}
		else
		{
			target = target.substr(0, target.find('-') - 1);
			mode = buf.substr(buf.find('-'), buf.find("\r\n") - buf.find('-'));
		}
	}
	else
	{
		target = target.substr(0, target.find("\r\n") - 1);
		mode = "";
	}
	std::cout << BLUE"target: " << target << "|\n" RESET;
	std::cout << BLUE"mode: " << mode << "|\n" RESET;

	size_t i;
	if (target.find('#') != std::string::npos)
		for (i = 0; i < getChannels().size(); i++)
			if (getChannels()[i].getName() == target)
				break;
	if (mode.empty() && target[0] != ' ')
	{
		if (target.find('#') != std::string::npos)
		{
			std::string msg = ":server PRIVMSG " + target + " :Channel modes: " + getChannels()[i].getModes() + "\r\n";
			send(fd, msg.c_str(), msg.length(), 0);
			return;
		}
		else
		{
			std::string user_mode;
			if (isInVector(server_ops, fd))
				user_mode = "+o";
			else
				user_mode = "-o";
			std::string msg = target + "modes: " + user_mode + "\r\n";
			send(fd, msg.c_str(), msg.length(), 0);
			return;
		}
	}
	else if (target[0] != ' ' && i < getChannels().size())
	{
		if (isInVector(getChannels()[i].getOps(), fd) || isInVector(server_ops, fd))
		{
			std::string mode_arg = mode.substr(mode.find(' ') + 1, mode.find("\r\n") - mode.find(' '));
			std::cout << BLUE"mode_arg: " << mode_arg << "|\n" RESET;
			if (mode.find("+i") != std::string::npos)
				getChannels()[i].changeInviteMode(true);
			else if (mode.find("-i") != std::string::npos)
				getChannels()[i].changeInviteMode(false);
			else if (mode.find("+t") != std::string::npos)
				getChannels()[i].changeTopicMode(true);
			else if (mode.find("-t") != std::string::npos)
				getChannels()[i].changeTopicMode(false);
			else if (mode.find("+k") != std::string::npos)
			{
				getChannels()[i].changeKeyMode(true);
				if (!mode_arg.empty() && validate_input((char *)"4242", (char *)mode_arg.c_str()))
					getChannels()[i].changeKey(mode_arg);
				else
				{
					std::string msg = ":server PRIVMSG " + target + " :You must specify an alphanumerical key!(at most 8 chars)\r\n";
					send(fd, msg.c_str(), msg.length(), 0);
					return;
				}
			}
			else if (mode.find("-k") != std::string::npos)
				getChannels()[i].changeKeyMode(false);
			else if (mode.find("+l") != std::string::npos)
			{
				if (!mode_arg.empty())
				{
					int numerical_arg = std::atoi(mode_arg.c_str());
					if (numerical_arg > 0 && numerical_arg < 100)
						getChannels()[i].changeMemberLimit(numerical_arg);
					else
					{
						std::string msg = ":server PRIVMSG " + target + " :Limit must be between 0-100!\r\n";
						send(fd, msg.c_str(), msg.length(), 0);
						return;
					}
				}
				else
				{
					std::string msg = ":server PRIVMSG " + target + " :You must specify a limit!\r\n";
					send(fd, msg.c_str(), msg.length(), 0);
					return;
				}
			}
			else if (mode.find("-l") != std::string::npos)
				getChannels()[i].changeMemberLimit(false);
			else if (mode.find("+o") != std::string::npos)
			{
				if (!mode_arg.empty())
					getChannels()[i].addOp(clientFd(mode_arg));
				else
				{
					std::string msg = ":server PRIVMSG " + target + " :You must specify a user!\r\n";
					send(fd, msg.c_str(), msg.length(), 0);
					return;
				}
			}
			else if (mode.find("-o") != std::string::npos)
				getChannels()[i].removeOp(clientFd(mode_arg));
			return;
		}
		else
		{
			std::string msg = ":server PRIVMSG " + target + " :You're not a channel operator!\r\n";
			send(fd, msg.c_str(), msg.length(), 0);
			return;
		}
	}
}
