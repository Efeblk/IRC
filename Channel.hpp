#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <map>
#include "User.hpp"

class Channel {
public:
    Channel(const std::string &name);
    Channel();
    ~Channel();

    void addUser(User* user);
    void addOperator(User* user);
    void removeUser(User* user);
    bool hasUser(User* user) const;
    void setTopic(const std::string &topic);
    std::string getTopic() const;

    const std::string& getName() const;
    const std::vector<User*>& getUsers() const; // Const reference for read-only access
    std::vector<User*>& getUsers(); // Non-const reference for modification
    int getUserCount() const;

    void messageAllUsers(const std::string& message, User* sender);
    bool isOperator(User* user) const;

private:
    std::string name;
    std::string topic;
    std::vector<User*> users;
    std::map<std::string, std::string> channelOperators;
};

#endif // CHANNEL_HPP
