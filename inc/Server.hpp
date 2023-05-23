//
// Created by rdas-nev on 4/19/23.
//

#ifndef INC_42_FT_IRC_SERVER_H
#define INC_42_FT_IRC_SERVER_H

#include "ft_irc.hpp"
#include "User.hpp"

class User;
class Channel;

class Server
{
		const std::string 	password;
        const std::string   oper_auth;
		int 				socket_fd;
		struct sockaddr_in	server_addr;
		int					epoll_fd;
        struct epoll_event 	event;
        std::vector<Channel> channels;
        std::map<int, User> users;
        std::vector<int> server_ops;

public:
	// Constructor
		Server(int prt, char *pd);
	// Getters
		const std::string	&getPassword() const { return password;}
		const int 			&getSocketFd() const {return socket_fd;}
		const int 			&getEpollFd() const {return epoll_fd;}
		struct epoll_event	&getEvent() {return event;}
		std::vector<Channel> &getChannels() {return channels;}
		std::map<int, User> &getUsers() {return users;}
	// Managing new and existing clients
		void processClient(int client_fd, const std::string& password);
		void setNICK(int fd);
        void setUSER(int fd);
		void newClient();
		void existingClient(int fd);
	// Managing channels
		void addChannel(const std::string &nm, const std::string &tp, int mx);
	// Commands
		void join(char *buf, int fd);
		void msg(std::string buf, int fd);
		void part(std::string buf, int fd);
        void oper(std::string buf, int fd);
        void kick(std::string buf, int fd);
        void invite(std::string buf, int fd);
        void topic(std::string buf, int fd);
	// Run server
		void run();
};

#endif //INC_42_FT_IRC_SERVER_H
