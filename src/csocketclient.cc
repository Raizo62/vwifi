#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <linux/vm_sockets.h> // struct sockaddr_vm
#include <unistd.h> // close

#include "csocketclient.h"

using namespace std;

CSocketClient::~CSocketClient() 
{
	CSocket::Close();
}

CSocketClient::CSocketClient() : CSocket()
{
	Init();
}

CSocketClient::CSocketClient(TSocket type) : CSocket(type)
{
	Init();
}

void CSocketClient::Init()
{
	IsConnected=false;
}

bool CSocketClient::Connect(const char* IP, TPort port)
{
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(port);

	while ( 1 )
	{
		//create a master socket
		if( ! Configure() )
		{
			cerr<<"Error : CSocketClient::Connect : Configure"<<endl;
			return false;
		}

		if( ! connect(Master,(struct sockaddr*) &server,sizeof(server)) )
		{
			IsConnected=true;

			return true;
		}

		perror("CSocketClient::Connect : connect");
		Close();
		sleep(2);
	}

	// impossible to be here
	return false;
}

bool CSocketClient::Connect(TPort port)
{
	//type of socket created
	struct sockaddr_vm server {
		.svm_family = AF_VSOCK,
		.svm_reserved1 = 0,
		.svm_port = port,
		.svm_cid = 2
	};


	while ( 1 )
	{
		//create a master socket
		if( ! Configure() )
		{
			cerr<<"Error : CSocketClient::Connect : Configure"<<endl;
			return false;
		}

		if( ! connect(Master,(struct sockaddr*) &server,sizeof(server)) )
		{
			IsConnected=true;

			return true;
		}

		perror("CSocketClient::Connect : connect");
		Close();
		sleep(2);
	}

	// impossible to be here
	return false;
}

ssize_t CSocketClient::Send(const char* data, ssize_t sizeOfData)
{
	if ( IsConnected )
		return CSocket::Send(Master, data, sizeOfData);

	return SOCKET_ERROR;
}

ssize_t CSocketClient::Read(char* data, ssize_t sizeOfData)
{
	if ( IsConnected )
		return CSocket::Read(Master, data, sizeOfData);

	return SOCKET_ERROR;
}

bool CSocketClient::SetBlocking(bool blocking)
{
	return CSocket::SetBlocking(Master,blocking);
}
