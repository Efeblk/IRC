#ifndef NUMERIC_HPP
#define NUMERIC_HPP

#define RPL_TOPIC(nickname, channel, topic) "332 " + nickname + " " + channel + " " + topic
#define RPL_NOTOPIC(nickname, channel) "331 " + nickname + " " + channel + " :No topic is set"
#define RPL_WELCOME(nick, user, host) std::string("001 ") + nick + " :Welcome to the Internet Relay Network " + "!" + user + "@" + host
#define RPL_CREATED(nick, date) (std::string("003 ") + ":This server was created " + date)
#define RPL_NAMREPLY(nickname, channel, nicknames) "353 " + nickname + " = " + channel + " :" + nicknames
#define RPL_ENDOFNAMES(nickname, channel) "366 " + nickname + " " + channel + " :End of /NAMES list"
#define RPL_WHOREPLY(nickname, channel, user, host, server, nick, status, hopcount, realname) "352 " + nickname + " " + channel + " " + user + " " + host + " " + server + " " + nick + " " + status + " " + hopcount + " :" + realname
#define RPL_ENDOFWHO(nickname, channel) "315 " + nickname + " " + channel + " :End of /WHO list"
#define RPL_CHANNELMODEIS(channel, mode) "324 " + channel + " " + mode
#define ERR_NEEDMOREPARAMS(cmd) (std::string("461 ") + cmd + " :Not enough parameter")
#define ERR_PASSWDMISMATCH() ("464 :Password incorrect")
#define ERR_NONICKNAMEGIVEN() ("431 :No nickname given")
#define ERR_UNKNOWNCOMMAND(cmd) ("421 ERR_UNKNOWNCOMMAND " + cmd + " :Unknown command")
#define ERR_ALREADYREGISTRED() ("462 :You may not reregister")
#define ERR_NICKNAMEINUSE(nick) ("433 * " + nick + " :Nickname is already in use")
#define ERR_NOSUCHNICK(nickname) "401 " + nickname + " :No such nick"
#define ERR_NOSUCHCHANNEL(channel) "403 " + channel + " :No such channel"
#define ERR_NOTONCHANNEL(channel) "442 " + channel + " :You're not on that channel"
#define ERR_CHANOPRIVSNEEDED(channel) "482 " + channel + " :You're not channel operator"
#define ERR_USERONCHANNEL(nick, channel) "443 " + nick + " " + channel + " :is already on channel"
#define ERR_CANNOTSENDTOCHAN(channel) "404 " + channel + " :Cannot send to channel"

#endif
