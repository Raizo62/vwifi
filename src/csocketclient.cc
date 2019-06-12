#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <linux/vm_sockets.h> // struct sockaddr_vm
#include <unistd.h> // close

#include "csocketclient.h"

#include <fcntl.h>

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
	//create a master socket
	if( ! Configure() )
	{
		cerr<<"Error : CSocketClient::Connect : Configure"<<endl;
		return false;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
	server.sin_port = htons(port);

	if( connect(Master,(struct sockaddr*) &server,sizeof(server)) != 0 )
	{
		perror("CSocketClient::Connect : connect");
		return false;
	}

	IsConnected=true;

	return true;
}

bool CSocketClient::Connect(TPort port)
{
	//create a master socket
	if( ! Configure() )
	{
		cerr<<"Error : CSocketClient::Connect : Configure"<<endl;
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
		perror("CSocketClient::Connect : connect");
		return false;
	}

	IsConnected=true;

	return true;
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



/** 
 * \fn int SetBloking(int blocking) 
 * \biref Set a file descriptor to blocking or non-blocking mode.
 *
 * \param blocking 0:non-blocking mode, 1:blocking mode
 *
 * \return 1:success, 0:failure.
 **/
int CSocketClient::SetBloking(int blocking) {
    
   /* Save the current flags */
    int flags = fcntl(Master, F_GETFL, 0);
    if (flags == -1)
        return 0;

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    return fcntl(Master, F_SETFL, flags) != -1;

}
