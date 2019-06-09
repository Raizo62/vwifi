#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <linux/vm_sockets.h> // struct sockaddr_vm
#include <unistd.h> // close

#include "csocketserver.h"

using namespace std;

CSocketServer::CSocketServer() : CSocket()
{
	NumberClient=0;
	MaxClient=0;
	SocketClients=NULL;
};

CSocketServer::CSocketServer(TSocket type) : CSocket(type)
{
	NumberClient=0;
	MaxClient=0;
	SocketClients=NULL;
};

void CSocketServer::Init(TPort port)
{
	Port=port;
}

CSocketServer::~CSocketServer()
{
	for (TNumber i = 0; i < NumberClient; i++)
		close(SocketClients[i]);

	CSocket::Close();
}

bool CSocketServer::Listen(TNumber maxClient)
{
	MaxClient=maxClient;

	//create a master socket
	if( ! Configure() )
	{
		cerr<<"Error : CSocketServer::Listen : Configure"<<endl;
		return false;
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	int opt = 1 ; // TRUE
	if( setsockopt(Master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("CSocketServer::Listen : setsockopt");
		return false;
	}

	switch ( Type )
	{
		case AF_VSOCK :
			{
				//type of socket created
				struct sockaddr_vm address {
					.svm_family = AF_VSOCK,
					.svm_reserved1 = 0,
					.svm_port = Port,
					.svm_cid = VMADDR_CID_ANY
				};

				//bind the socket
				if (bind(Master, (struct sockaddr*)&address, sizeof(address)) != 0)
				{
					perror("CSocketServer::Listen : bind");
					return false;
				}

				break ;
			}

		case AF_INET :
			{
				//type of socket created
				struct sockaddr_in address;
				address.sin_family = AF_INET;
				address.sin_addr.s_addr = INADDR_ANY;
				address.sin_port = htons( Port );

				//bind the socket
				if (bind(Master, (struct sockaddr *)&address, sizeof(address))<0)
				{
					perror("CSocketServer::Listen : bind");
					return false;
				}

				break;
			}
	}

	cout<<"Listener on port "<<Port<<endl;
	//try to specify maximum of 3 pending connections for the master socket
	if( listen(Master, 3) < 0 )
	{
		perror("CSocketServer::Listen : listen");
		return false;
	}

	SocketClients = new TDescriptor [ MaxClient ];
	if( SocketClients == NULL )
	{
		perror("CSocketServer::Listen : new");
		return false;
	}

	return true;
}

TDescriptor CSocketServer::Accept()
{
	struct sockaddr_in address;

	return Accept(address);
}

TDescriptor CSocketServer::Accept(struct sockaddr_in& address)
{
	TDescriptor new_socket;

	int addrlen = sizeof(address);

	if ((new_socket = accept(Master, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
	{
		perror("CSocketServer::Accept : accept");
		return SOCKET_ERROR;
	}

	if ( NumberClient >= MaxClient )
	{
		cerr<<"Error : CSocketServer::Accept : too much clients"<<endl;
		return SOCKET_ERROR;
	}

	//add new socket to array of sockets
	SocketClients[NumberClient] = new_socket;

	NumberClient++;

	return new_socket;
}

TDescriptor CSocketServer::GetSocketClient(TNumber number)
{
	if( number >= NumberClient )
		return SOCKET_ERROR;

	return SocketClients[number];
}

TNumber CSocketServer::GetNumberClient()
{
	return NumberClient;
}

void CSocketServer::CloseClient(TNumber number)
{
	if( number >= NumberClient )
		return;

	close(SocketClients[number]);

	NumberClient--;

	// SocketClients : [number,NumberClient[ <-=- [number+1,NumberClient]
	if( number <  NumberClient )
		memcpy(&(SocketClients[number]),&(SocketClients[number+1]),(NumberClient-number)*sizeof(TDescriptor));
}

ssize_t CSocketServer::Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData)
{
	return CSocket::Send(descriptor, data, sizeOfData);
}

ssize_t CSocketServer::Read(TDescriptor descriptor, char* data, ssize_t sizeOfData)
{
	return CSocket::Read(descriptor , data, sizeOfData);
}
