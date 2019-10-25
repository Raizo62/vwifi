#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
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

void CSocketClient::Init(const char* IP, TPort port)
{
	UseSocketVHOST=false;

	Server.inet.sin_family = AF_INET;
	Server.inet.sin_addr.s_addr = inet_addr(IP);
	Server.inet.sin_port = htons(port);
}

void CSocketClient::Init(TPort port)
{
	UseSocketVHOST=true;

	Server.vhost.svm_family = AF_VSOCK;
	Server.vhost.svm_reserved1 = 0;
	Server.vhost.svm_port = port;
	Server.vhost.svm_cid = 2;
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

bool CSocketClient::Connect()
{
	if( UseSocketVHOST )
		return ConnectLoop((struct sockaddr*) &(Server.vhost), sizeof(Server.vhost));
	else return ConnectLoop((struct sockaddr*) &(Server.inet), sizeof(Server.inet));
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

