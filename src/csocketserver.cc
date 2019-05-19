#include <iostream> // cout
#include <cstdio> //perror

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <unistd.h> // close

#include "csocketserver.h"

const int TRUE=1;
const int FALSE=1;

using namespace std;

void CSocketServer::Init(int port)
{
	Port=port;

	//initialise all client_socket[] to 0 so not checked
	for (int i = 0; i < MAX_CLIENT; i++)
	{
		SocketClients[i] = 0;
	}
	NumberClient=0;
}

bool CSocketServer::Listen()
{
	//create a master socket
	if( Configure() )
	{
		perror("socket failed");
		return SOCKET_ERROR;
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	int opt = TRUE;
	if( setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		return SOCKET_ERROR;
	}

	//type of socket created
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( Port );

	//bind the socket
	if (bind(MasterSocket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		return SOCKET_ERROR;
	}
	cout<<"Listener on port "<<Port<<endl;

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(MasterSocket, 3) < 0)
	{
		perror("listen");
		return SOCKET_ERROR;
	}

	return 0;
}

int CSocketServer::Accept()
{
	int new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	if ((new_socket = accept(MasterSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
	{
		perror("accept");
		return SOCKET_ERROR;
	}

	if ( NumberClient >= MAX_CLIENT )
		return SOCKET_ERROR;

	//add new socket to array of sockets
	cout<<"Adding to list of sockets as "<<NumberClient<<endl;
	SocketClients[NumberClient++] = new_socket;

	return new_socket;
}

int CSocketServer::GetSocketClient(int number)
{
	if( number < 0 || number > NumberClient )
		return SOCKET_ERROR;

	return SocketClients[number];
}

int CSocketServer::GetNumberClient()
{
	return NumberClient;
}

void CSocketServer::CloseClient(int number)
{
	if( number < 0 || number > NumberClient )
		return;

	close(SocketClients[number]);

	for (int i = number; i < MAX_CLIENT-1; i++)
	{
		SocketClients[i] = SocketClients[i+1];
	}
	NumberClient--;

	return;
}

void CSocketServer::SendAllOtherClients(int number,const char* data, ssize_t sizeOfData)
{
	cout<<"SendAllOtherClient de "<<number<<" vers "<<NumberClient<<" clients ("<<sizeOfData<<" octets)"<<endl;
	for (int i = 0; i < NumberClient; i++)
	{
		if( i != number )
			Send(SocketClients[i], data, sizeOfData);
	}
}
