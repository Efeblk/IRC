#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <set>
#include <netinet/in.h>

class User {
public:
    User(int socket_fd, struct sockaddr_in address);
    ~User();
    
    int getSocketFD() const;
    std::string getIP() const;
    int getPort() const;
    std::string getNickname() const;
    void setNickname(const std::string& nickname);
    bool isAuthenticated() const;
    void authenticate();
    void closeConnection();
    bool getConnected() const;

    void setUsername(const std::string& username);
    void setMode(const std::string& modeChanges);
    void setUnused(const std::string& unused);
    void setRealName(const std::string& realName);
    void setOperator(bool isOperator);
    
    std::string getUsername() const;
    std::string getModeString() const;
    std::string getUnused() const;
    std::string getRealName() const;
    std::string getChannels() const;
    void addChannel(const std::string& channelName);
    void removeChannel(const std::string& channelName);

    bool wallops;
    bool invisible;
    bool away;
    bool restricted;
    bool operator_;
    bool local_operator;
    bool receipt_of_server_notices;
    
    class UsernameLong : public std::exception {
    public:
        virtual const char* what() const throw();
    };

private:
    int socket_fd;
    struct sockaddr_in address;
    bool authenticated;
    bool is_connected;
    std::string nickname;

    std::string username;
    std::string unused;
    std::string realName;
    std::set<std::string> channels; // Store the list of channels the user is in
};

#endif // USER_HPP
