#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "csocketclient.h"
#include "cctrlserver.h" // the type of orders
#include "ccoordinate.h" // CCoordinate and Type of x, y and z
#include "cinfowifi.h"

using namespace std;

void Help(char* nameOfProg)
{
	cout<<nameOfProg<<" [order]"<<endl;
	cout<<" with [order] :"<<endl;
	cout<<"	ls"<<endl;
	cout<<"		- List the VMs"<<endl;
	cout<<"	set cid x y z"<<endl;
	cout<<"		- Change the coordinate of the VM with cid"<<endl;
}

int AskList()
{
	CSocketClient socket(AF_INET);

	if( ! socket.Connect(ADDRESS_IP,CTRL_PORT) )
	{
		cerr<<"Error : AskList : socket.Connect error"<<endl;
		return 1;
	}

	int err;
	err=socket.Send((char*)&ORDER_LIST,sizeof(ORDER_LIST));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskList : socket.Send : order"<<endl;
		return 1;
	}

	unsigned int number;
	err=socket.Read((char*)&number,sizeof(number));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskList : socket.Read : number"<<endl;
		return 1;
	}

	CInfoWifi info;
	for(unsigned int i=0; i<number;i++)
	{
		err=socket.Read((char*)&info,sizeof(info));
		if( err == SOCKET_ERROR )
		{
			cerr<<"Error : AskList : socket.Read : CInfoWifi"<<endl;
			return 1;
		}
		cout<<info<<endl;
	}

	socket.Close();

	return 0;
}

int ChangeCoordinate(int argc, char *argv[])
{
	if( argc != 6 )
	{
			cerr<<"Error : ChangeCoordinate : the number of parameter is uncorrect"<<endl;
			Help(argv[0]);
			return 1;
	}

	TypeCID cid=atoi(argv[2]);
	Value x=atoi(argv[3]);
	Value y=atoi(argv[4]);
	Value z=atoi(argv[5]);
	CCoordinate coo(x,y,z);

	cout<<cid<<" "<<coo<<" "<<endl;

	CSocketClient socket(AF_INET);

	if( ! socket.Connect(ADDRESS_IP,CTRL_PORT) )
	{
		cerr<<"Error : ChangeCoordinate : socket.Connect error"<<endl;
		return 1;
	}

	int err;
	err=socket.Send((char*)&ORDER_CHANGE_COORDINATE,sizeof(ORDER_CHANGE_COORDINATE));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ChangeCoordinate : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&cid,sizeof(cid));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ChangeCoordinate : socket.Send : cid"<<endl;
		return 1;
	}
	err=socket.Send((char*)&coo,sizeof(coo));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ChangeCoordinate : socket.Send : "<<coo<<endl;
		return 1;
	}

	socket.Close();

	return 0;
}

int main(int argc , char *argv[])
{
	if( argc == 1 )
	{
		Help(argv[0]);
		return 0;
	}

	if( ! strcmp(argv[1],"ls") )
		return AskList();

	if( ! strcmp(argv[1],"set") )
		return ChangeCoordinate(argc, argv);

	return 0;
}

