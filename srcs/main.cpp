//
// Created by rdas-nev on 4/19/23.
//

#include "../inc/ft_irc.hpp"

int main(int ac, char **av)
{
	if (ac == 3)
	{
		try
		{
		// Validate input-----------------------------------------------------------------------
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
		std::cerr << RED"Usage: ./ft_irc <port> <password>\n" RESET;
		return EXIT_FAILURE;
	}
}
