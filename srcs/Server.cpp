//
// Created by rdas-nev on 4/19/23.
//

#include "../inc/Server.hpp"

// Constructor ----------------------------------------------------------------------------------------------------------
Server::Server(int port, char *pd) :  password((std::string)pd), oper_auth("operador"), channels()
{
// Create a socket && make it reusable ---------------------------------------------------------------------------------
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		std::cerr << "Error opening socket\n";
		throw std::exception();
	}
	int optval = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
				   sizeof(optval)) < 0) {
		std::cerr << "Error setting socket options\n";
		close(socket_fd);
		throw std::exception();
	}
// Bind the socket to the local address and port -----------------------------------------------------------------------
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	if (bind(socket_fd, (struct sockaddr *) &server_addr,
			 sizeof(server_addr)) == -1) {
		std::cerr << "Error binding socket to port " << port
				  << std::endl;
		close(socket_fd);
		throw std::exception();
	}
	// Create an epoll instance -------------------------------------------------------------------------------------------
	this->epoll_fd = epoll_create1(0);
	if (epoll_fd < 0) {
		std::cerr << "Error creating epoll instance\n";
		throw std::exception();
	}
	// Add the server socket to the epoll instance -----------------------------------------------------------------------
	std::memset(&event, 0, sizeof(event));
	event.events = EPOLLIN;
	event.data.fd = this->getSocketFd();
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->getSocketFd(), &event) == -1) {
		std::cerr << "Error adding socket to epoll\n";
		close(epoll_fd);
		throw std::exception();
	}
}
// Managing new and existing clients -------------------------------------------------------------------------------------
void Server::newClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int client_fd = accept(getSocketFd(), (struct sockaddr *)&client_addr, &client_addr_len);

	if (client_fd == -1)
	{
		std::cerr << "Error accepting client connection\n";
	}
	else
	{
		this->users[client_fd] = User("new", "new", client_fd);
		std::memset(&getEvent(), 0, sizeof(getEvent()));
		getEvent().events = EPOLLIN;
		getEvent().data.fd = client_fd;
		if (epoll_ctl(getEpollFd(), EPOLL_CTL_ADD, client_fd, &getEvent()) == -1)
		{
			std::cerr << "Error adding client to epoll\n";
			close(client_fd);
		}
		else
			processClient(client_fd, getPassword());
	}
}

void Server::existingClient(int fd)
{
	char buffer[1024];
	int client_fd = fd;
	std::memset(buffer, 0, sizeof(buffer));
	int num_bytes = recv(client_fd, buffer, sizeof(buffer), 0);
	if (num_bytes == -1)
	{
		std::cerr << "Error receiving data from client\n";
		close(client_fd);
		epoll_ctl(getEpollFd(), EPOLL_CTL_DEL, client_fd, 0);
		getUsers().erase(client_fd);
	}
	else if (num_bytes == 0 || ((std::string) buffer).find("QUIT :Leaving") != std::string::npos)
	{
        for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
        {
            if (it->getUsers().find(users[client_fd].getNickName()) != it->getUsers().end())
            {
                it->getUsers().erase(users[client_fd].getNickName());
                std::cout << "Removed " << users[client_fd].getNickName() << " from channel " << it->getName() << std::endl;
                for (std::map<std::string, int>::const_iterator p = it->getUsers().begin(); p != it->getUsers().end(); ++p)
                {
                    if (p->second != fd)
                    {
                        std::cout << "Sending parting message to " << users[p->second].getNickName() << "\n";
                        std::string msg = ":server PRIVMSG " + it->getName() + " :" + users[client_fd].getNickName() + " left the channel." + "\r\n";
                        send(p->second, msg.c_str(), msg.length(), 0);
                    }
                }
            }
        }
        std::cout << "Client disconnected.\n";
        close(client_fd);
		epoll_ctl(getEpollFd(), EPOLL_CTL_DEL, client_fd, 0);
		getUsers().erase(client_fd);
	}
	else
	{
		std::cout << "Received " << num_bytes << " bytes from client\n";
		std::cout << buffer << std::endl;

		if (((std::string) buffer).substr(0, 5) == ("JOIN "))
			join(buffer, client_fd);
		else if (((std::string) buffer).substr(0, 8) == ("PRIVMSG "))
			msg((std::string)buffer, client_fd);
		else if (((std::string) buffer).substr(0, 5) == ("PART "))
			part((std::string)buffer, client_fd);
        else if (((std::string) buffer).substr(0, 5) == ("OPER "))
            oper((std::string)buffer, client_fd);
		else if (((std::string) buffer).substr(0, 5) == ("KICK "))
			kick((std::string)buffer, client_fd);
		else if (((std::string) buffer).substr(0, 7) == ("INVITE "))
			invite((std::string)buffer, client_fd);
		else if (((std::string) buffer).substr(0, 6) == ("TOPIC "))
			topic((std::string)buffer, client_fd);
		else if (((std::string) buffer).substr(0, 5) == ("MODE "))
			mode((std::string)buffer, client_fd);
	}
}

int Server::clientFd(std::string nick)
{
	for (std::map<int, User>::const_iterator p = getUsers().begin(); p != getUsers().end(); ++p)
	{
		if (p->second.getNickName() == nick)
			return p->first;
	}
	return -1;
}

// Run server ------------------------------------------------------------------------------------------------------------
void Server::run()
{
	// Listen for incoming connections ------------------------------------------------------------------------------------
	listen(this->getSocketFd(), 5); // Allow up to 5 pending connections in the queue
//	client_fds.push_back(this->getSocketFd());
    users[this->getSocketFd()] = User(".server", ".server", this->getSocketFd());
	// The event loop -----------------------------------------------------------------------------------------------------
	while (true)
	{
		// Wait for events on the epoll file descriptor ---------------------------------------------------------------
		std::vector<epoll_event> events(getUsers().size());
		int num_events = epoll_wait(getEpollFd(), events.data(), (int)events.size(), -1);
		// Check for errors --------------------------------------------------------------------------------------------
		if (num_events == -1)
		{
			std::cerr << "Error in epoll_wait\n";
			close(this->getEpollFd());
			throw std::exception();
		}
		// Handle events for each file descriptor -----------------------------------------------------------------------
		for (int i = 0; i < num_events; i++)
		{
			// Handle new client connections ----------------------------------------------------------------------------
			if (events[i].data.fd == this->getSocketFd())
				newClient();
			// Handle incoming data from clients --------------------------------------------------------------------------------
			else
				existingClient(events[i].data.fd);
		}
	}
}
