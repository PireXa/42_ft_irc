//
// Created by rdas-nev on 4/19/23.
//

#include "../inc/ft_irc.hpp"

bool validate_password(std::string const &password)
{
    std::ifstream file;
    file.open(".request.txt");
    if (!file.is_open())
    {
        std::cerr << "Error opening password file\n";
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

void Server::setNICK(int fd)
{
	std::ifstream file;
	file.open(".request.txt");
	if (!file.is_open())
	{
		std::cerr << "Error opening password file\n";
		return;
	}
	std::string line;
	while (std::getline(file, line))
		if (line.substr(0, 5) == "NICK ")
			break;
	std::string nick = line.substr(line.find("NICK")+5);
	if (nick[nick.length() - 1] == '\n' || nick[nick.length() - 1] == '\r')
		nick = nick.substr(0, nick.length() - 1);
	getUsers()[fd].setNickName(nick);
}

void Server::setUSER(int fd)
{
    std::ifstream file;
    file.open(".request.txt");
    if (!file.is_open())
    {
        std::cerr << "Error opening password file\n";
        return;
    }
    std::string line;
    while (std::getline(file, line))
        if (line.substr(0, 5) == "USER ")
            break;
    std::string user = line.substr(line.find("USER") + 5);
    if (user[user.length() - 1] == '\n' || user[user.length() - 1] == '\r')
        user = user.substr(0, user.length() - 1);
    user = user.substr(0, user.find(' '));
    getUsers()[fd].setUserName(user);
    std::cout << "User " << user << " has joined the server\n";
}

bool validate_input(char *port, char *password)
{
	if (std::atoi(port) < 1024 || std::atoi(port) > 65535)
	{
		std::cerr << "Invalid port number\n";
		return false;
	}
	if (((std::string)password).length() < 1 && ((std::string)password).length() > 8)
	{
		std::cerr << "Password must be at most 8 characters long\n";
		return false;
	}
	if (((std::string)password).find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") != std::string::npos)
	{
		std::cerr << "Password must contain only alphanumeric characters\n";
		return false;
	}
	return true;
}

void Server::processClient(int client_fd, const std::string& password)
{
	std::cout << "Client checking in\n";
	size_t num_bytes = 0;
	char buffer[1024];
	char buffer2[1024];
	std::memset(buffer2, 0, sizeof(buffer2));
	const int timeoutMs = 5;  // Timeout in milliseconds

	while (!(((((std::string)buffer2).find("PASS") != std::string::npos) &&
			  (((std::string)buffer2).find("NICK") != std::string::npos) &&
			  (((std::string)buffer2).find("USER")) != std::string::npos)))
	{
		int epollResult = epoll_wait(epoll_fd, &event, 1, timeoutMs);
		if (epollResult == -1)
		{
			// Error occurred
			std::cerr << "Error in epoll_wait()" << std::endl;
			break;
		}
		else if (epollResult == 0)
		{
			// Timeout occurred
			std::cout << "Timeout occurred" << std::endl;
			return;
		}
		else
		{
			num_bytes += recv(client_fd, buffer, sizeof(buffer), 0);
			std::strcat(buffer2, buffer);
			std::memset(buffer, 0, sizeof(buffer));
		}
	}
	buffer2[num_bytes] = '\0';
	std::cout << "Received " << num_bytes << " bytes from client\n";
	std::cout << buffer2 << std::endl;

	std::ofstream request_file(".request.txt", std::ios::out | std::ios::binary);
	request_file.write(buffer2, static_cast<int>(num_bytes));
	request_file.close();
	if (validate_password(password))
	{
		send(client_fd, "Valid password, welcome.\r\n", 26, 0);
		std::cout << "Password validated.\n";
		setNICK(client_fd);
        setUSER(client_fd);
		users[client_fd].setAuthenticated('1', 0);
		users[client_fd].setAuthenticated('1', 1);
		users[client_fd].setAuthenticated('1', 2);
		request_file.clear();
		unlink(".request.txt");
		std::memset(buffer2, 0, sizeof(buffer2));
	}
	else
	{
		std::cout << "Password not validated.\n";
		send(client_fd, "Invalid password, goodbye.\r\n", 26, 0);
		close(client_fd);
		unlink(".request.txt");
		std::memset(buffer2, 0, sizeof(buffer2));
	}
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

int main(int ac, char **av)
{
	if (ac == 3)
	{
		try
		{
		// Validate input------------------------------------------------------------------------
			if (!validate_input(av[1], av[2]))
				return EXIT_FAILURE;
		// Run server---------------------------------------------------------------------------
			Server ircserver(std::atoi(av[1]), av[2]);
			ircserver.run();
		}
		catch (std::exception &e)
		{
			std::cout << e.what() << std::endl;
		}
	}
	else
	{
		std::cerr << "Usage: ./ft_irc <port> <password>\n";
		return EXIT_FAILURE;
	}
}
