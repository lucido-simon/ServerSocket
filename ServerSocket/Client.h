#pragma once

#pragma once
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "Client.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

class Client 
{
public : 
	SOCKET socket;
	int id;
	std::string name;
	bool bActive;

	Client(SOCKET s, int id, std::string name);

	void notActive();
	bool isActive();
};

