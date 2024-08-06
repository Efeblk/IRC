#include "Server.hpp"

int main(int gc, char** gv)
{
	if (gc != 3)
	{
		std::cout << "Argument ERROR " << "./ircserv <port> <password>" << std::endl;
		exit(EXIT_FAILURE);
	}
	Server server(gv[1], gv[2]); // Sunucu nesnesi oluştur
	server.start(); // Sunucuyu başlat

	return 0;
}
