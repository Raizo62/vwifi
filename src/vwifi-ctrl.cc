#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "csocketclientitcp.h"
#include "types.h"
#include "ccoordinate.h" // CCoordinate
#include "cinfowifi.h"

using namespace std;

std::string IP_Ctrl = std::string(DEFAULT_ADDRESS_IP);
TPort Port_Ctrl = DEFAULT_CTRL_PORT;

char* NameOfProg;

void Help()
{
	cout<<NameOfProg<<" [order]"<<endl;
	cout<<" with [order] :"<<endl;
	cout<<"	ls"<<endl;
	cout<<"		- List the Clients"<<endl;
	cout<<"	set cid x y z"<<endl;
	cout<<"		- Change the coordinate of the Client with cid"<<endl;
	cout<<"	loss yes/no"<<endl;
	cout<<"		- loss yes : packets can be lost"<<endl;
	cout<<"		- loss no : no packets can be lost"<<endl;
	cout<<"	show"<<endl;
	cout<<"		- Display the status of loss and list of Clients"<<endl;
	cout<<"	status"<<endl;
	cout<<"		- Display the status of the configuration of vwifi-server"<<endl;
	cout<<"	distance cid1 cid2"<<endl;
	cout<<"		- Distance in meters between the Client with cid1 and the Client with cid2"<<endl;
	cout<<"	scale value"<<endl;
	cout<<"		- Set the scale of the distances between the clients to value"<<endl;
	cout<<"		- value can be a decimal number"<<endl;
	cout<<"	close"<<endl;
	cout<<"		- Close all the connections with Wifi Clients"<<endl;
	cout<<endl;
	cout<<" [-p PORT] or [--port PORT] : Set the port used by the vwifi-server"<<endl;
	cout<<" [-i IP] or [--ip IP] : Set the IP used by the vwifi-server"<<endl;
	cout<<" [-v] or [--version] : Display the version of "<<NameOfProg<<endl;
	cout<<" [-h] or [--help] : this help"<<endl;
}

int AskList()
{
	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

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

	// Spies :

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
		cout<<"S:"<<info.GetCid()<<endl;
	}

	// Clients :

	err=socket.Read((char*)&number,sizeof(number));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : ls : socket.Read : number"<<endl;
		return 1;
	}

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
	if( argc != 5 )
	{
			cerr<<"Error : set : the number of parameter is uncorrect"<<endl;
			Help();
			return 1;
	}

	TCID cid=atoi(argv[1]);

	if( cid < TCID_GUEST_MIN )
	{
			cerr<<"Error : set : the CID must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	TValue x=atoi(argv[2]);
	TValue y=atoi(argv[3]);
	TValue z=atoi(argv[4]);
	CCoordinate coo(x,y,z);

	cout<<cid<<" "<<coo<<" "<<endl;

	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

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
	if( argc != 2 )
	{
			cerr<<"Error : loss : the number of parameter is uncorrect"<<endl;
			Help();
			return 1;
	}

	int value;
	if ( ! strcmp(argv[1],"yes") )
		value=1;
	else if ( ! strcmp(argv[1],"no") )
		value=0;
	else
	{
			cerr<<"Error : loss : the value can only be \"yes\" or \"no\""<<endl;
			return 1;
	}

	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

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
	cout<<"CTRL : IP : "<<IP_Ctrl.c_str()<<endl;
	cout<<"CTRL : Port : "<<Port_Ctrl<<endl;

	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

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

	TScale scale;
	err=socket.Read((char*)&scale,sizeof(scale));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : scale"<<endl;
		return 1;
	}
	cout<<"SRV : Scale : "<<scale<<endl;

	// VHOST

	TPort port;
	err=socket.Read((char*)&port,sizeof(port));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Port VHOST"<<endl;
		return 1;
	}
	cout<<"SRV VHOST : Port : "<<port<<endl;

	// INET

	err=socket.Read((char*)&port,sizeof(port));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Port INET"<<endl;
		return 1;
	}
	cout<<"SRV INET : Port : "<<port<<endl;

	// SizeOfDisconnected
	// becareful : the same List is shared by WifiServerVTCP and WifiServerITCP

	TIndex size;
	err=socket.Read((char*)&size,sizeof(size));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : Size INET"<<endl;
		return 1;
	}
	cout<<"SRV : SizeOfDisconnected : "<<size<<endl;

	// SPY

	bool spyIsConnected;
	err=socket.Read((char*)&spyIsConnected,sizeof(spyIsConnected));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : status : socket.Read : spyIsConnected"<<endl;
		return 1;
	}
	cout<<"SPY : ";
	if ( spyIsConnected )
		cout<<"Connected"<<endl;
	else
		cout<<"Disconnected"<<endl;

	socket.Close();

	return 0;
}

int AskShow()
{
	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

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

	TScale scale;
	err=socket.Read((char*)&scale,sizeof(scale));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Read : scale"<<endl;
		return 1;
	}
	cout<<"Scale : "<<scale<<endl;

	bool spyIsConnected;
	err=socket.Read((char*)&spyIsConnected,sizeof(spyIsConnected));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : show : socket.Read : spyIsConnected"<<endl;
		return 1;
	}
	cout<<"Spy : ";
	if ( spyIsConnected )
		cout<<"Connected"<<endl;
	else
		cout<<"Disconnected"<<endl;

	socket.Close();

	cout<<"----------------"<<endl;

	return AskList();
}

int DistanceBetweenCID(int argc, char *argv[])
{
	if( argc != 3 )
	{
			cerr<<"Error : distance : the number of parameter is uncorrect"<<endl;
			Help();
			return 1;
	}

	TCID cid1=atoi(argv[1]);

	if( cid1 < TCID_GUEST_MIN )
	{
			cerr<<"Error : distance : the CID 1 must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	TCID cid2=atoi(argv[2]);

	if( cid2 < TCID_GUEST_MIN )
	{
			cerr<<"Error : distance : the CID2 must be greater than or equal to "<<TCID_GUEST_MIN<<endl;
			return 1;
	}

	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

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

int SetScale(int argc, char *argv[])
{
	if( argc != 2)
	{
			cerr<<"Error : scale : the number of parameter is uncorrect"<<endl;
			Help();
			return 1;
	}

	TScale scale=atof(argv[1]);

	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

	if( ! socket.ConnectLoop() )
	{
		cerr<<"Error : scale : socket.Connect error"<<endl;
		return 1;
	}

	int err;

	TOrder order=TORDER_SET_SCALE;
	err=socket.Send((char*)&order,sizeof(order));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : scale : socket.Send : order"<<endl;
		return 1;
	}
	err=socket.Send((char*)&scale,sizeof(scale));
	if( err == SOCKET_ERROR )
	{
		cerr<<"Error : scale : socket.Send : scale"<<endl;
		return 1;
	}

	socket.Close();

	return 0;
}

int CloseAllClient()
{
	CSocketClientITCP socket;

	socket.Init(IP_Ctrl.c_str(),Port_Ctrl);

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
	char** param_cmd = new char*[argc];
	int nbr_param_cmd=0;

	NameOfProg=argv[0];

	int arg_idx = 1;
	while (arg_idx < argc)
	{
		if( ! strcmp("-v", argv[arg_idx]) || ! strcmp("--version", argv[arg_idx]) )
		{
			std::cout<<"Version : "<<VERSION<<std::endl;
			return 0;
		}
		if( ! strcmp("-h", argv[arg_idx]) || ! strcmp("--help", argv[arg_idx]) )
		{
			Help();
			return 1;
		}
		if( ( ! strcmp("-p", argv[arg_idx]) || ! strcmp("--port", argv[arg_idx]) ) && (arg_idx + 1) < argc)
		{
			Port_Ctrl = std::stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ( ! strcmp("-i", argv[arg_idx]) || ! strcmp("--ip", argv[arg_idx]) ) && (arg_idx + 1) < argc)
		{
			IP_Ctrl = std::string(argv[arg_idx+1]);
			arg_idx++;
		}
		else
		{
			param_cmd[nbr_param_cmd++]=argv[arg_idx];
		}

		arg_idx++;
	}

	if( nbr_param_cmd == 0 )
	{
		Help();
		return 0;
	}

	if( ! strcmp(param_cmd[0],"ls") )
		return AskList();

	if( ! strcmp(param_cmd[0],"set") )
		return ChangeCoordinate(nbr_param_cmd, param_cmd);

	if( ! strcmp(param_cmd[0],"loss") )
		return ChangePacketLoss(nbr_param_cmd, param_cmd);

	if( ! strcmp(param_cmd[0],"show") )
		return AskShow();

	if( ! strcmp(param_cmd[0],"status") )
		return AskStatus();

	if( ! strcmp(param_cmd[0],"distance") )
		return DistanceBetweenCID(nbr_param_cmd, param_cmd);

	if( ! strcmp(param_cmd[0],"scale") )
		return SetScale(nbr_param_cmd, param_cmd);

	if( ! strcmp(param_cmd[0],"close") )
		return CloseAllClient();

	cerr<<NameOfProg<<" : Error : unknown order : "<<param_cmd[1]<<endl;

	return 1;
}

