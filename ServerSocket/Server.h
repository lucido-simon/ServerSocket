#pragma once
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "Client.h"
#include <wingdi.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

class Server
{
private:
	WSADATA wsaData;

	bool bWaiting;
	bool bNewClient;

	std::string commandeClient;

	std::vector<Client> clients;
	SOCKET ListenSocket;
	SOCKET ClientSocket;

	struct addrinfo* result;
	struct addrinfo hints;

public :
	Server();

	int init();
	int receive(std::string* s, int id);
	int sendUnique(std::string s, int id, std::string code, bool log = true);
	int ssend(std::string s, int id, bool log = true);
	int ssend(std::string s, int id, std::string code, bool log = true);
	int start();
	void treatCode10(std::string message, int id);
	void rename(int id, std::string message);
	void acceptClient();
	void run();
	void newClient();
	bool waiting(int *id);
	void waitingClients(int* id);
	void treatWaiting(int id);
	void treatMessage(std::string message, int code, int i);
	void closeConnection(int id);
	void sendListUser(int id);
	int shutdownServer();
};

