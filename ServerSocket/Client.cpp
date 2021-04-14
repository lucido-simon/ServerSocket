#include "Client.h"

Client::Client(SOCKET socket, int id, std::string name) : socket(socket), id(id), name(name)
{
	bActive = true;
}

void Client::notActive()
{
	bActive = false;
}

bool Client::isActive()
{
	return bActive;
}