#include "Server.h"
#include <thread>
#include <string>


Server::Server()
{
	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	result = NULL;

	commandeClient = "Commandes disponibles sur le serveur : \n10\t-ping\n10\t-rename <pseudo>\n10\t-list\n";

	bWaiting = false;
	bNewClient = false;
	clients = std::vector<Client>();

	wsaData;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
}

int Server::init()
{
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
	{
		printf("--WSAStartup failed with error: %d\n", iResult);
		return 1;
	}


	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) 
	{
		printf("--getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	return 0;
}

int Server::start()
{

	int iResult;

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("--socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("--bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) 
	{
		printf("--listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

void Server::treatCode10(std::string input, int id)
{
	input.erase(input.begin(), input.begin() + 2);

	if (input.size() == 0);
	else if (input.find("-rename") == 0)
	{
		rename(id, input);
	}

	else if (input.find("-list") == 0)
	{
		sendListUser(id);
	}

	else if (input.find("-commandeServeur") == 0)
	{
		sendUnique(commandeClient, id, "10", false);
	}

	else
	{
		std::string messageEnvoie(clients[id].name + " : " + input + "\n");
		ssend(messageEnvoie, id);
	}
}

void Server::rename(int id, std::string message)
{
	std::string messageEnvoie = "--Utilisateur " + clients[id].name + " rename en ";
	message.erase(message.begin(), message.begin() + 8);
	clients[id].name = message;
	messageEnvoie += message + "\n";
	ssend(messageEnvoie, id);
}

void Server::acceptClient()
{

	while (true)
	{
		ClientSocket = accept(ListenSocket, NULL, NULL);

		if (ClientSocket == INVALID_SOCKET)
		{
			printf("--accept failed with error: %d\n", WSAGetLastError());
		}

		else
		{
			clients.push_back(Client(SOCKET(ClientSocket), clients.size(), "Anonyme " + std::to_string(clients.size())));
			bNewClient = true;
			newClient();
		}
	}
	
	closesocket(ListenSocket);
}

void Server::run()
{

	int id = 0;

	std::string res;

	std::cout << "--Initialisation.." << std::endl;

	init();
	std::cout << "--Demarrage.." << std::endl;
	start();
	std::cout << "--Attente de connexion.." << std::endl;

	std::thread t1(&Server::acceptClient, this);
	std::thread t2(&Server::waitingClients, this, &id);
	t1.detach();
	t2.detach();

	while (true)
	{
		if (bWaiting)
		{
			treatWaiting(id);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	std::cout << "6" << std::endl;
	shutdownServer();
	std::cout << "7" << std::endl;

}

void Server::newClient()
{
	std::string message("--Nouveau client connecte, ID : " + std::to_string(+clients[clients.size() - 1].id) + "\n");
	ssend(message, clients[clients.size() - 1].id);
	sendUnique(commandeClient, clients.size() - 1, "10", false);
	bNewClient = false;
}

bool Server::waiting(int *id)
{	
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].isActive())
		{
			fd_set rfd;
			FD_ZERO(&rfd);
			FD_SET(clients[i].socket, &rfd);
			timeval t;
			t.tv_usec = 10000;
			int r = select(clients[i].socket + 1, &rfd, NULL, NULL, &t);

			if (r > 0)
			{
				*id = clients[i].id;
				return true;
			}

			else if (r == SOCKET_ERROR)
			{
				closeConnection(i);
			}
		}

	}	

	return false;
}

void Server::waitingClients(int* id)
{
	while (true)
	{
		if (bWaiting == false)
			if (waiting(id))
			{
				bWaiting = true;
			}
		std::this_thread::sleep_for(std::chrono::milliseconds(100 / ( clients.size() + 1)));
	}
}

void Server::treatWaiting(int id)
{

	if (clients[id].isActive())
	{
		std::string message;
		receive(&message, id);

		bWaiting = false;

		if (message.size() < 2)
		{
			std::cout << "--Server::treatWaiting() : TAILLE MESSAGE INFERIEUR A 2, ID :" << id << " MESSAGE : " << message << std::endl;
			closeConnection(id);
			return;
		}
			

		if (id < 0 || id >= clients.size())
		{
			std::cout << "--Server::treatWaiting() : userID NOT IN RANGE, ID :" << id << std::endl;
			return;
		}

		if (message.find("\n") == std::string::npos)
		{
			std::string t;
			t += message[0];
			t += message[1];
			int code = stoi(t);

			treatMessage(message, code, id);
		}

		else
		{
			std::vector<std::pair<std::string, int>> messages;

			while (message.find("\n") != std::string::npos && message.size() > 1)
			{
				std::string codeS;
				std::string subMessage;

				size_t pos = message.find("\n");
				subMessage = message.substr(0, pos + 1);
				message.erase(0, pos + 1);

				codeS += subMessage[0];
				codeS += subMessage[1];

				messages.push_back(std::pair<std::string, int>(subMessage, stoi(codeS)));
			}

			for (size_t i = 0; i < messages.size(); i++)
			{
				treatMessage(messages[i].first, messages[i].second, id);
			}
		}

	}
	

}

void Server::treatMessage(std::string message, int code, int id)
{
	switch (code)
	{
	case 10:
		treatCode10(message, id);
		break;

	case 11:
		rename(id, message);
		break;

	case 12:
		sendUnique(std::string(), id, "12");
		break;

	case 13:
		sendListUser(id);
		break;
	case 14:
		sendUnique(commandeClient, id, "10", false);
		break;

	default:
		std::cout << "--CODE NON GERER : " << message;
		break;
	}
}

void Server::closeConnection(int id)
{
	std::cout << "--FERMETURE CONNECTION " << id << ".." << std::endl;
	SOCKET s = clients[id].socket;
	clients[id].notActive();
	shutdown(s, SD_BOTH);
	closesocket(s);
	std::cout << "--CONNECTION " << id << " FERMEE" << std::endl;
	std::string message("--Utilisateur " + clients[id].name + " s'est deconnecte\n");
	ssend(message, -1);
}

void Server::sendListUser(int id)
{
	sendUnique("--Liste des utilisateurs connectes :\n", id, "10");
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (i != id && clients[i].isActive())
		{
			sendUnique("  " + clients[i].name + "\n", id, "10");
		}
	}
}
	

int Server::receive(std::string* s, int id)
{

	if (clients[id].isActive())
	{
		int iResult;

		char recvbuf[2048];

		iResult = recv(clients[id].socket, recvbuf, 2048, 0);

		if (iResult < 0)
		{
			printf("--recv failed with error: %d\n", WSAGetLastError());
			closeConnection(id);
			return iResult;
		}

		recvbuf[iResult] = NULL;
		*s = recvbuf;

		return iResult;
	}
	return -1;
	
}

int Server::sendUnique(std::string s, int id, std::string code, bool log)
{
	if (log)
		std::cout << s;

	if (clients[id].isActive())
	{
		code += s;
		send(clients[id].socket, code.c_str(), code.size(), 0);
	}
	return 1;
}

int Server::ssend(std::string s, int id, bool log)
{
	return ssend(s, id, "10", log);
}

int Server::ssend(std::string s, int id, std::string code, bool log)
{
	if (log)
		std::cout << s;

	code += s;
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].id != id && clients[i].isActive())
		{
			send(clients[i].socket, code.c_str(), code.size(), 0);
		}

	}

	return 1;
}

int Server::shutdownServer()
{
	for (size_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].isActive())
			closeConnection(i);
	}

	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}
