#include <iostream> // cout
#include <cstdio> //perror

#include <sys/ioctl.h> // ioctl
#include <fcntl.h> // open

#include <arpa/inet.h> // INADDR_ANY
#include <unistd.h> // close
#include <assert.h> // assert

#include "csocketclientinet.h"
#include "tools.h"

using namespace std;

CSocketClientINET::CSocketClientINET() : CSocketClient(AF_INET) {}

void CSocketClientINET::Init(const char* IP, TPort port)
{
	Server.sin_family = AF_INET;
	Server.sin_addr.s_addr = inet_addr(IP);
	Server.sin_port = htons(port);
}

bool CSocketClientINET::Connect()
{
	return ConnectCore((struct sockaddr*) &Server, sizeof(Server));
}

int CSocketClientINET::GetID()
{
	struct sockaddr_in my_addr;

	socklen_t len = sizeof(my_addr);
	getsockname(Master, (struct sockaddr *) &my_addr, &len);

	return hash_ipaddr(&my_addr) ;
}
