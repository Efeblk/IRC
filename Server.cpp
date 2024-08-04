#include "Server.hpp"
#include "User.hpp"

Server::Server(std::string port, std::string password) {
    if (!isPortValid(port)) {
        std::cout << "Port ERROR" << std::endl;
        exit(EXIT_FAILURE);
    }

    // FIXME: Uncomment this line
    // if (!isPasswordValid(password)) {
    //     std::cout << "Invalid password" << std::endl;
    //     exit(EXIT_FAILURE);
    // }
    this->serverName = "irc.githubbers.com";
    this->operName = "admin";
    this->operPassword = "admin";
    this->port = ft_stoi(port);
    this->password = password;
    this->addrlen = sizeof(address);
    initializeServer();
}

Server::~Server() {
    for (std::vector<User*>::iterator it = users.begin(); it != users.end(); ++it) {
        delete *it;
    }
    for (std::vector<User*>::iterator it = pending_users.begin(); it != pending_users.end(); ++it) {
        delete *it;
    }
    close(server_fd);
}

bool Server::isPortValid(std::string port) {
    // Rakam kontrolü
    for (size_t i = 0; i < port.length(); ++i) {
        if (!isdigit(port[i])) {
            return false;
        }
    }
    int port_i = ft_stoi(port);
    return (port_i > 1024 && port_i < 65535);
}

bool Server::isPasswordValid(const std::string& password) {
    bool hasDigit = false;
    bool hasUpper = false;
    bool hasLower = false;

    if (password.length() < 4) {
        return false;
    }

    for (size_t i = 0; i < password.length(); ++i) {
        if (isdigit(password[i])) {
            hasDigit = true;
        } else if (isupper(password[i])) {
            hasUpper = true;
        } else if (islower(password[i])) {
            hasLower = true;
        }
    }

    return (hasDigit && hasUpper && hasLower);
}

void Server::initializeServer() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        printError("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1) {
        printError("fcntl F_SETFL failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        printError("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        printError("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
    max_sd = server_fd;
}

void Server::start() {
    while (true) {
        std::cout << "Loop start" << std::endl;
        memcpy(&working_set, &master_set, sizeof(master_set));

        int activity = select(max_sd + 1, &working_set, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            printError("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &working_set)) {
            handleNewConnection();
        }

        std::cout << "Pending users: " << pending_users.size() << std::endl;
        handlePendingUsers();
        std::cout << "Connected users: " << users.size() << std::endl;
        handleConnectedUsers();
        std::cout << "Loop end" << std::endl;
    }
}

void Server::handleNewConnection() {
    int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            printError("Accept failed");
        }
        return;
    }

    User* new_user = new User(new_socket, address);
    std::cout << "New connection, socket fd is " << new_user->getSocketFD()
              << ", ip is: " << new_user->getIP()
              << ", port is: " << new_user->getPort() << std::endl;

    pending_users.push_back(new_user);
    FD_SET(new_user->getSocketFD(), &master_set);
    if (new_user->getSocketFD() > max_sd) {
        max_sd = new_user->getSocketFD();
    }
}

void Server::handlePendingUsers() {
    for (std::vector<User*>::iterator it = pending_users.begin(); it != pending_users.end();) {
        if (FD_ISSET((*it)->getSocketFD(), &working_set)) {
            if (!(*it)->getConnected()) {
                delete *it;
                it = pending_users.erase(it);
                continue;
            }
            handleExistingConnection(*it);
            if ((*it)->isAuthenticated()) {
                it = pending_users.erase(it);
            } else {
                authenticate(*it);
                ++it;
            }
        } else {
            ++it;
        }
    }
}

void Server::handleConnectedUsers() {
    for (std::vector<User*>::iterator it = users.begin(); it != users.end();) {
        if (!(*it)->getConnected()) {
            std::cout << "Disconnecting user" << std::endl;
            delete *it;
            it = users.erase(it);
        } else {
            if (FD_ISSET((*it)->getSocketFD(), &working_set)) {
                handleExistingConnection(*it);
            }
            ++it;
        }
    }
}

void Server::handleExistingConnection(User* user) {
    char buffer[1024];
    int received = recv(user->getSocketFD(), buffer, sizeof(buffer) - 1, 0);
    if (received < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            printError("Recv failed");
            removeUser(user);
        }
    } else if (received == 0) {
        getpeername(user->getSocketFD(), (struct sockaddr*)&address, (socklen_t*)&addrlen);
        std::cout << "Host disconnected, ip: " << user->getIP()
                  << ", port: " << user->getPort() << std::endl;
        removeUser(user);
    } else {
        buffer[received] = '\0';
        std::cout << "Message from " << user->getNickname() << ": " << buffer << std::endl;
        processMessage(user, buffer);
    }
}

std::string Server::parseText(const std::string& str) {
    std::string result;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        char c = *it;
        if (std::isprint(c) && !std::isspace(c)) {
            result += c;
        }
    }
    return result;
}

void Server::processMessage(User* user, const std::string& message) {
    std::stringstream ss(message);
    std::string cmd;

    while (std::getline(ss, cmd, '\n')) {
        if (cmd.empty()) continue;
        
        // if (cmd.back() == '\r') cmd.pop_back();
        if (!cmd.empty() && cmd[cmd.size() - 1] == '\r')
            cmd.erase(cmd.size() - 1);


        std::vector<std::string> tokens;
        parser(&tokens, cmd);
        executeCommands(user, tokens, cmd);
    }
}

void Server::executeCommands(User* user, std::vector<std::string> tokens, std::string cmd) {
    void (Server::*cmds[])(User*, std::vector<std::string>) = {
        &Server::PASS, &Server::NICK, &Server::USER, &Server::JOIN, &Server::QUIT,
        &Server::PRIVMSG, &Server::PART, &Server::KICK, &Server::TOPIC, &Server::NOTICE,
        &Server::MODE, &Server::WHO, &Server::OPER 
    };
    std::string commands[] = {
        "PASS", "NICK", "USER", "JOIN", "QUIT", "PRIVMSG", "PART", "KICK", "TOPIC", "NOTICE", "MODE", "WHO", "OPER"
    };

    if (tokens.empty()) return;
	int i;
    for (i = 0; i < 13; ++i) {
        if (tokens[0] == commands[i]) {
            if (!user->isAuthenticated() && (i != 0 && i != 1 && i != 2)) {
                send(user->getSocketFD(), "ERROR :You need to authenticate first\r\n", 40, 0);
                break;
            }

            (this->*cmds[i])(user, tokens);
            break;
        }
    }
    (void)cmd;
}

bool Server::authenticate(User* user) {
    const char* request_password = "Please enter the password using PASS command: \r\n";
    send(user->getSocketFD(), request_password, strlen(request_password), 0);
    return true;
}

void Server::sendWelcomeMessage(User* user) {
    std::string welcome_msg = ":irc.example.com 001 " + user->getNickname() + " :Welcome to the IRC server\r\n";
    std::string motd_msg = ":irc.example.com 372 " + user->getNickname() + " :- Welcome to this IRC server\r\n";
    std::string end_motd_msg = ":irc.example.com 376 " + user->getNickname() + " :End of /MOTD command\r\n";

    send(user->getSocketFD(), welcome_msg.c_str(), welcome_msg.length(), 0);
    send(user->getSocketFD(), motd_msg.c_str(), motd_msg.length(), 0);
    send(user->getSocketFD(), end_motd_msg.c_str(), end_motd_msg.length(), 0);
}

void Server::removeUser(User* user) {
    close(user->getSocketFD());
    FD_CLR(user->getSocketFD(), &master_set);
    user->closeConnection();
    std::cout << "User removed" << std::endl;
}

void Server::messageChannel(const std::string& channelName, const std::string& message, User* sender) {
    if (channels.find(channelName) != channels.end()) {
		//defaultChannel->messageAllUsers(message, sender);
        channels[channelName].messageAllUsers(message, sender);
    } else {
        std::string error_msg = "ERROR: Channel " + channelName + " does not exist.\r\n";
        send(sender->getSocketFD(), error_msg.c_str(), error_msg.length(), 0);
    }
}

void Server::parser(std::vector<std::string> *tokens, std::string cmd) {
    size_t pos = cmd.find(':');
    std::string left_str = "";
    std::string right_str = "";

    if (pos != std::string::npos) {
        left_str = cmd.substr(0, pos - 1);
        right_str = cmd.substr(pos, cmd.size());

        if (right_str[0] == ':') right_str.erase(0, 1);
        while (!right_str.empty() && right_str[0] == ' ') right_str.erase(0, 1);
    } else {
        left_str = cmd;
    }

    std::stringstream line(left_str);
    std::string str;

    while (std::getline(line, str, ' ')) {
        tokens->push_back(str);
    }

    if (!right_str.empty()) {
        tokens->push_back(right_str);
    }
}
