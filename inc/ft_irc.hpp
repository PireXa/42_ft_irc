//
// Created by rdas-nev on 4/20/23.
//

#ifndef INC_42_FT_IRC_FT_IRC_HPP
#define INC_42_FT_IRC_FT_IRC_HPP

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

class Server;
class Channel;
//class User;

#endif //INC_42_FT_IRC_FT_IRC_HPP
