#include "Server.hpp"

void signalHandler(int code)
{
	std::stringstream ss;
	ss << "Signal " << code << " caught!";
	std::cout << ss.str() << std::endl;
	exit(0);
}

int main(int gc, char** gv)
{
	signal(SIGINT, signalHandler);
	if (gc != 3)
	{
		std::cout << "Argument ERROR " << "./ircserv <port> <password>" << std::endl;
		exit(EXIT_FAILURE);
	}
	Server server(gv[1], gv[2]); // Sunucu nesnesi oluştur
	server.start(); // Sunucuyu başlat

	return 0;
}
