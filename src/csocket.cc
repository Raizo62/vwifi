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

ssize_t CSocket::Read(TDescriptor descriptor, char* data, ssize_t sizeOfData)
{
	return recv(descriptor , data, sizeOfData, 0);
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
