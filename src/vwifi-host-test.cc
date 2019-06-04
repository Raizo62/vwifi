#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "csocketclient.h"

using namespace std;

int main(int argc , char *argv[])
{
	CSocketClient socket(AF_INET);

	if( ! socket.Connect("127.0.0.1",CTRL_PORT) )
	{
		cerr<<"Error : socket.Connect error"<<endl;
		return 1;
	}

	int value;
	int err;

	value=1;
	err=socket.Send((char*)&value,sizeof(value));
	if( err == SOCKET_ERROR )
	{
		cerr<<"socket.Read error"<<endl;
		return 1;
	}
	value=10;
	err=socket.Send((char*)&value,sizeof(value));
	if( err == SOCKET_ERROR )
	{
		cerr<<"socket.Read error"<<endl;
		return 1;
	}
	value=11;
	err=socket.Send((char*)&value,sizeof(value));
	if( err == SOCKET_ERROR )
	{
		cerr<<"socket.Read error"<<endl;
		return 1;
	}
	value=12;
	err=socket.Send((char*)&value,sizeof(value));
	if( err == SOCKET_ERROR )
	{
		cerr<<"socket.Read error"<<endl;
		return 1;
	}
	value=13;
	err=socket.Send((char*)&value,sizeof(value));
	if( err == SOCKET_ERROR )
	{
		cerr<<"socket.Read error"<<endl;
		return 1;
	}

	socket.Close();

	return 0;
}

