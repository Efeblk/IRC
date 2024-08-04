#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <sstream>
#include "numeric.hpp"
#include "User.hpp"
#include <map>
#include <csignal>
#include "Channel.hpp"
#include "Utils.hpp"

class Server
{
	public:
		Server(std::string port, std::string password); // Sunucu oluşturucu fonksiyonu
		~Server(); // Sunucu yıkıcı fonksiyonu
		void start(); // Sunucuyu başlatan fonksiyon

	private:
		std::string serverName;
		int port; // Port numarası
		int server_fd; // Sunucu soket dosya tanımlayıcısı
		struct sockaddr_in address; // Sunucu adres bilgisi
		int addrlen; // Adres uzunluğu
		fd_set master_set, working_set; // Select fonksiyonu için dosya tanımlayıcı setleri
		int max_sd; // Maksimum dosya tanımlayıcısı
		std::string password; // Şifre
		std::vector<User*> users; // Bağlı ve doğrulanmış kullanıcılar
		std::vector<User*> pending_users; // Bağlanmaya çalışan kullanıcılar
		std::string operName;
		std::string operPassword;
		

		bool isPortValid(std::string port); // Port numarasının geçerliliğini kontrol eden fonksiyon
		bool isPasswordValid(const std::string& password);
		void parser(std::vector<std::string> *tokens, std::string cmd);
		void executeCommands(User* user, std::vector<std::string> tokens, std::string cmd);
		void initializeServer(); // Sunucu başlangıç ayarları
		void handleNewConnection(); // Yeni bağlantıları işleme
		void handleExistingConnection(User* user); // Mevcut bağlantıları işleme
		void processMessage(User* user, const std::string& message); // Gelen mesajları işleme
		void sendWelcomeMessage(User* user); // Hoşgeldin mesajı gönderme
		bool authenticate(User* user); // İstemciyi doğrulama
		void removeUser(User* user); // Kullanıcıyı kaldırma
		void handlePendingUsers(); //
		void handleConnectedUsers(); //
		void messageChannel(const std::string& channelName, const std::string& message, User* sender);
		std::string parseText(const std::string& str); // Mesajdan metni ayrıştırma
		User* findUserByNickname(const std::string& nickname);
		std::string formatWhoReply(User* user, const std::string& channelName);

		bool isValidNickname(const std::string& nickname);
		bool isNicknameInUse(const std::string& nickname);
		void broadcastToAllUsers(const std::string& message);

		bool validateOperCredentials(const std::string& operName, const std::string& operPassword);
		void PASS(User* user, std::vector<std::string> tokens);
		void NICK(User* user, std::vector<std::string> tokens);
		void USER(User* user, std::vector<std::string> tokens);
		void JOIN(User* user, std::vector<std::string> tokens);
		void QUIT(User* user, std::vector<std::string> tokens);
		void PRIVMSG(User* user, std::vector<std::string> tokens);
		void PART(User* user, std::vector<std::string> tokens);
		void KICK(User* user, std::vector<std::string> tokens);
		void TOPIC(User* user, std::vector<std::string> tokens);
		void NOTICE(User* user, std::vector<std::string> tokens);
		void MODE(User* user, std::vector<std::string> tokens);
		void LIST(User* user, std::vector<std::string> tokens);
		void NAMES(User* user, std::vector<std::string> tokens);
		void WHO(User* user, std::vector<std::string> tokens);
		void OPER(User* user, std::vector<std::string> tokens);
		std::map<std::string, Channel> channels;
};

#endif // SERVER_HPP
