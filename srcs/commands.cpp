//
// Created by rdas-nev on 5/18/23.
//

#include "../inc/ft_irc.hpp"

void Server::join(char *buffer, int client_fd)
{
	std::string channel_name;
	if (((std::string) buffer).find('#') != std::string::npos)
	{
		channel_name = ((std::string) buffer).substr(((std::string) buffer).find('#'));
		channel_name = channel_name.substr(0, channel_name.find('\n') - 1);
	}
	else
	{
		send(client_fd, "error on command\r\n", 18, 0);
		return;
	}
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == channel_name)
		{
			std::cout << "Channel already exists\n";
			if (getChannels()[i].getUsers().find(users[client_fd].getNickName()) != getChannels()[i].getUsers().end())
			{
				std::cout << "User already in channel\n";
				return;
			}
			getChannels()[i].printVectorInt(getChannels()[i].getInviteList());
			if (getChannels()[i].getInviteOnly() && getChannels()[i].isInVector(getChannels()[i].getInviteList(), client_fd) == 0)
			{
				std::string rejectMessage = ":localhost 001 " + users[client_fd].getNickName() + " :You are not on the invite list for " + getChannels()[i].getName() +"!\r\n";
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
	std::cout << "Channel not found, creating new channel\n";
	Channel newChannel(channel_name, "Welcome to " + channel_name + "!", 10, 1);
	getChannels().push_back(newChannel);
	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == channel_name)
		{
			getChannels()[i].addMember(users[client_fd].getNickName(), client_fd);
            getChannels()[i].addOp(client_fd);
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
	message = message.substr(0, message.find('\n') - 1);
	std::cout << "Message: " << message << "\n";

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
						std::cout << "Sending message to " << users[it->second].getNickName() << "\n";
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
				std::cout << "Sending message to " << users[it->first].getNickName() << "\n";
                std::cout << users[fd].getNickName() << " " << users[fd].getUserName() << "\n";
				std::string msg = ":" + users[fd].getNickName() + "!" + users[fd].getNickName() + "@localhost" + " PRIVMSG " + users[it->first].getNickName() + " :" + message + "\n";
				send(it->first, msg.c_str(), msg.length(), 0);
				return;
			}
		}
	}
}

void Server::part(std::string buf, int fd)
{
	std::string channel_name = buf.substr(buf.find('#'));
	channel_name = channel_name.substr(0, channel_name.find(" "));
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
					std::cout << "Sending parting message to " << users[it->second].getNickName() << "\n";
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

    std::cout << "target: " << target << "|\n";
    std::cout << "expected: " << users[fd].getUserName() << "|\n";

    std::cout << "input pass: " << input_pass << "|\n";
    std::cout << "expected: " << oper_auth << "|\n";

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
			if (!(getChannels()[i].isInVector(getChannels()[i].getOps(), fd)))
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
						std::cout << "Sending parting message to " << users[it->second].getNickName() << "\n";
						std::string msg = ":server PRIVMSG " + channel_name + " :" + target_user + " was kicked by " + op + "\r\n";
						send(it->second, msg.c_str(), msg.length(), 0);
					}
				}
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

	std::cout << "target user: " << target_user << "|\n";
	std::cout << "channel name: " << channel_name << "|\n";

	for (size_t i = 0; i < getChannels().size(); i++)
	{
		if (getChannels()[i].getName() == channel_name)
		{
			if (!(getChannels()[i].isInVector(getChannels()[i].getOps(), fd)))
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

//void Server::topic(std::string buf, int fd)
//{
//
//}