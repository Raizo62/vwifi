#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <unistd.h> // close

#include "cvsocketclient.h"

using namespace std;

void CVSocketClient::Init(int port)
{
	IsConnected=false;
}

bool CVSocketClient::Connect(char* IP, int port)
{
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(port);

	if( connect(Master,(struct sockaddr*) &server,sizeof(server)) != 0 )
	{
		perror("connect");
		return SOCKET_ERROR;
	}

	IsConnected=true;

	return SOCKET_OK;
}
