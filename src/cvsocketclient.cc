#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <unistd.h> // close

#include "cvsocketclient.h"

using namespace std;

CVSocketClient::CVSocketClient()
{
	Init();
}

void CVSocketClient::Init()
{
	IsConnected=false;
}

bool CVSocketClient::Connect(const char* IP, int port)
{
	//create a master socket
	if( ! Configure() )
	{
		perror("socket failed");
		return false;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(port);

	if( connect(Master,(struct sockaddr*) &server,sizeof(server)) != 0 )
	{
		perror("connect");
		return false;
	}

	IsConnected=true;

	return true;
}

ssize_t CVSocketClient::Send(const char* data, ssize_t sizeOfData)
{
	if ( IsConnected )
		return CVSocket::Send(Master, data, sizeOfData);

	return SOCKET_ERROR;
}

ssize_t CVSocketClient::Read(char* data, ssize_t sizeOfData)
{
	if ( IsConnected )
		return CVSocket::Read(Master, data, sizeOfData);

	return SOCKET_ERROR;
}
