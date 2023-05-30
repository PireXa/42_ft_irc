//
// Created by rdas-nev on 4/20/23.
//

#include "../inc/User.hpp"

User::User()
{
    user	= "";
    nick	= "";
    ufd		= -1;
}

User::User(std::string username, std::string nickname, int fd)
{
	user	= username;
	nick	= nickname;
	ufd		= fd;
	authenticated = "000";
}

int User::isAuthenticated() const
{
	if (authenticated == "111")
		return 1;
	return 0;
}
