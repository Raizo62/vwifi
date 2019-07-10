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
	StopTheReconnect=false ;
}

bool CSocketClient::ConnectLoop(struct sockaddr* server, size_t size_of_server)
{
	while ( ! StopTheReconnect )
	{
		//create a master socket
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
	}

	// impossible to be here
	return false;
}

bool CSocketClient::Connect(const char* IP, TPort port)
{
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(port);

	return ConnectLoop((struct sockaddr*) &server, sizeof(server));
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

	return ConnectLoop((struct sockaddr*) &server, sizeof(server));
}

ssize_t CSocketClient::Send(const char* data, ssize_t sizeOfData)
{
	if ( IsConnected ){

		int ret = CSocket::Send(Master, data, sizeOfData);

		if( ret > 0 )
			return ret ;
		if(ret < 0 && errno == EWOULDBLOCK )
			return SOCKET_ERROR ;

		// if recv returns<0 and errno!=EWOULDBLOCK -->  then it is something that can be considered as connection-losing
		return SOCKET_DISCONNECT ;

	}
	return SOCKET_ERROR;
}

ssize_t CSocketClient::Read(char* data, ssize_t sizeOfData)
{
	if ( IsConnected ){
		int ret = CSocket::Read(Master, data, sizeOfData);
		if( ret > 0 )
			return ret ;
		if(ret == 0 )
			return SOCKET_DISCONNECT ;

		if ( ret < 0 && errno == EWOULDBLOCK )
			return SOCKET_ERROR ;

		// if recv returns<0 and errno!=EWOULDBLOCK -->  then it is something that can be considered as EOF
		return SOCKET_DISCONNECT ;
	}
	return SOCKET_ERROR;
}

bool CSocketClient::SetBlocking(bool blocking)
{
	return CSocket::SetBlocking(Master,blocking);
}

void CSocketClient::StopReconnect(bool status){

	StopTheReconnect = true ;
}

