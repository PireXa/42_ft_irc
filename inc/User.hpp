//
// Created by rdas-nev on 4/20/23.
//

#ifndef INC_42_FT_IRC_USER_HPP
#define INC_42_FT_IRC_USER_HPP

#include "ft_irc.hpp"
#include "Channel.hpp"
#include "Server.hpp"

class User
{
		std::string			user;
		std::string			nick;
		int					ufd;
        std::vector<std::string>	channels;
		std::string 		authenticated; // [0] = PASS, [1] = NICK, [2] = USER

	public:
        User();
		User(std::string username, std::string nickname, int fd);
	// Getters
		const std::string	&getUserName() const { return user;}
		const std::string	&getNickName() const { return nick;}
		const int			&getUserfd() const { return ufd;}
	// Setters
		void setUserName(const std::string &uname) { User::user = uname; }
		void setNickName(const std::string &nname) { User::nick = nname; }
		void setUserfd(int fd) { User::ufd = fd; }
    // Managing channels
        void addChannel(const std::string &nm) {channels.push_back(nm);}
	// Authentication
		void setAuthenticated(char code, int position) {User::authenticated[position] = code;}
		int isAuthenticated() const;
};
#endif //INC_42_FT_IRC_USER_HPP
