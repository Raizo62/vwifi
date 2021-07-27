#include <iostream> // cout
#include <cstdio> //perror

#include <sys/ioctl.h> // ioctl
#include <fcntl.h> // open

#include <arpa/inet.h> // INADDR_ANY
#include <unistd.h> // close
#include <assert.h> // assert

#include "csocketclient.h"
#include "tools.h"

using namespace std;

// ----------------- CSocketClient

CSocketClient::~CSocketClient()
{
	CSocket::Close();
}

CSocketClient::CSocketClient() : CSocket()
{
	Init();
}

void CSocketClient::Init()
{
	IsConnected=false;
	StopTheReconnect=false ;
}

bool CSocketClient::ConnectLoop()
{
	while ( ! StopTheReconnect )
	{
		if( Connect() )
			return true;
	}

	// The system asks to stop the reconnect
	return false;
}

bool CSocketClient::Connect(struct sockaddr* server, size_t size_of_server)
{
	if( ! Configure() )
	{
		cerr<<"Error : CSocketClient::Connect : Configure"<<endl;
		return false;
	}

	if( ! connect(Master,server,size_of_server) )
	{
		IsConnected=true;

		return true;
	}

	perror("CSocketClient::Connect : connect");
	Close();
	sleep(2);

	return false;
}

ssize_t CSocketClient::Send(const char* data, ssize_t sizeOfData)
{
	if ( IsConnected ){

		return CSocket::Send(Master, data, sizeOfData);

	}
	return SOCKET_ERROR;
}

ssize_t CSocketClient::SendBigData(const char* data, TMinimalSize sizeOfData)
{
	if ( IsConnected ){
		return CSocket::SendBigData(Master, data, sizeOfData);
	}
	return SOCKET_ERROR;
}

ssize_t CSocketClient::Read(char* data, ssize_t sizeOfData)
{
	if ( IsConnected ){

		return CSocket::Read(Master, data, sizeOfData);

	}
	return SOCKET_ERROR;
}

ssize_t CSocketClient::ReadBigData(CDynBuffer* data)
{
	if ( IsConnected ){
		return CSocket::ReadBigData(Master, data);
	}
	return SOCKET_ERROR;
}
