//
// Created by rdas-nev on 6/2/23.
//

#include "../inc/ft_irc.hpp"

bool validate_password(std::string const &password)
{
	std::ifstream file;
	file.open(".request.txt");
	if (!file.is_open())
	{
		std::cerr << RED"Error opening request file\n" RESET;
		return false;
	}
	std::string line;
	while (std::getline(file, line))
	{
		std::string check = line.substr(line.find("PASS")+5);
		if (check[check.length() - 1] == '\n' || check[check.length() - 1] == '\r')
			check = check.substr(0, check.length() - 1);
		if (check == password)
		{
			return true;
		}
	}
	return false;
}

bool validate_input(char *port, char *password)
{
	if (std::atoi(port) < 1024 || std::atoi(port) > 65535)
	{
		std::cerr << RED"Invalid port number\n" RESET;
		return false;
	}
	if (((std::string)password).length() < 1 && ((std::string)password).length() > 8)
	{
		std::cerr << RED"Password must be at most 8 characters long\n" RESET;
		return false;
	}
	if (((std::string)password).find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos)
	{
		std::cerr << RED"Password must contain only alphanumeric characters\n" RESET;
		return false;
	}
	return true;
}

bool Server::validateNickUser(std::string &name, int client_fd, int flag)
{
	if (name.length() < 5)
	{
		send(client_fd, "Nickname or username too short\r\n", 32, 0);
		return false;
	}
	if (name.length() > 12)
	{
		send(client_fd, "Nickname or username too long\r\n", 31, 0);
		return false;
	}
	if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-") != std::string::npos)
	{
		send(client_fd, "Nickname or username contains invalid characters\r\n", 51, 0);
		return false;
	}
	for (size_t i = 0; i < users.size(); i++)
	{
		if (users[(int)i].getNickName() == name && flag == 0)
		{
			send(client_fd, "Nickname already in use\r\n", 25, 0);
			return false;
		}
	}
	return true;
}

bool isInVector(const std::vector<int>& vec, int target)
{
	for (size_t num = 0; num < vec.size(); num++)
	{
		if (vec[num] == target)
		{
			return true;
		}
	}
	return false;
}