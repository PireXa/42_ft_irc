//
// Created by rdas-nev on 6/2/23.
//

#include "../inc/ft_irc.hpp"

int turn_off = false;

void	Server::ctrlc(int s)
{
	if (s == 2)
	{
		std::cout << BLUE"\nShutting down server.\n" RESET;
		turn_off = true;
	}
}

void	sigHandler()
{
	signal(SIGINT, Server::ctrlc);
	signal(SIGQUIT, SIG_IGN);
}