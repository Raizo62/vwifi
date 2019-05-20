#include <cstdio> //perror

#include <iostream> // cout

#include <sys/socket.h> //socket
#include <arpa/inet.h> // struct sockaddr_in & inet_ntoa & ntohs
#include <unistd.h> // close

#include "cvsocket.h"

using namespace std;

CVSocket::CVSocket()
{
	Master=0;
}

CVSocket::~CVSocket()
{
	Close();
}

bool CVSocket::Configure()
{
	//create a master socket
	Master = socket(AF_INET , SOCK_STREAM , 0);
	if( Master == SOCKET_ERROR )
	{
		perror("socket : failed");
		return false;
	}

	return true;
}

ssize_t CVSocket::Send(Descriptor descriptor, const char* data, ssize_t sizeOfData)
{
	return send(descriptor, data, sizeOfData, 0);
}

ssize_t CVSocket::Read(Descriptor descriptor, char* data, ssize_t sizeOfData)
{
	return recv(descriptor , data, sizeOfData, 0);
}

void CVSocket::ShowInfo(Descriptor descriptor)
{
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	getpeername(descriptor , (struct sockaddr*)&address , (socklen_t*)&addrlen);
	cout<<"ip "<<inet_ntoa(address.sin_addr)<<" , port "<<ntohs(address.sin_port);
}

CVSocket::operator int()
{
	return Master;
}

void CVSocket::Close()
{
	close(Master);
}


