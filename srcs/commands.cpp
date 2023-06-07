//
// Created by rdas-nev on 5/18/23.
//

#include "../inc/ft_irc.hpp"

void Server::commandmaster(std::string buffer, int client_fd)
{
	std::istringstream ss(buffer);
	std::vector<std::string> lines;
	std::string line;

	while (std::getline(ss, line))
		lines.push_back(line);

	for (size_t i = 0; i < lines.size(); i++)
		commands(lines[i], client_fd);
}

void Server::commands(std::string buffer, int client_fd)
{
	if (buffer[0] == '\n' || buffer[0] == '\r')
		return;
	std::stringstream ss(buffer);
	std::vector<std::string> blocks;
	std::string block;

	while (ss >> block)
	{
		if (block[0] == ':' && !blocks[0].empty())
		{
			blocks.push_back(buffer.substr(buffer.find(':')));
			break;
		}
		blocks.push_back(block);
	}
	if (blocks[0] == "PASS")
	{
		if (pass(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct PASS format: PASS <password>\r\n", 48, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (users[client_fd].getPassAuth() == '0') // If password is not authenticated
	{
		send(client_fd, "\nerror: introduce password first.\r\n", 35, 0);
		send(client_fd, "\n", 1, 0);
		return;
	}
	else if (blocks[0] == "NICK")
	{
		if (nick(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct NICK format: NICK <nickname>\r\n", 48, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "USER")
	{
		if (user(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct USER format: USER <username> <hostname> <servername> :<realname>\r\n", 84, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (!users[client_fd].isAuthenticated()) // If user is not authenticated
	{
		send(client_fd, "\nerror: not authenticated.\n", 26, 0);
		send(client_fd, "\n", 1, 0);
		return;
	}
	else if (blocks[0] == "JOIN")
	{
		if (join(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct JOIN format: JOIN <#channel>\r\n", 48, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "PRIVMSG")
	{
		if(msg(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct PRIVMSG format: PRIVMSG <#channel/nickname> :<message>\r\n", 75, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "NOTICE")
	{
		if(notice(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct NOTICE format: NOTICE <#channel/nickname> :<message>\r\n", 75, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "PART")
	{
		if(part(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct PART format: PART <#channel>\r\n", 48, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "OPER")
	{
		if(oper(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct OPER format: OPER <password>\r\n", 48, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "KICK")
	{
		if(kick(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct KICK format: KICK <#channel> <nickname>\r\n", 59, 0);
			send (client_fd, "error: correct KICK format: KICK <#channel> <nickname> :<comment>\r\n", 69, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "INVITE")
	{
		if(invite(blocks, client_fd) == -1)
		{
			send(client_fd, "\nerror: correct INVITE format: INVITE <nickname> <#channel>\r\n", 63, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "TOPIC")
	{
		if(topic(blocks, client_fd) == -1)
		{
			send(client_fd, "error: correct TOPIC format: TOPIC <#channel> :<topic>\r\n", 56, 0);
			send (client_fd, "error: correct TOPIC format: TOPIC <#channel>\r\n", 47, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "MODE")
	{
		if(mode(blocks, client_fd) == -1)
		{
			// Display info about MODE
			send(client_fd, "error: correct MODE format: MODE <nickname>\r\n", 45, 0);
			send(client_fd, "error: correct MODE format: MODE <#channel>\r\n", 45, 0);
			// Change mode of channel
			send(client_fd, "error: correct MODE format: MODE <#channel> <mode>\r\n", 52, 0);
			// Change mode of user on channel
			send(client_fd, "error: correct MODE format: MODE <#channel> <mode> <nickname>\r\n", 63, 0);
			send(client_fd, "error: correct MODE format: MODE <#channel> <mode> <argument>\r\n", 63, 0);
			send(client_fd, "\n", 1, 0);
			return;
		}
	}
	else if (blocks[0] == "CAP" || blocks[0] == "WHO")
		return;
	else
	{
		std::cout << RED"error: command not found: " RESET << buffer <<std::endl;
		send(client_fd, "\nerror: command not found.\r\n", 27, 0);
		send(client_fd, "\n",1, 0);
	}
}

int Server::pass(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() != 2)
		return -1;
	if (users[fd].getPassAuth() == '1')
	{
		send(fd, "Password already set.\r\n", 24, 0);
		return 0;
	}
	if (blocks[1] == password)
	{
		users[fd].setAuthenticated('1', 0);
		send(fd, "Password correct.\r\n", 19, 0);
		return 0;
	}
	else
	{
		send(fd, "Invalid password, submit again please.\r\n", 40, 0);
		return 0;
	}
}

int Server::nick(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() != 2)
		return -1;
	if (!validateNickUser(blocks[1], fd, 0))
		return 0;
	users[fd].setNickName(blocks[1]);
	users[fd].setAuthenticated('1', 1);
	send(fd, "Your nickname has been set.\r\n", 29, 0);
	if (users[fd].isAuthenticated() && !users[fd].getAuthCount())
	{
		send(fd, "You are now authenticated.\r\n", 28, 0);
		users[fd].setAuthCount(1);
	}
	return 0;
}

int Server::user(std::vector<std::string> blocks, int fd)
{
	if (users[fd].getUserAuth() == '1')
	{
		send(fd, "User already set.\r\n", 19, 0);
		return 0;
	}
	if (blocks.size() != 5)
		return -1;
	if (blocks[4][0] != ':')
		return -1;
	else
		blocks[4] = blocks[4].substr(1);
	if (!validateNickUser(blocks[1], fd, 1))
		return 0;
	users[fd].setUserName(blocks[1]);
	users[fd].setAuthenticated('1', 2);
	send(fd, "Your username has been set.\r\n", 29, 0);
	if (users[fd].isAuthenticated() && !users[fd].getAuthCount())
	{
		send(fd, "You are now authenticated.\r\n", 28, 0);
		users[fd].setAuthCount(1);
	}
	return 0;
}

int Server::join(std::vector<std::string> blocks, int client_fd)
{
	std::string input_key;

	if (blocks.size() < 2)
		return -1;
    if (blocks[1][0] != '#')
	{
		send(client_fd, "error on command\r\n", 18, 0);
		send(client_fd, "\n",1, 0);
		return 0;
	}
	if (blocks.size() == 3)
		input_key = blocks[2];
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == blocks[1]) // CHANNEL ALREADY EXISTS --------------------------------------
		{
			std::cout << GREEN"Channel already exists\n" RESET;
			if (getChannels()[i].getUsers().find(users[client_fd].getNickName()) != getChannels()[i].getUsers().end())
			{
				std::cout << YELLOW"User already in channel\n" RESET;
				return 0;
			}
			if (getChannels()[i].getMemberLimit() && getChannels()[i].getUsers().size() == (size_t)getChannels()[i].getMemberLimit())
			{
				std::string rejectMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Channel " + getChannels()[i].getName() +" is full!\r\n";
				send(client_fd, rejectMessage.c_str(), rejectMessage.length(), 0);
				return 0;
			}
			if (getChannels()[i].getInviteOnly() && isInVector(getChannels()[i].getInviteList(), client_fd) == 0)
			{
				std::string rejectMessage = ":localhost 001 " + users[client_fd].getNickName() + " :You are not on the invite list for " + getChannels()[i].getName() +"!\r\n";
				send(client_fd, rejectMessage.c_str(), rejectMessage.length(), 0);
				return 0;
			}
			if (getChannels()[i].getKeyMode() && input_key != getChannels()[i].getKey())
			{
				std::cout << BLUE"input key: " << input_key << "\n" RESET;
				std::string rejectMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Wrong key for " + getChannels()[i].getName() +"!\r\n";
				send(client_fd, rejectMessage.c_str(), rejectMessage.length(), 0);
				return 0;
			}
			getChannels()[i].addMember(users[client_fd].getNickName(), client_fd);
			std::string welcomeMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Welcome to the IRC server\r\n";
			std::string topicMessage = ":localhost 332 " + users[client_fd].getNickName() + " " + blocks[1] + " :" + getChannels()[i].getTopic() + "\r\n";
			std::string namesReplyMessage = ":localhost 353 " + users[client_fd].getNickName() + " = " + blocks[1] + " :";
			for (std::map<std::string, int>::iterator j = getChannels()[i].getUsers().begin(); j != getChannels()[i].getUsers().end(); j++)
			{
				namesReplyMessage += j->first;
				if (i != getChannels()[i].getUsers().size() - 1)
					namesReplyMessage += " ";
			}
			namesReplyMessage += "\r\n";
			std::string endOfNamesMessage = ":localhost 366 "+ users[client_fd].getNickName() + blocks[1] + " :End of /NAMES list\r\n";
			std::string joinMessage = ":" + users[client_fd].getNickName() + " JOIN " + blocks[1] + "\r\n";
			send(client_fd, welcomeMessage.c_str(),welcomeMessage.length(), 0);
			send(client_fd, topicMessage.c_str(), topicMessage.length(),0);
			send(client_fd, namesReplyMessage.c_str(),namesReplyMessage.length(), 0);
			send(client_fd, endOfNamesMessage.c_str(), endOfNamesMessage.length(), 0);
			send(client_fd, joinMessage.c_str(), joinMessage.length(), 0);
            users[client_fd].addChannel(blocks[1]);
			return 0;
		}
	}
	std::cout << GREEN"Channel not found, creating new channel\n" RESET; // NEW CHANNEL -------------------------------------------
	getChannels().push_back(Channel(blocks[1], "Welcome to " + blocks[1] + "!"));
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == blocks[1])
		{
			getChannels()[i].addMember(users[client_fd].getNickName(), client_fd);
            getChannels()[i].addOp(client_fd);
			server_ops.push_back(client_fd);
			std::string welcomeMessage = ":localhost 001 " + users[client_fd].getNickName() + " :Welcome to the IRC server\r\n";
			std::string topicMessage = ":localhost 332 " + users[client_fd].getNickName() + " " + blocks[1] + " :" + getChannels()[i].getTopic() + "\r\n";
			std::string namesReplyMessage = ":localhost 353 " + users[client_fd].getNickName() + " = " + blocks[1] + " :";
			for (std::map<std::string, int>::iterator j = getChannels()[i].getUsers().begin(); j != getChannels()[i].getUsers().end(); j++)
			{
				namesReplyMessage += j->first;
				if (i != getChannels()[i].getUsers().size() - 1)
					namesReplyMessage += " ";
			}
			namesReplyMessage += "\r\n";
			std::string endOfNamesMessage = ":localhost 366 "+ users[client_fd].getNickName() + blocks[1] + " :End of /NAMES list\r\n";
			std::string joinMessage = ":" + users[client_fd].getNickName() + " JOIN " + blocks[1] + "\r\n";
			send(client_fd, welcomeMessage.c_str(),welcomeMessage.length(), 0);
			send(client_fd, topicMessage.c_str(), topicMessage.length(),0);
			send(client_fd, namesReplyMessage.c_str(),namesReplyMessage.length(), 0);
			send(client_fd, endOfNamesMessage.c_str(), endOfNamesMessage.length(), 0);
			send(client_fd, joinMessage.c_str(), joinMessage.length(), 0);
            users[client_fd].addChannel(blocks[1]);
			return 0;
		}
	}
	return 0;
}

int Server::msg(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() < 3)
		return -1;
	std::stringstream message_stream;
	if (blocks[2][0] != ':')
		return -1;
	else
	{
		blocks[2] = blocks[2].substr(1);
		for (size_t i = 2; i < blocks.size(); i++)
		{
			message_stream  << blocks[i];
			if (i != blocks.size() - 1)
				message_stream << " ";
		}
	}
	std::cout << BLUE"Message: " RESET<< message_stream.str() << "\n" ;
	if (blocks[1][0] == '#') // Channel message
	{
		for (size_t i = 0; i < getChannels().size(); i++)
		{
			if (getChannels()[i].getName() == blocks[1])
			{
				for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin();
					 it != getChannels()[i].getUsers().end(); ++it)
				{
					if (it->second != fd)
					{
						std::cout << BLUE"Sending message to " RESET << users[it->second].getNickName() << "\n";
						std::string message = message_stream.str();
						std::string msg = ":" + users[fd].getNickName() + " PRIVMSG " + blocks[1] + " :" + message + "\r\n";
						send(it->second, msg.c_str(), msg.length(), 0);
					}
				}
				return 0;
			}
		}
		std::string no_target = ":localhost 401 " + users[fd].getNickName() + " " + blocks[1] + " :No such nick/channel\r\n";
		send(fd, no_target.c_str(), no_target.length(), 0);
	}
	else // Private message
	{
		std::cout << BLUE"Receiver: " << blocks[1] << "|\n" RESET;
		for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->second.getNickName() == blocks[1])
			{
				std::string msg = ":" + users[fd].getNickName() + "!" + users[fd].getUserName() + "@localhost" + " PRIVMSG " + users[it->first].getNickName() + " :" + message_stream.str() + "\n";
				send(it->first, msg.c_str(), msg.length(), 0);
				return 0;
			}
		}
		std::string no_target = ":localhost 401 " + users[fd].getNickName() + " " + blocks[1] + " :No such nick/channel\r\n";
		send(fd, no_target.c_str(), no_target.length(), 0);
	}
	return 0;
}

int Server::notice(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() < 3)
		return -1;
	std::stringstream message_stream;
	if (blocks[2][0] != ':')
		return -1;
	else
	{
		blocks[2] = blocks[2].substr(1);
		for (size_t i = 2; i < blocks.size(); i++)
		{
			message_stream  << blocks[i];
			if (i != blocks.size() - 1)
				message_stream << " ";
		}
	}
	std::cout << BLUE"Notice: " << message_stream.str() << "\n" RESET;
	if (blocks[1][0] == '#') // Channel message
	{
		for (size_t i = 0; i < getChannels().size(); i++)
		{
			if (getChannels()[i].getName() == blocks[1])
			{
				for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin();
					 it != getChannels()[i].getUsers().end(); ++it)
				{
					if (it->second != fd)
					{
						std::cout << "Sending notice to " << users[it->second].getNickName() << "\n";
						std::string msg = ":" + users[fd].getNickName() + " NOTICE " + blocks[1] + " :" + message_stream.str() + "\r\n";
						send(it->second, msg.c_str(), msg.length(), 0);
					}
				}
				return 0;
			}
		}
		std::string no_target = ":localhost 401 " + users[fd].getNickName() + " " + blocks[1]+ " :No such nick/channel\r\n";
		send(fd, no_target.c_str(), no_target.length(), 0);
	}
	else // Private message
	{
		std::cout << BLUE"Receiver: " << blocks[1] << "\n" RESET;
		for (std::map<int, User>::iterator it = users.begin(); it != users.end(); ++it)
		{
			if (it->second.getNickName() == blocks[1])
			{
				std::string msg = ":" + users[fd].getNickName() + " NOTICE " + users[it->first].getNickName() + " :" + message_stream.str() + "\n";
				send(it->first, msg.c_str(), msg.length(), 0);
				return 0;
			}
		}
		std::string no_target = ":localhost 401 " + users[fd].getNickName() + " " + blocks[1] + " :No such nick/channel\r\n";
		send(fd, no_target.c_str(), no_target.length(), 0);
	}
	return 0;
}

int Server::part(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() != 3 || blocks[1][0] != '#' || blocks[2][0] != ':')
		return -1;
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == blocks[1])
		{
			std::string partMessage = ":" + users[fd].getNickName() + " PART " + blocks[1] + "\r\n";
			send(fd, partMessage.c_str(), partMessage.length(), 0);

			std::string leaver = users[fd].getNickName();
			getChannels()[i].removeMember(leaver);
			for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin(); it != getChannels()[i].getUsers().end(); ++it)
			{
				if (it->second != fd)
				{
					std::cout << BLUE"Sending parting message to " RESET << users[it->second].getNickName() << "\n";
					std::string msg = ":server PRIVMSG " + blocks[1] + " :" + leaver + " left the channel." + "\r\n";
					send(it->second, msg.c_str(), msg.length(), 0);
				}
			}
			return 0;
		}
	}
	return 0;
}

int Server::oper(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() != 3)
		return -1;
    std::cout << GREEN"target: " RESET << blocks[1] << "|\n";
    std::cout << BLUE"expected: " RESET << users[fd].getUserName() << "|\n";
    std::cout << GREEN"input pass: " RESET << blocks[2] << "|\n";
    std::cout << BLUE"expected: " RESET << oper_auth << "|\n";

    if (blocks[1] == users[fd].getUserName() && blocks[2] == oper_auth)
    {
        server_ops.push_back(fd);
        send(fd, "You are now a server operator.\r\n", 32, 0);
    }
    else
    {
        std::string msg = ":localhost 464 " + users[fd].getNickName() + " :Password incorrect\r\n";
        send(fd, msg.c_str(), 50, 0);
    }
	return 0;
}

int Server::kick(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() < 3 || blocks[1][0] != '#')
		return -1;
	std::cout << GREEN"channel_name: " RESET << blocks[1] << "|\n";
	std::cout << BLUE"target_user: " RESET << blocks[2] << "|\n";

    for (size_t i = 0; i < getChannels().size(); i++)
    {
        if (getChannels()[i].getName() == blocks[1])
        {
			if (!(isInVector(getChannels()[i].getOps(), fd)))
			{
				std::string msg = ":server PRIVMSG " + blocks[1] + " :You're not a channel operator!\r\n";
				send(fd, msg.c_str(), msg.length(), 0);
				return 0;
			}
			if (getChannels()[i].getUsers().find(blocks[2]) != getChannels()[i].getUsers().end())
			{
				std::string kickMessage = ":" + users[fd].getNickName() + " KICK " + blocks[1] + " " + blocks[2];
				if (blocks.size() > 3)
				{
					blocks[3] = blocks[3].substr(1);
					kickMessage += " :" + blocks[3] + "\r\n";
				}
				else
					kickMessage += "\r\n";
				send(getChannels()[i].getUsers()[blocks[2]], kickMessage.c_str(), kickMessage.length(), 0);

				std::string op = users[fd].getNickName();
				for (std::map<std::string, int>::const_iterator it = getChannels()[i].getUsers().begin(); it != getChannels()[i].getUsers().end(); ++it)
				{
					if (it->second != getChannels()[i].getUsers()[blocks[2]])
					{
						std::cout << BLUE"Sending parting message to " GREEN << users[it->second].getNickName() << "\n" RESET;
						std::string msg = ":server PRIVMSG " + blocks[1] + " :" + blocks[2] + " was kicked by " + op + "\r\n";
						send(it->second, msg.c_str(), msg.length(), 0);
					}
				}
				getChannels()[i].removeInvite(clientFd(blocks[2]));
				getChannels()[i].removeMember(blocks[2]);
				return 0;
			}
        }
    }
	return 0;
}

int Server::invite(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() != 3 || blocks[2][0] != '#')
		return -1;
	std::cout << BLUE"target user: " << blocks[1] << "|\n" RESET;
	std::cout << BLUE"channel name: " << blocks[2] << "|\n" RESET;

	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == blocks[2])
		{
			if (!(isInVector(getChannels()[i].getOps(), fd)))
			{
				std::string msg = ":server PRIVMSG " + blocks[2] + " :You're not a channel operator!\r\n";
				send(fd, msg.c_str(), msg.length(), 0);
				return 0;
			}
			if (clientFd(blocks[1]) != -1)
			{
				std::string inviteMessage = ":" + users[fd].getNickName() + " INVITE " + blocks[1] + " " + blocks[2] + "\r\n";
				send(clientFd(blocks[1]), inviteMessage.c_str(), inviteMessage.length(), 0);
				getChannels()[i].getInviteList().push_back(clientFd(blocks[1]));
				getChannels()[i].printVectorInt(getChannels()[i].getInviteList());
				return 0;
			}
		}
	}
	return 0;
}

int Server::topic(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() < 2 || blocks[1][0] != '#')
		return -1;
	std::string topic;
	if (blocks.size() > 2 && blocks[2][0] == ':' && blocks[2].length() > 1)
	{
		blocks[2] = blocks[2].substr(1);
		topic = blocks[2];
	}
	size_t i;
	std::cout << BLUE"channel name: " << blocks[1] << "|\n" RESET;
	std::cout << BLUE"input topic: " << topic << "|\n" RESET;
	for (i = 0; i < getChannels().size(); i++)
		if (getChannels()[i].getName() == blocks[1])
			break;
	std::cout << BLUE"Current topic: " << getChannels()[i].getTopic() << "\n" RESET;
	if (topic.empty() && i < getChannels().size())
	{
		std::string msg = ":server PRIVMSG " + blocks[1] + " :" + getChannels()[i].getTopic() + "\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
		return 0;
	}
	if ((isInVector(getChannels()[i].getOps(), fd) || !getChannels()[i].getTopicMode()) && !topic.empty())
	{
		getChannels()[i].changeTopic(topic);
		std::string msg = ":server PRIVMSG " + blocks[1] + " :Topic changed to -> " + getChannels()[i].getTopic() + "\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
	}
	else
	{
		std::string msg = ":server PRIVMSG " + blocks[1] + " :You're not a channel operator!\r\n";
		send(fd, msg.c_str(), msg.length(), 0);
	}
	return 0;
}

int Server::mode(std::vector<std::string> blocks, int fd)
{
	if (blocks.size() < 2 || blocks.size() > 4)
		return -1;

	if (blocks.size() == 2) // Display info about MODE on Channel or User
	{
		if (blocks[1][0] == '#')
		{
			for (size_t i = 0; i < getChannels().size(); i++)
				if (getChannels()[i].getName() == blocks[1])
				{
					std::string msg = ":server PRIVMSG " + blocks[1] + " :Channel modes: " + getChannels()[i].getModes() + "\r\n";
					send(fd, msg.c_str(), msg.length(), 0);
					return 0;
				}
		}
		else if (clientFd(blocks[1]) != -1)
		{
			std::string user_mode;
			if (isInVector(server_ops, fd))
				user_mode = "+o";
			else
				user_mode = "-o";
			std::string msg = blocks[1] + " modes: " + user_mode + "\r\n";
			send(fd, msg.c_str(), msg.length(), 0);
			return 0;
		}
		else
			send(fd, "error: Channel not found!\r\n", 27, 0);
	}
	else if (!isInVector(server_ops, fd))
	{
		std::string message = ":server PRIVMSG " + blocks[1] + " :You're not a server operator!\r\n";
		send(fd, message.c_str(), message.length(), 0);
		return 0;
	}
	else if (blocks.size() == 3)  // Change mode of channel
	{
		if (blocks[1][0] != '#')
			return -1;
		else
		{
			for (size_t i = 0; i < getChannels().size(); i++)
				if (getChannels()[i].getName() == blocks[1])
				{
					if (blocks[2] == "+i")
						getChannels()[i].changeInviteMode(true);
					else if (blocks[2] == "-i")
						getChannels()[i].changeInviteMode(false);
					else if (blocks[2] == "+t")
						getChannels()[i].changeTopicMode(true);
					else if (blocks[2] == "-t")
						getChannels()[i].changeTopicMode(false);
					else if (blocks[2] == "-k")
						getChannels()[i].changeKeyMode(false);
					else if (blocks[2] == "-l")
						getChannels()[i].changeMemberLimit(false);
					else if (blocks[2] == "+k" || blocks[2] == "+l" || blocks[2] == "+o" || blocks[2] == "-o")
					{
						std::string msg = ":server PRIVMSG " + blocks[1] + " :Not enough arguments!\r\n";
						send(fd, msg.c_str(), msg.length(), 0);
					}
					else
						return -1;
					return 0;
				}
			send(fd, "error: Channel not found!\r\n", 27, 0);
		}
	}
	else if (blocks.size() == 4) // Change mode of user in channel
	{
		if (blocks[1][0] != '#')
			return -1;
		else
		{
			for (size_t i = 0; i < getChannels().size(); i++)
				if (getChannels()[i].getName() == blocks[1])
				{
					if (blocks[2] == "+k")
					{
						if (validate_input((char *)"4242", (char *)blocks[3].c_str()))
						{
							getChannels()[i].changeKeyMode(true);
							getChannels()[i].changeKey(blocks[3]);
						}
						else
						{
							std::string msg = ":server PRIVMSG " + blocks[1] + " :You must specify an alphanumerical key!(at most 8 chars)\r\n";
							send(fd, msg.c_str(), msg.length(), 0);
							return 0;
						}
					}
					else if (blocks[2] == "+l")
					{
						int numerical_arg = std::atoi(blocks[3].c_str());
						if (numerical_arg > 0 && numerical_arg < 100)
							getChannels()[i].changeMemberLimit(numerical_arg);
						else
						{
							std::string msg = ":server PRIVMSG " + blocks[1] + " :Limit must be between 0-100!\r\n";
							send(fd, msg.c_str(), msg.length(), 0);
							return 0;
						}
					}
					else if (blocks[2] == "+o")
					{
						if (clientFd(blocks[3]) != -1)
						{
							if (!isInVector(getChannels()[i].getOps(), clientFd(blocks[3])))
								getChannels()[i].addOp(clientFd(blocks[3]));
						}
						else
						{
							std::string msg = ":server PRIVMSG " + blocks[1] + " :No such nick!\r\n";
							send(fd, msg.c_str(), msg.length(), 0);
							return 0;
						}
					}
					else if (blocks[2] == "-o")
					{
						if (clientFd(blocks[3]) != -1)
						{
							if (isInVector(getChannels()[i].getOps(), clientFd(blocks[3])))
								getChannels()[i].removeOp(clientFd(blocks[3]));
						}
						else
						{
							std::string msg = ":server PRIVMSG " + blocks[1] + " :No such nick!\r\n";
							send(fd, msg.c_str(), msg.length(), 0);
							return 0;
						}
					}
					else
						return -1;
					return 0;
				}
		}
	}
	else
		return -1;
	return 0;
}
