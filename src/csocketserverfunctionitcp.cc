#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy
#include <assert.h> // assert

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>

#include "csocket.h" // SOCKET_ERROR
#include "csocketserverfunctionitcp.h"
#include "tools.h"

using namespace std;

bool CSocketServerFunctionITCP::Configure(TDescriptor& master)
{
	//create a master socket

	master = socket(AF_INET , SOCK_STREAM , 0);

	if( master == SOCKET_ERROR )
	{
		perror("CSocketServerCoreITCP::Configure : socket");
		return false;
	}

	return true;
}

bool CSocketServerFunctionITCP::_Listen(TDescriptor& master, TPort port)
{
	if( ! Configure(master) )
	{
		cerr<<"Error : CSocketServerCoreITCP::_Listen : Configure"<<endl;
		return false;
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	int opt = 1 ; // TRUE
	if( setsockopt(master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("CSocketServerCoreITCP::_Listen : setsockopt : SO_REUSEADDR");
		return false;
	}

	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	if (setsockopt (master, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServerCoreITCP::_Listen : setsockopt : SO_RCVTIMEO\n");
	if (setsockopt (master, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServerCoreITCP::_Listen : setsockopt : SO_SNDTIMEO\n");

	//type of socket created
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );

	//bind the socket
	if (bind(master, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("CSocketServer::_Listen : bind");
		return false;
	}

	cout<<"Listener on port : "<<port<<endl;
	//try to specify maximum of 3 pending connections for the master socket
	if( listen(master, 3) < 0 )
	{
		perror("CSocketServerCoreITCP::_Listen : listen");
		return false;
	}

	return true;
}

TDescriptor CSocketServerFunctionITCP::_Accept(TDescriptor master, TCID& cid)
{
	TDescriptor new_socket;

	struct sockaddr_in address;
	int addrlen = sizeof(address);

	if( (new_socket = accept(master, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 )
	{
		perror("CSocketServerCoreITCP::_Accept : accept");
		return SOCKET_ERROR;
	}

	cid=hash_ipaddr(&address);

	return new_socket;
}
