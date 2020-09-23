#include <cstdio> //perror

#include <iostream> // cout

#include <sys/socket.h> //socket
#include <arpa/inet.h> // struct sockaddr_in & inet_ntoa & ntohs
#include <linux/vm_sockets.h> // struct sockaddr_vm
#include <unistd.h> // close
#include <fcntl.h> // F_GETFL O_NONBLOCK

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

TDescriptor CSocket::GetDescriptor()
{
	return Master;
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
	ssize_t ret = send(descriptor, data, sizeOfData, 0);
	if ( ret != sizeOfData )
		return SOCKET_ERROR ;

	return ret;
}

ssize_t CSocket::SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData)
{
	ssize_t ret = Send(descriptor, (char*)&sizeOfData, (unsigned)sizeof(sizeOfData));
	if( ret == SOCKET_ERROR )
		return SOCKET_ERROR;

	return Send(descriptor, data, sizeOfData);
}

ssize_t CSocket::Read(TDescriptor descriptor, char* data, ssize_t sizeOfData)
{
	ssize_t ret = recv(descriptor , data, sizeOfData, 0);
	if ( ret <= 0 )
		return SOCKET_ERROR ;

	return ret;
}

ssize_t CSocket::ReadBigData(TDescriptor descriptor, CDynBuffer* data)
{
	TMinimalSize size;

	int ret = Read(descriptor, (char*)&size, (unsigned)sizeof(size));
	if( ret == SOCKET_ERROR )
		return SOCKET_ERROR;

	data->NeededSize(size);

	return Read(descriptor, data->GetBuffer(), size);
}

CSocket::operator int()
{
	return Master;
}

TSocket CSocket::GetType()
{
	return Type;
}

void CSocket::Close()
{
	close(Master);
}
