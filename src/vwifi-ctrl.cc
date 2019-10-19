#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "csocketclient.h"
#include "types.h"
#include "ccoordinate.h" // CCoordinate
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
	cout<<"	loss yes/no"<<endl;
	cout<<"		- loss yes : packets can be lost"<<endl;
	cout<<"		- loss no : no packets can be lost"<<endl;
	cout<<"	show"<<endl;
	cout<<"		- Display the status of loss and list of VMs"<<endl;
	cout<<"	status"<<endl;
	cout<<"		- Display the status of the configuration of vwifi-server"<<endl;
	cout<<"	close"<<endl;
	cout<<"		- Close all the connections with Wifi VMs"<<endl;
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

	TOrder order=TORDER_LIST;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskList : socket.Send : order"<<endl;
		return 1;
	}

	TIndex number;
	err=socket.Read((char*)&number,sizeof(number));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskList : socket.Read : number"<<endl;
		return 1;
	}

	CInfoWifi info;
	for(TIndex i=0; i<number;i++)
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

	TCID cid=atoi(argv[2]);

	if( cid < TCID_GUEST_MIN )
	{
			cerr<<"Error : ChangeCoordinate : the CID must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	TValue x=atoi(argv[3]);
	TValue y=atoi(argv[4]);
	TValue z=atoi(argv[5]);
	CCoordinate coo(x,y,z);

	cout<<cid<<" "<<coo<<" "<<endl;

	CSocketClient socket(AF_INET);

	if( ! socket.Connect(ADDRESS_IP,CTRL_PORT) )
	{
		cerr<<"Error : ChangeCoordinate : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_CHANGE_COORDINATE;
	err=socket.Send((char*)&order,sizeof(order));
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

int ChangePacketLoss(int argc, char *argv[])
{
	if( argc != 3 )
	{
			cerr<<"Error : loss : the number of parameter is uncorrect"<<endl;
			Help(argv[0]);
			return 1;
	}

	int value;
	if ( ! strcmp(argv[2],"yes") )
		value=1;
	else if ( ! strcmp(argv[2],"no") )
		value=0;
	else
	{
			cerr<<"Error : loss : the value can only be \"yes\" or \"no\""<<endl;
			return 1;
	}

	CSocketClient socket(AF_INET);

	if( ! socket.Connect(ADDRESS_IP,CTRL_PORT) )
	{
		cerr<<"Error : loss : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_PACKET_LOSS;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : loss : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&value,sizeof(value));
	if( err == SOCKET_ERROR )
	{
		if ( value  )
			cerr<<"Error : loss : socket.Send : yes"<<endl;
		else
			cerr<<"Error : loss : socket.Send : no"<<endl;

		return 1;
	}

	socket.Close();

	return 0;
}

int AskStatus()
{
	CSocketClient socket(AF_INET);

	cout<<"CTRL : IP : "<<ADDRESS_IP<<endl;
	cout<<"CTRL : Port : "<<CTRL_PORT<<endl;

	if( ! socket.Connect(ADDRESS_IP,CTRL_PORT) )
	{
		cerr<<"Error : AskStatus : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_STATUS;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskStatus : socket.Send : order"<<endl;
		return 1;
	}

	TSocket type;
	err=socket.Read((char*)&type,sizeof(type));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskStatus : socket.Read : Type"<<endl;
		return 1;
	}
	switch ( type )
	{
		case AF_VSOCK :
		{
			cout<<"SRV : Type : AF_VSOCK"<<endl;

			break ;
		}

		case AF_INET :
		{
			cout<<"SRV : Type : AF_INET"<<endl;

			break ;
		}
	}

	TPort port;
	err=socket.Read((char*)&port,sizeof(port));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskStatus : socket.Read : Type"<<endl;
		return 1;
	}
	cout<<"SRV : Port : "<<port<<endl;

	TIndex size;
	err=socket.Read((char*)&size,sizeof(size));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskStatus : socket.Read : Type"<<endl;
		return 1;
	}
	cout<<"SRV : Size of disconnected : "<<size<<endl; // 15

	bool loss;
	err=socket.Read((char*)&loss,sizeof(loss));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskStatus : socket.Read : Type"<<endl;
		return 1;
	}
	if ( loss )
		cout<<"SRV : Packet loss : Enable"<<endl;
	else
		cout<<"SRV : Packet loss : Disable"<<endl;

	socket.Close();

	return 0;
}

int AskShow()
{
	CSocketClient socket(AF_INET);

	if( ! socket.Connect(ADDRESS_IP,CTRL_PORT) )
	{
		cerr<<"Error : AskStatus : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_SHOW;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskStatus : socket.Send : order"<<endl;
		return 1;
	}

	bool loss;
	err=socket.Read((char*)&loss,sizeof(loss));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : AskShow : socket.Read : Type"<<endl;
		return 1;
	}
	if ( loss )
		cout<<"Packet loss : Enable"<<endl;
	else
		cout<<"Packet loss : Disable"<<endl;

	socket.Close();

	cout<<"----------------"<<endl;

	return AskList();
}

int CloseAllClient()
{
	CSocketClient socket(AF_INET);

	if( ! socket.Connect(ADDRESS_IP,CTRL_PORT) )
	{
		cerr<<"Error : CloseAllClient : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_CLOSE_ALL_CLIENT;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : CloseAllClient : socket.Send : order"<<endl;
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

	if( ! strcmp(argv[1],"loss") )
		return ChangePacketLoss(argc, argv);

	if( ! strcmp(argv[1],"show") )
		return AskShow();

	if( ! strcmp(argv[1],"status") )
		return AskStatus();

	if( ! strcmp(argv[1],"close") )
		return CloseAllClient();

	cerr<<argv[0]<<" : Error : unknown order : "<<argv[1]<<endl;

	return 1;
}

