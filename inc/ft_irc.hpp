//
// Created by rdas-nev on 4/20/23.
//

#ifndef FT_IRC_HPP
#define FT_IRC_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <ctime>

#include "Server.hpp"
#include "User.hpp"
#include "Channel.hpp"

#define GREEN "\033[32m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

class Server;
class Channel;
//class User;

bool isInVector(const std::vector<int>& vec, int target);
bool validate_input(char *port, char *password);

#endif //FT_IRC_HPP
