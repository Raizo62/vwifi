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
	cout<<"	distance cid1 cid2"<<endl;
	cout<<"		- Distance in meters between the VM with cid1 and the VM with cid2"<<endl;
	cout<<"	close"<<endl;
	cout<<"		- Close all the connections with Wifi VMs"<<endl;

	cout<<" -v or --version"<<endl;
	cout<<"	Display the version of "<<nameOfProg<<endl;
}

int AskList()
{
	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,CTRL_PORT);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : ls : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_LIST;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ls : socket.Send : order"<<endl;
		return 1;
	}

	TIndex number;
	err=socket.Read((char*)&number,sizeof(number));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ls : socket.Read : number"<<endl;
		return 1;
	}

	CInfoWifi info;
	for(TIndex i=0; i<number;i++)
	{
		err=socket.Read((char*)&info,sizeof(info));
		if( err == SOCKET_ERROR )
		{
			cerr<<"Error : ls : socket.Read : CInfoWifi"<<endl;
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
			cerr<<"Error : set : the number of parameter is uncorrect"<<endl;
			Help(argv[0]);
			return 1;
	}

	TCID cid=atoi(argv[2]);

	if( cid < TCID_GUEST_MIN )
	{
			cerr<<"Error : set : the CID must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	TValue x=atoi(argv[3]);
	TValue y=atoi(argv[4]);
	TValue z=atoi(argv[5]);
	CCoordinate coo(x,y,z);

	cout<<cid<<" "<<coo<<" "<<endl;

	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,CTRL_PORT);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : set : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_CHANGE_COORDINATE;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : set : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&cid,sizeof(cid));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : set : socket.Send : cid"<<endl;
		return 1;
	}
	err=socket.Send((char*)&coo,sizeof(coo));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : set : socket.Send : "<<coo<<endl;
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

	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,CTRL_PORT);

	if( ! socket.ConnectLoop() )
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
	CSocketClientINET socket;

	cout<<"CTRL : IP : "<<ADDRESS_IP<<endl;
	cout<<"CTRL : Port : "<<CTRL_PORT<<endl;

	socket.Init(ADDRESS_IP,CTRL_PORT);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : status : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_STATUS;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Send : Order"<<endl;
		return 1;
	}

	bool loss;
	err=socket.Read((char*)&loss,sizeof(loss));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Loss"<<endl;
		return 1;
	}
	cout<<"SRV : PacketLoss : ";
	if ( loss )
		cout<<"Enable"<<endl;
	else
		cout<<"Disable"<<endl;

	// VHOST

	TPort port;
	err=socket.Read((char*)&port,sizeof(port));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Port"<<endl;
		return 1;
	}
	cout<<"SRV VHOST: Port : "<<port<<endl;

	TIndex size;
	err=socket.Read((char*)&size,sizeof(size));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Size"<<endl;
		return 1;
	}
	cout<<"SRV VHOST: SizeOfDisconnected : "<<size<<endl;

	// INET

	err=socket.Read((char*)&port,sizeof(port));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Port"<<endl;
		return 1;
	}
	cout<<"SRV INET: Port : "<<port<<endl;

	err=socket.Read((char*)&size,sizeof(size));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Size"<<endl;
		return 1;
	}
	cout<<"SRV INET: SizeOfDisconnected : "<<size<<endl;

	// HOST

	bool hostIsConnected;
	err=socket.Read((char*)&hostIsConnected,sizeof(hostIsConnected));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : HostIsConnected"<<endl;
		return 1;
	}
	cout<<"HOST : ";
	if ( hostIsConnected )
		cout<<"Connected"<<endl;
	else
		cout<<"Disconnected"<<endl;

	socket.Close();

	return 0;
}

int AskShow()
{
	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,CTRL_PORT);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : show : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_SHOW;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Send : Order"<<endl;
		return 1;
	}

	bool loss;
	err=socket.Read((char*)&loss,sizeof(loss));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Read : Loss"<<endl;
		return 1;
	}
	cout<<"PacketLoss : ";
	if ( loss )
		cout<<"Enable"<<endl;
	else
		cout<<"Disable"<<endl;

	bool hostIsConnected;
	err=socket.Read((char*)&hostIsConnected,sizeof(hostIsConnected));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Read : HostIsConnected"<<endl;
		return 1;
	}
	cout<<"Host : ";
	if ( hostIsConnected )
		cout<<"Connected"<<endl;
	else
		cout<<"Disconnected"<<endl;

	socket.Close();

	cout<<"----------------"<<endl;

	return AskList();
}

int DistanceBetweenCID(int argc, char *argv[])
{
	if( argc != 4 )
	{
			cerr<<"Error : distance : the number of parameter is uncorrect"<<endl;
			Help(argv[0]);
			return 1;
	}

	TCID cid1=atoi(argv[2]);

	if( cid1 < TCID_GUEST_MIN )
	{
			cerr<<"Error : distance : the CID 1 must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	TCID cid2=atoi(argv[3]);

	if( cid2 < TCID_GUEST_MIN )
	{
			cerr<<"Error : distance : the CID2 must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,CTRL_PORT);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : distance : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_DISTANCE_BETWEEN_CID;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&cid1,sizeof(cid1));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Send : cid 1"<<endl;
		return 1;
	}
	err=socket.Send((char*)&cid2,sizeof(cid2));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Send : cid 2"<<endl;
		return 1;
	}

	int codeError;
	err=socket.Read((char*)&codeError,sizeof(codeError));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Read : codeError"<<endl;
		return 1;
	}

	if ( codeError == -1 )
	{
		cerr<<"Error : distance : unknown cid 1 : "<<cid1<<endl;
		return 1;
	}
	if ( codeError == -2 )
	{
		cerr<<"Error : distance : unknown cid 2 : "<<cid2<<endl;
		return 1;
	}

	TDistance distance;
	err=socket.Read((char*)&distance,sizeof(distance));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : distance : socket.Read : distance"<<endl;
		return 1;
	}

	cout<<"Distance : "<<distance<<endl;

	socket.Close();

	return 0;
}

int CloseAllClient()
{
	CSocketClientINET socket;

	socket.Init(ADDRESS_IP,CTRL_PORT);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : close : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_CLOSE_ALL_CLIENT;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : close : socket.Send : order"<<endl;
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

	if( ! strcmp(argv[1],"-v") || ! strcmp(argv[1],"--version") )
	{
		cout<<"Version : "<<VERSION<<endl;
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

	if( ! strcmp(argv[1],"distance") )
		return DistanceBetweenCID(argc, argv);

	if( ! strcmp(argv[1],"close") )
		return CloseAllClient();

	cerr<<argv[0]<<" : Error : unknown order : "<<argv[1]<<endl;

	return 1;
}

