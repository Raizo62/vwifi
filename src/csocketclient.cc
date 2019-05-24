#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <linux/vm_sockets.h> // struct sockaddr_vm
#include <unistd.h> // close

#include "cvsocketclient.h"

using namespace std;

CVSocketClient::CVSocketClient() : CVSocket()
{
	Init();
}

CVSocketClient::CVSocketClient(TypeSocket type) : CVSocket(type)
{
	Init();
}

void CVSocketClient::Init()
{
	IsConnected=false;
}

bool CVSocketClient::Connect(const char* IP, unsigned int port)
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

bool CVSocketClient::Connect(unsigned int port)
{
	//create a master socket
	if( ! Configure() )
	{
		perror("socket failed");
		return false;
	}

	//type of socket created
	struct sockaddr_vm server {
		.svm_family = AF_VSOCK,
		.svm_reserved1 = 0,
		.svm_port = port,
		.svm_cid = 2
	};

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
