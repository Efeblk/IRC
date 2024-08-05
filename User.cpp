#include "User.hpp"
#include <arpa/inet.h>
#include <stdexcept>
#include <sstream>

User::User(int socket_fd, struct sockaddr_in address)
    : wallops(false), invisible(false), away(false), restricted(false),
      operator_(false), local_operator(false), receipt_of_server_notices(false), 
      socket_fd(socket_fd), address(address), authenticated(false), is_connected(true),
      nickname("default"), username("default"), unused("*"), realName("default")
       {}

// Destructor for User class
User::~User() {
    // Resource cleanup if necessary
}

// Get the socket file descriptor
int User::getSocketFD() const {
    return socket_fd;
}

// Get the IP address as a string
std::string User::getIP() const {
    return inet_ntoa(address.sin_addr);
}

// Get the port number
int User::getPort() const {
    return ntohs(address.sin_port);
}

// Get the user's nickname
std::string User::getNickname() const {
    return nickname;
}

// Set the user's nickname
void User::setNickname(const std::string& nickname) {
    this->nickname = nickname;
}

// Check if the user is authenticated
bool User::isAuthenticated() const {
    return authenticated;
}

// Authenticate the user
void User::authenticate() {
    authenticated = true;
}

// Check if the user is connected
bool User::getConnected() const {
    return is_connected;
}

// Close the user's connection
void User::closeConnection() {
    is_connected = false;
}

// Set the user's username
void User::setUsername(const std::string& username) {
    if (username.length() > 9) {
        throw UsernameLong();
    }
    this->username = username;
}

// Set the user's mode using a string-based approach
void User::setMode(const std::string& modeChanges) {
    bool adding = true;
    for (std::string::const_iterator it = modeChanges.begin(); it != modeChanges.end(); ++it) {
        char modeChar = *it;
        switch (modeChar) {
            case '+':
                adding = true;
                break;
            case '-':
                adding = false;
                break;
            case 'i':
                invisible = adding;
                break;
            case 'w':
                wallops = adding;
                break;
            case 'r':
                if (adding) {
                    restricted = true;
                }
                break;
            case 'o':
                if (!adding) {
                    operator_ = false;
                }
                break;
            case 'O':
                if (!adding) {
                    local_operator = false;
                }
                break;
            case 's':
                receipt_of_server_notices = adding;
                break;
            default:
                // Handle unknown mode
                break;
        }
    }
}

// Get the current modes as a string
std::string User::getModeString() const {
    std::string modeStr = "+";
    if (invisible) modeStr += "i";
    if (wallops) modeStr += "w";
    if (restricted) modeStr += "r";
    if (operator_) modeStr += "o";
    if (local_operator) modeStr += "O";
    if (receipt_of_server_notices) modeStr += "s";
    return modeStr;
}

// Set unused field (placeholder functionality)
void User::setUnused(const std::string& unused) {
    this->unused = unused;
}

// Set the user's real name
void User::setRealName(const std::string& realName) {
    this->realName = realName;
}

// Get the user's username
std::string User::getUsername() const {
    return username;
}

// Get the unused field value
std::string User::getUnused() const {
    return unused;
}

// Get the user's real name
std::string User::getRealName() const {
    return realName;
}

// Get the list of channels the user is in as a comma-separated string
std::string User::getChannels() const {
    std::stringstream ss;
    for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it != channels.begin()) {
            ss << ",";
        }
        ss << *it;
    }
    return ss.str();
}

// Add a channel to the user's list
void User::addChannel(const std::string& channelName) {
    channels.insert(channelName);
}

// Remove a channel from the user's list
void User::removeChannel(const std::string& channelName) {
    channels.erase(channelName);
}

// Exception for when the username is too long
const char* User::UsernameLong::what() const throw() {
    return "Username is too long";
}

void User::setOperator(bool isOperator) {
    operator_ = isOperator;
}