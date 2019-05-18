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
		return ERROR_SOCKET;
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	int opt = TRUE;
	if( setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		return ERROR_SOCKET;
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
		return ERROR_SOCKET;
	}
	cout<<"Listener on port "<<Port<<endl;

	//try to specify maximum of 3 pending connections for the master socket
	if (listen(MasterSocket, 3) < 0)
	{
		perror("listen");
		return ERROR_SOCKET;
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
		return ERROR_SOCKET;
	}

	if ( NumberClient >= MAX_CLIENT )
		return ERROR_SOCKET;

	//add new socket to array of sockets
	cout<<"Adding to list of sockets as "<<NumberClient<<endl;
	SocketClients[NumberClient++] = new_socket;

	return new_socket;
}

int CSocketServer::GetSocketClient(int number)
{
	if( number < 0 || number > NumberClient )
		return ERROR_SOCKET;

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

int CSocketServer::MaxDescriptor()
{
	int max=MasterSocket;

	for (int i = 0; i < MAX_CLIENT-1; i++)
	{
		if ( SocketClients[i] > max )
			max = SocketClients[i];
	}
	return max;
}
