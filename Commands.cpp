#include "Server.hpp"

// Handles the PASS command, authenticating the user
void Server::PASS(User* user, std::vector<std::string> tokens)
{
    if (!user->isAuthenticated()) {
        if (tokens.size() < 2) {
            send(user->getSocketFD(), "ERROR :Password required\r\n", 27, 0);
            return;
        }

        std::string receivedPassword = parseText(tokens[1]);
        if (receivedPassword == password) {
            user->authenticate();
            sendWelcomeMessage(user);
            send(user->getSocketFD(), "OK :Password accepted\r\n", 24, 0);
            std::cout << "User authenticated" << std::endl;
            users.push_back(user);
        } else {
            send(user->getSocketFD(), "ERROR :Password incorrect\r\n", 28, 0);
            std::cout << "Incorrect password" << std::endl;
        }
    } else {
        send(user->getSocketFD(), "ERROR :Already authenticated\r\n", 30, 0);
    }
}

void Server::NICK(User* user, std::vector<std::string> tokens)
{
    if (tokens.size() < 2) {
        send(user->getSocketFD(), "431 :No nickname given\r\n", strlen("431 :No nickname given\r\n"), 0); // ERR_NONICKNAMEGIVEN
        return;
    }

    std::string newNickname = parseText(tokens[1]);

    // Validate the new nickname
    if (!isValidNickname(newNickname)) {
        std::string errorMsg = "432 " + newNickname + " :Erroneous nickname\r\n";
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0); // ERR_ERRONEUSNICKNAME
        return;
    }

    // Check if the nickname is already in use
    if (isNicknameInUse(newNickname)) {
        std::string errorMsg = "433 " + newNickname + " :Nickname is already in use\r\n";
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0); // ERR_NICKNAMEINUSE
        return;
    }

    // Notify other users of the nickname change
    std::string oldNickname = user->getNickname();
    user->setNickname(newNickname);

    std::string notification = ":" + oldNickname + " NICK " + newNickname + "\r\n";
    broadcastToAllUsers(notification);
}


// Handles the USER command, setting user details
void Server::USER(User* user, std::vector<std::string> tokens)
{
    if (tokens.size() < 5) {
        send(user->getSocketFD(), "ERROR :Not enough parameters\r\n", 30, 0);
        return;
    }

    std::string tempUsername = user->getUsername();
    std::string tempRealName = user->getRealName();    
    std::string tempUnused = user->getUnused();

    try {
        user->setUsername(tokens[1]);
        user->setUnused(tokens[3]);
        user->setRealName(tokens[4]);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        user->setUsername(tempUsername);
        user->setRealName(tempRealName);
        user->setUnused(tempUnused);
        send(user->getSocketFD(), "ERROR :Invalid input\r\n", 21, 0);
    }
}

// Placeholder function for the JOIN command
void Server::JOIN(User* user, std::vector<std::string> tokens)
{
    if (tokens.size() < 2) {
        send(user->getSocketFD(), "ERROR :No channel specified\r\n", 29, 0);
        return;
    }

    std::string channelName = parseText(tokens[1]);

    if (channelName == "0")
    {
        PART(user, tokens);
        return;
    }

    // Ensure channel name starts with a valid prefix
    if (channelName.empty() || (channelName[0] != '#' && channelName[0] != '&')) {
        send(user->getSocketFD(), "ERROR :Invalid channel name\r\n", 29, 0);
        return;
    }

    Channel* channel = NULL;
    if (channels.find(channelName) == channels.end()) {
        channels[channelName] = Channel(channelName);
    }
    channel = &channels[channelName];

    // If the channel was newly created, perform additional setup
    if (channel->getUsers().empty()) {
        channel->addOperator(user);
    }

    channel->addUser(user);
    user->addChannel(channelName);

    //Notify all users in the channel about the new join
    //bu method doğru değil aslında ama tek for döngüsünde çalışmıyor(?), evo için yeterli yapacak bir şey yok.
    std::string joinMessage = ":" + user->getNickname() + " JOIN " + channelName + "\r\n";
    std::vector<User*>::iterator it;
    for (it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
        send((*it)->getSocketFD(), joinMessage.c_str(), joinMessage.length(), 0);
    }
    for (it = channel->getUsers().begin(); it != channel->getUsers().end(); ++it) {
        std::string joinMessage = ":" + (*it)->getNickname() + " JOIN " + channelName + "\r\n";
        send((user->getSocketFD()), joinMessage.c_str(), joinMessage.length(), 0);
    }

    // Send topic or no-topic message
    if (!channel->getTopic().empty()) {
        std::string topicMessage = RPL_TOPIC(user->getNickname(), channelName, channel->getTopic()) + "\r\n";
        send(user->getSocketFD(), topicMessage.c_str(), topicMessage.length(), 0);
    } else {
        std::string noTopicMessage = RPL_NOTOPIC(user->getNickname(), channelName) + "\r\n";
        send(user->getSocketFD(), noTopicMessage.c_str(), noTopicMessage.length(), 0);
    }

    std::string infoMessage = "JOIN, MODE, KICK, PART, QUIT, PRIVMSG/NOTICE. \r\n";
    send(user->getSocketFD(), infoMessage.c_str(), infoMessage.length(), 0);
}

// Handles the QUIT command, removing the user
void Server::QUIT(User* user, std::vector<std::string> tokens)
{
    (void)tokens;
    removeUser(user);
}

// Placeholder function for the PRIVMSG command
void Server::PRIVMSG(User* user, std::vector<std::string> tokens)
{
    if (tokens.size() < 3) {
        send(user->getSocketFD(), "ERROR :No target or message specified\r\n", 40, 0);
        return;
    }

    std::string target = tokens[1];
    std::string message = tokens[2]; // Consider handling additional tokens as part of the message

    // Determine if the target is a channel or a user
    if (target[0] == '#') {
        // Message to a channel
        std::map<std::string, Channel>::iterator it = channels.find(target);
        if (it != channels.end()) {
            Channel& channel = it->second;
            channel.messageAllUsers(message, user);
        } else {
            std::string errorMessage = "ERROR :No such channel\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
        }
    } else {
        // Message to a specific user
        User* targetUser = findUserByNickname(target);
        if (targetUser) {
            std::string fullMessage = ":" + user->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
            send(targetUser->getSocketFD(), fullMessage.c_str(), fullMessage.length(), 0);
        } else {
            std::string errorMessage = "ERROR :No such user\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
        }
    }
}

void Server::PART(User* user, std::vector<std::string> tokens) {
    if (tokens.size() < 2) {
        send(user->getSocketFD(), "ERROR :No channel specified\r\n", 29, 0);
        return;
    }

    std::string partMessage = (tokens.size() > 2) ? tokens[2] : user->getNickname();
    std::string channelsList = (tokens[1] == "0") ? user->getChannels() : tokens[1];
    std::stringstream ss(channelsList);
    std::string channelName;

    while (std::getline(ss, channelName, ',')) {
        std::map<std::string, Channel>::iterator it = channels.find(channelName);
        if (it == channels.end()) {
            std::string errorMessage = "ERROR :No such channel " + channelName + "\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
            std::cout << "No such channel: " << channelName << std::endl;
            continue;
        }

        Channel& channel = it->second;
        if (!channel.hasUser(user)) {
            std::string errorMessage = "ERROR :You're not on that channel " + channelName + "\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
            std::cout << "User not in channel: " << channelName << std::endl;
            continue;
        }

        std::string partMessageFull = ":" + user->getNickname() + " PART " + channelName + " :" + partMessage + "\r\n";
        for (std::vector<User*>::iterator userIt = channel.getUsers().begin(); userIt != channel.getUsers().end(); ++userIt) {
            if (*userIt != user) {
                send((*userIt)->getSocketFD(), partMessageFull.c_str(), partMessageFull.length(), 0);
            }
        }

        channel.removeUser(user);
        user->removeChannel(channelName);
        if (channel.getUsers().empty()) {
            channels.erase(channelName);
            std::cout << "Channel emptied and removed: " << channelName << std::endl;
        }

        // Send PART message to the user who is leaving the channel
        send(user->getSocketFD(), partMessageFull.c_str(), partMessageFull.length(), 0);
        std::cout << "User parted from channel: " << channelName << std::endl;
    }
}

// Placeholder function for the TOPIC command
void Server::TOPIC(User* user, std::vector<std::string> tokens)
{
    (void)user;
    if (tokens.size() < 3) {
        // Hata: Kanal ismi veya konu belirtilmedi
        return;
    }
    std::string channelName = tokens[1];
    std::string newTopic = tokens[2]; // Burada tüm geri kalan kısmı almak gerekebilir
    if (channels.find(channelName) != channels.end()) {
        channels[channelName].setTopic(newTopic);
        // Tüm kullanıcılara yeni konuyu bildir
    }
}

// Placeholder function for the NOTICE command
void Server::NOTICE(User* user, std::vector<std::string> tokens) {
    if (tokens.size() < 3) {
        send(user->getSocketFD(), "ERROR :No target or message specified\r\n", 40, 0);
        return;
    }

    std::string target = tokens[1];
    std::string message;

    // Concatenate the message parts if the message spans multiple tokens
    for (std::vector<std::string>::iterator it = tokens.begin() + 2; it != tokens.end(); ++it) {
        if (it != tokens.begin() + 2) {
            message += " ";
        }
        message += *it;
    }

    // Determine if the target is a channel or a user
    if (target[0] == '#') {
        // Notice to a channel
        std::map<std::string, Channel>::iterator it = channels.find(target);
        if (it != channels.end()) {
            Channel& channel = it->second;
            std::string fullMessage = ":" + user->getNickname() + " NOTICE " + target + " :" + message + "\r\n";

            // Store the reference to the users vector in a local variable
            const std::vector<User*>& channelUsers = channel.getUsers();
            for (std::vector<User*>::const_iterator userIt = channelUsers.begin(); userIt != channelUsers.end(); ++userIt) {
                if ((*userIt)->getNickname() != user->getNickname()) {
                    send((*userIt)->getSocketFD(), fullMessage.c_str(), fullMessage.length(), 0);
                }
            }
        } else {
            std::string errorMessage = "ERROR :No such channel\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
        }
    } else {
        // Notice to a specific user
        User* targetUser = findUserByNickname(target);
        if (targetUser) {
            std::string fullMessage = ":" + user->getNickname() + " NOTICE " + target + " :" + message + "\r\n";
            send(targetUser->getSocketFD(), fullMessage.c_str(), fullMessage.length(), 0);
        } else {
            std::string errorMessage = "ERROR :No such user\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
        }
    }
}

// Implementation of the MODE command
void Server::MODE(User* user, std::vector<std::string> tokens)
{
    if (tokens.size() < 2) {
        send(user->getSocketFD(), "461 MODE :Not enough parameters\r\n", 37, 0); // ERR_NEEDMOREPARAMS
        return;
    }

    std::string target = tokens[1];
    // User can only change their own modes
    if (user->getNickname() != target) {
        send(user->getSocketFD(), "502 MODE :You can only change your own mode\r\n", 45, 0); // ERR_USERSDONTMATCH
        return;
    }
    // If no mode changes are specified, return current mode
    if (tokens.size() == 2) {
        std::string modeReply = ":" + serverName + " 221 " + user->getNickname() + " " + user->getModeString() + "\r\n"; // RPL_UMODEIS
        send(user->getSocketFD(), modeReply.c_str(), modeReply.length(), 0);
        return;
    }
    // Parse mode changes
    std::string modes = tokens[2];
    user->setMode(modes);

    // Send acknowledgment
    std::string modeAck = ":" + serverName + " MODE " + user->getNickname() + " " + modes + "\r\n";
    send(user->getSocketFD(), modeAck.c_str(), modeAck.length(), 0);
}


// Placeholder function for the LIST command
void Server::LIST(User* user, std::vector<std::string> tokens)
{
    (void)user;
    (void)tokens;
}

// Placeholder function for the NAMES command
void Server::NAMES(User* user, std::vector<std::string> tokens)
{
    (void)user;
    (void)tokens;
}

// Placeholder function for the WHO command
void Server::WHO(User* user, std::vector<std::string> tokens) {
    if (tokens.size() < 2) {
        // Global WHO - not implemented here for simplicity
        return;
    }

    std::string target = tokens[1];
    bool operatorsOnly = (tokens.size() > 2 && tokens[2] == "o");

    if (target[0] == '#') {
        // WHO for a channel
        std::map<std::string, Channel>::iterator it = channels.find(target);
        if (it != channels.end()) {
            Channel& channel = it->second;
            std::vector<User*> channelUsers = channel.getUsers();
            for (std::vector<User*>::iterator userIt = channelUsers.begin(); userIt != channelUsers.end(); ++userIt) {
                User* channelUser = *userIt;
                if (!operatorsOnly || channel.isOperator(channelUser)) {
                    std::string whoReply = formatWhoReply(channelUser, channel.getName());
                    send(user->getSocketFD(), whoReply.c_str(), whoReply.length(), 0);
                }
            }
            std::string endOfWho = RPL_ENDOFWHO(user->getNickname(), target);
            send(user->getSocketFD(), endOfWho.c_str(), endOfWho.length(), 0);
        } else {
            std::string errorMessage = "ERROR :No such channel\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
        }
    } else {
        // WHO for a specific user
        User* targetUser = findUserByNickname(target);
        if (targetUser) {
            if (!operatorsOnly || targetUser->operator_) {
                std::string whoReply = formatWhoReply(targetUser, "*");
                send(user->getSocketFD(), whoReply.c_str(), whoReply.length(), 0);
            } else {
                std::string errorMessage = "ERROR :No such user\r\n";
                send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
            }
        } else {
            std::string errorMessage = "ERROR :No such user\r\n";
            send(user->getSocketFD(), errorMessage.c_str(), errorMessage.length(), 0);
        }
    }
}

void Server::OPER(User* user, std::vector<std::string> tokens) {
    if (tokens.size() < 3) {
        // Not enough parameters, respond with an error
        std::string errorMsg = ":" + serverName + " 461 OPER :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    std::string operUsername = tokens[1];
    std::string operPassword = tokens[2];

    // Validate the credentials (this is a placeholder for actual validation logic)
    if (validateOperCredentials(operUsername, operPassword)) {
        // Grant operator status
        user->setOperator(true);
        std::string successMsg = ":" + serverName + " 381 " + user->getNickname() + " :You are now an IRC operator\r\n"; // RPL_YOUREOPER
        send(user->getSocketFD(), successMsg.c_str(), successMsg.length(), 0);
    } else {
        // Invalid credentials
        std::string errorMsg = ":" + serverName + " 464 " + user->getNickname() + " :Password incorrect\r\n"; // ERR_PASSWDMISMATCH
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0);
    }
}

void Server::KICK(User* user, std::vector<std::string> tokens) {
    if (tokens.size() < 3) {
        std::string errorMsg = ":" + serverName + " 461 KICK :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    std::string channelName = tokens[1];
    std::string targetNickname = tokens[2];
    std::string comment = user->getNickname(); // Default comment is the nickname of the issuer

    if (tokens.size() > 3) {
        comment = tokens[3];
    }

    // Check if the channel exists
    std::map<std::string, Channel>::iterator channelIt = channels.find(channelName);
    if (channelIt == channels.end()) {
        std::string errorMsg = ":" + serverName + " 403 " + channelName + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    Channel& channel = channelIt->second;

    // Check if the issuer is a channel operator
    if (!channel.isOperator(user) && !user->operator_) {
        std::string errorMsg = ":" + serverName + " 482 " + channelName + " :You're not a channel operator\r\n"; // ERR_CHANOPRIVSNEEDED
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Check if the target user is in the channel
    User* targetUser = findUserByNickname(targetNickname);
    if (targetUser == 0 || !channel.hasUser(targetUser)) {
        std::string errorMsg = ":" + serverName + " 441 " + targetNickname + " " + channelName + " :They aren't on that channel\r\n"; // ERR_USERNOTINCHANNEL
        send(user->getSocketFD(), errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Perform the kick
    channel.removeUser(targetUser);
    // Notify the channel and the user being kicked
    std::string kickMsg = ":" + user->getNickname() + "!" + user->getUsername() + "@" + user->getIP() + " KICK " + channelName + " " + targetNickname + " :" + comment + "\r\n";
    std::vector<User*>::iterator userIt;
    for (userIt = channel.getUsers().begin(); userIt != channel.getUsers().end(); ++userIt) {
        send((*userIt)->getSocketFD(), kickMsg.c_str(), kickMsg.length(), 0); // Notify the channel
    }
    send(targetUser->getSocketFD(), kickMsg.c_str(), kickMsg.length(), 0); // Notify the user being kicked
}


//HELPERS
//HELPERS
//HELPERS

std::string Server::formatWhoReply(User* user, const std::string& channelName) {
    std::string userPrefix = user->operator_ ? "@" : "";
    std::string whoReply = ":" + serverName + " 352 " + channelName + " " +
                           user->getNickname() + " " +
                           user->getUsername() + " " +
                           user->getIP() + " " +
                           serverName + " " +
                           userPrefix + user->getNickname() + " H :0 " +
                           user->getRealName() + "\r\n";
    return whoReply;
}

User* Server::findUserByNickname(const std::string& nickname) {
    for (std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getNickname() == nickname) {
            return *it;
        }
    }
    return NULL;
}

bool Server::isValidNickname(const std::string& nickname) {
    if (nickname.length() < 1 || nickname.length() > 9) {
        return false;
    }
    if (!std::isalpha(nickname[0])) {
        return false;
    }
    for (std::string::const_iterator it = nickname.begin(); it != nickname.end(); ++it) {
        char c = *it;
        if (!std::isalnum(c) && c != '-' && c != '_') {
            return false;
        }
    }
    return true;
}

bool Server::isNicknameInUse(const std::string& nickname) {
    for (std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it) {
        if ((*it)->getNickname() == nickname) {
            return true;
        }
    }
    return false;
}

void Server::broadcastToAllUsers(const std::string& message) {
    for (std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it) {
        User* user = *it;
        send(user->getSocketFD(), message.c_str(), message.length(), 0);
    }
}

// Placeholder function for validating oper credentials
bool Server::validateOperCredentials(const std::string& username, const std::string& password) {
    return (username == operName && password == operPassword);
}
