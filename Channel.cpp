#include "Channel.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel(const std::string &name) : name(name), topic("") {}

Channel::Channel() : name(""), topic("") {}

Channel::~Channel() {}

void Channel::addUser(User* user) {
    if(hasUser(user)) {
        return;
    }
    users.push_back(user);
}

void Channel::addOperator(User* user) {
    channelOperators[user->getNickname()] = user->getNickname();
}

void Channel::removeUser(User* user) {
    users.erase(std::remove(users.begin(), users.end(), user), users.end());
    channelOperators.erase(user->getNickname());
}

bool Channel::hasUser(User* user) const {
    return std::find(users.begin(), users.end(), user) != users.end();
}

void Channel::setTopic(const std::string &topic) {
    this->topic = topic;
}

std::string Channel::getTopic() const {
    return topic;
}

const std::string& Channel::getName() const {
    return name;
}

const std::vector<User*>& Channel::getUsers() const {
    return users;
}

std::vector<User*>& Channel::getUsers() {
    return users;
}

// void Channel::messageAllUsers(const std::string& message, User* sender) {
//     std::string nick;
//     if (sender == nullptr)
//         nick = "Server";
//     else
//         nick = sender->getNickname();
//     std::string fullMessage = ":" + nick + " PRIVMSG " + this->getName() + " :" + message + "\r\n";
//     for (std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it) {
//         if (*it != sender && *it != nullptr) {
//             send((*it)->getSocketFD(), fullMessage.c_str(), fullMessage.length(), 0);
//         }
//     }
// }

void Channel::messageAllUsers(const std::string& message, User* sender) {
    std::string nick;
    if (sender == NULL)
        nick = "Server";
    else
        nick = sender->getNickname();
    std::string fullMessage = ":" + nick + " PRIVMSG " + this->getName() + " :" + message + "\r\n";
    for (std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it) {
        if (*it != sender && *it != NULL) {
            send((*it)->getSocketFD(), fullMessage.c_str(), fullMessage.length(), 0);
        }
    }
}


int Channel::getUserCount() const {
    return users.size();
}

bool Channel::isOperator(User* user) const {
    return channelOperators.find(user->getNickname()) != channelOperators.end();
}
