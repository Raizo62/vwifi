#include <iostream> // cout

#include <string.h> //strlen

#include "vwifi-guest.h"
#include "cvsocketclient.h"

using namespace std;

int main(int argc , char *argv[])
{
	CVSocketClient socket;

	if( ! socket.Connect(ADDRESS_IP,PORT) )
	{
		cout<<"socket.Connect error"<<endl;
		return 1;
	}

	char buffer[1025]; //data buffer of 1K

	int value=socket.Read(buffer,sizeof(buffer));
	if( value == SOCKET_ERROR )
	{
		cout<<"socket.Read error"<<endl;
		return 1;
	}

	buffer[value]='\0';
	string sbuffer(buffer);

	cout<<sbuffer<<endl;

	int number;
	for(int i=0; i< 3 ; i++ )
	{
		number=i+1;
		sprintf(buffer,"%d\n",number);
		value=socket.Send((char*)buffer,strlen(buffer));
		if( value == SOCKET_ERROR )
		{
			cout<<"socket.Send error"<<endl;
			return 1;
		}
	}

	return 0;
}

