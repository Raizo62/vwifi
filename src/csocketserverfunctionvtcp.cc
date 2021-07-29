#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy
#include <assert.h> // assert

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "csocket.h" // SOCKET_ERROR

#include "csocketserverfunctionvtcp.h"

using namespace std;

bool CSocketServerFunctionVTCP::Configure(TDescriptor& master)
{
	//create a master socket

	master = socket(AF_VSOCK , SOCK_STREAM , 0);

	if( master == SOCKET_ERROR )
	{
		perror("CSocketServerCoreVTCP::_Configure : socket");
		return false;
	}

	return true;
}

bool CSocketServerFunctionVTCP::_Listen(TDescriptor& master, TPort port)
{
	if( ! Configure(master) )
	{
		cerr<<"Error : CSocketServerCoreVTCP::_Listen : Configure"<<endl;
		return false;
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	int opt = 1 ; // TRUE
	if( setsockopt(master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("CSocketServerCoreVTCP::_Listen : setsockopt : SO_REUSEADDR");
		return false;
	}

	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	if (setsockopt (master, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServerCoreVTCP::_Listen : setsockopt : SO_RCVTIMEO\n");
	if (setsockopt (master, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServerCoreVTCP::_Listen : setsockopt : SO_SNDTIMEO\n");

	//type of socket created
	struct sockaddr_vm address;
	address.svm_family = AF_VSOCK;
	address.svm_reserved1 = 0;
	address.svm_port = port;
	address.svm_cid = VMADDR_CID_ANY;
	memset(address.svm_zero, 0, sizeof(address.svm_zero));

	//bind the socket
	if (bind(master, (struct sockaddr*)&address, sizeof(address)) != 0)
	{
		perror("CSocketServerCoreVTCP::_Listen : bind");
		return false;
	}

	cout<<"Listener on port : "<<port<<endl;
	//try to specify maximum of 3 pending connections for the master socket
	if( listen(master, 3) < 0 )
	{
		perror("CSocketServerCoreVTCP::_Listen : listen");
		return false;
	}

	return true;
}

TDescriptor CSocketServerFunctionVTCP::_Accept(TDescriptor master, TCID& cid)
{
	TDescriptor new_socket;

	struct sockaddr_in address;
	int addrlen = sizeof(address);

	if( (new_socket = accept(master, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0 )
	{
		perror("CSocketServerCoreVTCP::_Accept : accept");
		return SOCKET_ERROR;
	}

	cid=((struct sockaddr_vm*)&address)->svm_cid;

	return new_socket;
}
