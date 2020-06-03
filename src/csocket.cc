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

/**
 * \fn int SetBlocking(int blocking)
 * \biref Set a file descriptor to blocking or non-blocking mode.
 *
 * \param blocking false:non-blocking mode, true:blocking mode
 *
 * \return true:success, false:failure.
 **/
bool CSocket::SetBlocking(TDescriptor descriptor, bool blocking)
{
	/* Save the current flags */
	int flags = fcntl(descriptor, F_GETFL, 0);
	if (flags == -1)
		return 0;

	if( blocking )
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	return fcntl(descriptor, F_SETFL, flags) != -1;
}

ssize_t CSocket::Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData)
{
	return send(descriptor, data, sizeOfData, 0);
}

ssize_t CSocket::SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData)
{
	ssize_t ret = Send(descriptor, (char*)&sizeOfData, (unsigned)sizeof(sizeOfData));
	if( ret == 0 )
		return SOCKET_DISCONNECT ;
	if ( ret < 0 )
	{
		if ( errno == EWOULDBLOCK )
			return SOCKET_ERROR ;
		// if recv returns<0 and errno!=EWOULDBLOCK -->  then it is something that can be considered as EOF
		return SOCKET_DISCONNECT ;
	}

	ret = Send(descriptor, data, sizeOfData);

	if( ret > 0 )
		return ret ;
	if(ret < 0 && errno == EWOULDBLOCK )
		return SOCKET_ERROR ;

	// if recv returns<0 and errno!=EWOULDBLOCK -->  then it is something that can be considered as connection-losing
	return SOCKET_DISCONNECT ;
}

ssize_t CSocket::Read(TDescriptor descriptor, char* data, ssize_t sizeOfData)
{
	return recv(descriptor , data, sizeOfData, 0);
}

ssize_t CSocket::ReadBigData(TDescriptor descriptor, char* data, TMinimalSize sizeOfData)
{
	TMinimalSize size;

	int ret_size = Read(descriptor, (char*)&size, (unsigned)sizeof(size));

	if( ret_size == 0 )
		return SOCKET_DISCONNECT ;
	if ( ret_size < 0 )
	{
		if ( errno == EWOULDBLOCK )
			return SOCKET_ERROR ;
		// if recv returns<0 and errno!=EWOULDBLOCK -->  then it is something that can be considered as EOF
		return SOCKET_DISCONNECT ;
	}

	if( size > sizeOfData )
	{
#ifdef _DEBUG
		cerr<<"Error : CSocket : ReadBigData : "<< size << " > "<<sizeOfData<<endl;
#endif
		return SOCKET_ERROR ;
	}

	int ret_data = Read(descriptor, data, size);
	if( ret_data > 0 )
		return ret_data ;

	if ( ret_data < 0 && errno == EWOULDBLOCK )
		return SOCKET_ERROR ;

	// if recv returns<0 and errno!=EWOULDBLOCK -->  then it is something that can be considered as EOF
	return SOCKET_DISCONNECT ;
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
