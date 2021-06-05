#include <iostream> // cout
#include <cstdio> //perror

#include <sys/ioctl.h> // ioctl
#include <fcntl.h> // open

#include <arpa/inet.h> // INADDR_ANY
#include <unistd.h> // close
#include <assert.h> // assert

#include "csocketclientitcp.h"
#include "tools.h"

using namespace std;

CSocketClientITCP::CSocketClientITCP() : CSocketClient(AF_INET) {}

void CSocketClientITCP::Init(const char* IP, TPort port)
{
	Server.sin_family = AF_INET;
	Server.sin_addr.s_addr = inet_addr(IP);
	Server.sin_port = htons(port);
}

bool CSocketClientITCP::Connect()
{
	return ConnectCore((struct sockaddr*) &Server, sizeof(Server));
}

int CSocketClientITCP::GetID()
{
	struct sockaddr_in my_addr;

	socklen_t len = sizeof(my_addr);
	getsockname(Master, (struct sockaddr *) &my_addr, &len);

	return hash_ipaddr(&my_addr) ;
}
