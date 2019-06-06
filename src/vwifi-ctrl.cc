#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "csocketclient.h"
#include "cctrlserver.h" // the type of orders
#include "ccoordinate.h" // CCoordinate and Type of x, y and z
#include "cinfowifi.h" //Type of CID

using namespace std;

void Help(char* nameOfProg)
{
	cout<<nameOfProg<<" [order]"<<endl;
	cout<<" with [order] :"<<endl;
	cout<<"	-	Change the coordinate of the VM with cid :"<<endl;
	cout<<"			coo cid x y z"<<endl;
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
	err=socket.Send((char*)&CHANGE_COORDINATE,sizeof(CHANGE_COORDINATE));
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

	return ChangeCoordinate(argc, argv);

	return 0;
}

