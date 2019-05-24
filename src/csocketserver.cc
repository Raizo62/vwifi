#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <linux/vm_sockets.h> // struct sockaddr_vm
#include <unistd.h> // close

#include "csocketserver.h"

const int TRUE=1;

using namespace std;

CSocketServer::CSocketServer() : CSocket()
{
};

CSocketServer::CSocketServer(TypeSocket type) : CSocket(type)
{
};

void CSocketServer::Init(unsigned int port)
{
	Port=port;

	NumberClient=0;
}

CSocketServer::~CSocketServer()
{
	for (unsigned int i = 0; i < NumberClient; i++)
		close(SocketClients[i]);

	CSocket::Close();
}

bool CSocketServer::Listen()
{
	//create a master socket
	if( ! Configure() )
	{
		perror("socket failed");
		return false;
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	int opt = TRUE;
	if( setsockopt(Master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
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
					perror("bind failed");
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
					perror("bind failed");
					return false;
				}

				break;
			}
	}

	cout<<"Listener on port "<<Port<<endl;
	//try to specify maximum of 3 pending connections for the master socket
	if( listen(Master, 3) < 0 )
	{
		perror("listen");
		return false;
	}

	return true;
}

Descriptor CSocketServer::Accept()
{
	Descriptor new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	if ((new_socket = accept(Master, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
	{
		perror("accept");
		return SOCKET_ERROR;
	}

	if ( NumberClient >= MAX_CLIENT )
	{
		cerr<<"Error : CSocketServer::Accept : too much clients"<<endl;
		return SOCKET_ERROR;
	}

	//add new socket to array of sockets
	cout<<"Adding to list of sockets as "<<NumberClient<<endl;
	SocketClients[NumberClient] = new_socket;
	switch ( Type )
	{
		case AF_VSOCK :
			{
				InfoClient[NumberClient] = ((struct sockaddr_vm*)&address)->svm_cid;
				break ;
			}

		case AF_INET :
			{
				InfoClient[NumberClient] = ntohs(address.sin_port);
				break;
			}
	}

	NumberClient++;

	return new_socket;
}

Descriptor CSocketServer::GetSocketClient(unsigned int number)
{
	if( number >= NumberClient )
		return SOCKET_ERROR;

	return SocketClients[number];
}

unsigned int CSocketServer::GetNumberClient()
{
	return NumberClient;
}

void CSocketServer::CloseClient(unsigned int number)
{
	if( number >= NumberClient )
		return;

	close(SocketClients[number]);

	NumberClient--;

	for (unsigned int i = number; i < NumberClient; i++)
	{
		SocketClients[i] = SocketClients[i+1];
		InfoClient[i] = InfoClient[i+1];
	}

	return;
}

void CSocketServer::ShowInfoClient(unsigned int number)
{
	cout<<"["<<InfoClient[number]<<"]";
}

void CSocketServer::SendAllOtherClients(unsigned int number,const char* data, ssize_t sizeOfData)
{
	cout<<"SendAllOtherClient de "<<number<<" vers "<<NumberClient<<" clients ("<<sizeOfData<<" octets)"<<endl;
	for (unsigned int i = 0; i < NumberClient; i++)
	{
		if( i != number )
			Send(SocketClients[i], data, sizeOfData);
	}
}

ssize_t CSocketServer::Send(Descriptor descriptor, const char* data, ssize_t sizeOfData)
{
	return CSocket::Send(descriptor, data, sizeOfData);
}

ssize_t CSocketServer::Read(Descriptor descriptor, char* data, ssize_t sizeOfData)
{
	return CSocket::Read(descriptor , data, sizeOfData);
}
