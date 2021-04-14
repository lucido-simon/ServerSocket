#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "Server.h"


#include <conio.h>
#include <Windows.h>




int main(void)
{

	Server s;
	s.run();
	
	return EXIT_SUCCESS;
}
