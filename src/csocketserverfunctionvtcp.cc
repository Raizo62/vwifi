#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memset
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
		perror("CSocketServerFunctionVTCP::_Configure : socket");
		return false;
	}

	return true;
}

bool CSocketServerFunctionVTCP::_Listen(TDescriptor& master, TPort port)
{
	if( ! Configure(master) )
	{
		cerr<<"Error : CSocketServerFunctionVTCP::_Listen : Configure"<<endl;
		return false;
	}

	//type of socket created
	struct sockaddr_vm address;
	memset(&address, 0, sizeof(address));
	address.svm_family = AF_VSOCK;
	address.svm_port = port;
	address.svm_cid = VMADDR_CID_ANY;
	memset(address.svm_zero, 0, sizeof(address.svm_zero));

	//bind the socket
	if (bind(master, (struct sockaddr*)&address, sizeof(address)) != 0)
	{
		perror("CSocketServerFunctionVTCP::_Listen : bind");
		return false;
	}

	/* Seems useless with VSOCK : the server detects immediately that a client is disconnected
	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	if (setsockopt (master, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServerFunctionVTCP::_Listen : setsockopt : SO_RCVTIMEO\n");
	if (setsockopt (master, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServerFunctionVTCP::_Listen : setsockopt : SO_SNDTIMEO\n");
	*/

	cout<<"Listener on port : "<<port<<endl;
	//try to specify maximum of 3 pending connections for the master socket
	if( listen(master, 3) < 0 )
	{
		perror("CSocketServerFunctionVTCP::_Listen : listen");
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
		perror("CSocketServerFunctionVTCP::_Accept : accept");
		return SOCKET_ERROR;
	}

	cid=((struct sockaddr_vm*)&address)->svm_cid;

	return new_socket;
}

