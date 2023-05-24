//
// Created by rdas-nev on 5/18/23.
//

#ifndef INC_42_FT_IRC_CHANNEL_HPP
#define INC_42_FT_IRC_CHANNEL_HPP

#include "ft_irc.hpp"

class Channel
{
	std::string					name;
	std::string					topic;
	int 						max_members;
	std::map<std::string, int>	ch_members;
	std::vector<int>	        ops;
	int 						invite_only;
	std::vector<int>			invite_list;
public:
	Channel(const std::string &nm, const std::string &tp, int mx, int invitemode) : name(nm), topic(tp), max_members(mx), invite_only(invitemode) {}
	// ADD
	void addMember(const std::string &nm, int fd){ch_members.insert(std::pair<std::string, int>(nm, fd));}
	void addOp(int fd){ops.push_back(fd);}
	// REMOVE
	void removeMember(const std::string &nm){ch_members.erase(nm);}
	void removeOp(int fd){ops.erase(std::remove(ops.begin(), ops.end(), fd), ops.end());}
	// GETTERS
	const std::string &getName() const {return name;}
	const std::string &getTopic() const {return topic;}
	int getMaxMembers() const {return max_members;}
	std::map<std::string, int> &getUsers()  {return ch_members;}
	std::vector<int> getOps() const {return ops;}
	std::vector<int> &getInviteList() {return invite_list;}
	int getInviteOnly() const {return invite_only;}
	// CHANGERS
	void changeTopic(const std::string &tp){topic = tp;}
	void changeMaxMembers(int mx){ max_members = mx;}
	// SEARCH
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
	// PRINTER
	void printMembers() const
	{
		std::cout << "Members in channel: " << std::endl;
		for (std::map<std::string, int>::const_iterator it = ch_members.begin(); it != ch_members.end(); ++it) {
			std::cout << "name: " << it->first << ", fd: " << it->second << std::endl;
		}
		std::cout << std::endl;
	}
	void printVectorInt(std::vector<int> vec)
	{
		std::cout << "Vector of ints: " << std::endl;
		for (std::vector<int>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
			std::cout << *it << std::endl;
		}
		std::cout << std::endl;
	}
};

#endif //INC_42_FT_IRC_CHANNEL_HPP
