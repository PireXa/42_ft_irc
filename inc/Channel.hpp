//
// Created by rdas-nev on 5/18/23.
//

#ifndef INC_42_FT_IRC_CHANNEL_HPP
#define INC_42_FT_IRC_CHANNEL_HPP

#include "ft_irc.hpp"

class Channel
{
	std::string					name;
	std::map<std::string, int>	ch_members;
	std::vector<int>	        ops;
	std::vector<int>			invite_list;
	int 						invite_mode;
	std::string					topic;
	int 						topic_mode;
	std::string					key;
	int							key_mode;
	int 						member_limit;
public:
	Channel(const std::string &nm, const std::string &tp) : name(nm), topic(tp)
	{
		// MODES
		invite_mode	= 0;
		topic_mode 	= 1;
		key_mode	= 0;
		member_limit	= 10;
	}
	// ADD
	void addMember(const std::string &nm, int fd){ch_members.insert(std::pair<std::string, int>(nm, fd));}
	void addOp(int fd){ops.push_back(fd);}
	// REMOVE
	void removeMember(const std::string &nm){ch_members.erase(nm);}
	void removeOp(int fd){ops.erase(std::remove(ops.begin(), ops.end(), fd), ops.end());}
	// GETTERS
	const std::string &getName() const {return name;}
	const std::string &getTopic() const {return topic;}
	int getMaxMembers() const {return member_limit;}
	std::map<std::string, int> &getUsers()  {return ch_members;}
	std::vector<int> getOps() const {return ops;}
	std::vector<int> &getInviteList() {return invite_list;}
	int getInviteOnly() const {return invite_mode;}
	int getTopicMode() const {return topic_mode;}
	int getKeyMode() const {return key_mode;}
	const std::string &getKey() const {return key;}
	int getMemberLimit() const {return member_limit;}
	std::string getModes() const
	{
		std::string modes;
		if (invite_mode)
			modes += "i";
		if (topic_mode)
			modes += "t";
		if (key_mode)
			modes += "k";
		if (member_limit)
			modes += "l";
		return modes;
	}
	// CHANGERS

	void changeInviteMode(int inv){ invite_mode = inv;}
	void changeTopic(const std::string &tp){topic = tp;}
	void changeTopicMode(int tp){ topic_mode = tp;}
	void changeKey(const std::string &k){ key = k;}
	void changeKeyMode(int k){ key_mode = k;}
	void changeMemberLimit(int mx){ member_limit = mx;}
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
