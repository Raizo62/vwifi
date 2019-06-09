#include <cstdio> //perror

#include <iostream> // cout

#include <sys/socket.h> //socket
#include <arpa/inet.h> // struct sockaddr_in & inet_ntoa & ntohs
#include <linux/vm_sockets.h> // struct sockaddr_vm
#include <unistd.h> // close

#include "csocket.h"

using namespace std;

CSocket::CSocket()
{
	Master=0;
#ifdef _USE_VSOCK_BY_DEFAULT_
	cout<<"Type : AF_VSOCK"<<endl;
	Type=AF_VSOCK;
#else
	cout<<"Type : AF_INET"<<endl;
	Type=AF_INET;
#endif
}

CSocket::CSocket(TSocket type)
{
	Master=0;
	Type=type;
}

CSocket::~CSocket()
{
	Close();
}

bool CSocket::Configure()
{
	//create a master socket

	Master = socket(Type , SOCK_STREAM , 0);

	if( Master == SOCKET_ERROR )
	{
		perror("CSocket::Configure : socket");
		return false;
	}

	return true;
}

ssize_t CSocket::Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData)
{
	return send(descriptor, data, sizeOfData, 0);
}

ssize_t CSocket::Read(TDescriptor descriptor, char* data, ssize_t sizeOfData)
{
	return recv(descriptor , data, sizeOfData, 0);
}

CSocket::operator int()
{
	return Master;
}

void CSocket::Close()
{
	close(Master);
}


