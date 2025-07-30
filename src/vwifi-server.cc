#include <iostream> // cout

#include <string.h> // strcmp

#include "config.h"
#include "tools.h" // isPositiveInt
#include "cwifiserveritcp.h"
#ifdef ENABLE_VHOST
	#include "cwifiservervtcp.h"
#endif
#include "cctrlserver.h"
#include "cselect.h"
#include "cdynbuffer.h"

using namespace std;

CDynBuffer Buffer; // Buffer to stock received values
#ifdef ENABLE_VHOST
TPort Port_VHOST = DEFAULT_WIFI_CLIENT_PORT_VHOST;
#endif
TPort Port_TCP = DEFAULT_WIFI_CLIENT_PORT_INET;
TPort Port_Spy = DEFAULT_WIFI_SPY_PORT;
TPort Port_Ctrl = DEFAULT_CTRL_PORT;

CSelect Scheduler;

void RemoveClient(CWifiServer* srv, bool srvIsSpy, TIndex i, TDescriptor socket)
{
	if( ! srvIsSpy )
	{
		cout<<"Client disconnected : "; srv->ShowInfoWifi(i) ; cout<<endl;
	}
	else
		cout<<"Spy disconnected : "<< srv->GetReferenceOnInfoWifiByIndex(i)->GetCid() <<endl;

	srv->CloseClient(i);

	//del master socket to set
	Scheduler.DelNode(socket);
}

void ForwardData(bool srcIsSpy, CWifiServer* src, CWifiServer* otherDst)
{
	int valread;
	TPower power;

	for ( TIndex i = 0 ; i < src->GetNumberClient() ; )
	{
		TDescriptor socket = (*src)[i];

		if( ! src->IsEnable(i) )
		{
			RemoveClient(src, srcIsSpy , i, socket);

			continue;
		}

		if( Scheduler.DescriptorHasAction(socket) )
		{
			//Check if it was for closing , and also read the
			//incoming message

			valread=src->RecvSignal(socket,&power,&Buffer);
			if( valread <=0 )
			{
				RemoveClient(src, srcIsSpy , i, socket);

				continue;
			}

			if( ! srcIsSpy )
			{
				src->SendAllOtherClients(i,power,Buffer.GetBuffer(),valread);
				otherDst->SendAllClientsWithoutLoss(power,Buffer.GetBuffer(),valread);
			}
			else
			{
				src->SendAllOtherClientsWithoutLoss(i,power,Buffer.GetBuffer(),valread);
				otherDst->SendAllClientsWithoutLoss(power,Buffer.GetBuffer(),valread);
			}
		}

		i++;
	}
}

int vwifi_server()
{
	TDescriptor socket;

	CListInfo<CInfoSocket> infoSockets;
	CListInfo<CInfoWifi> infoWifis;
	CListInfo<CInfoWifi> infoWifisDeconnected;

#ifdef ENABLE_VHOST
	CWifiServerVTCP wifiServerVTCP(&infoSockets,&infoWifis,&infoWifisDeconnected);
	cout<<"CLIENT VHOST : ";
	wifiServerVTCP.Init(Port_VHOST);
	if( ! wifiServerVTCP.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiServerVTCP.Listen"<<endl;
		exit(EXIT_FAILURE);
	}
#endif

	CWifiServerITCP wifiServerITCP(&infoSockets,&infoWifis,&infoWifisDeconnected);
	cout<<"CLIENT TCP : ";
	wifiServerITCP.Init(Port_TCP);
	if( ! wifiServerITCP.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiServerITCP.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	CWifiServer* wifiServer=&wifiServerITCP; // or wifiServerVTCP, if exist, it doesn't change anything

	cout<<"SPY : ";
	CWifiServerITCP wifiServerSPY;
	wifiServerSPY.Init(Port_Spy);
	if( ! wifiServerSPY.Listen(1) )
	{
		cerr<<"Error : wifiServerSPY.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"CTRL : ";
#ifdef ENABLE_VHOST
	CCTRLServer ctrlServer(&wifiServerVTCP, &wifiServerITCP, &wifiServerSPY,&Scheduler);
#else
	CCTRLServer ctrlServer(NULL, &wifiServerITCP, &wifiServerSPY,&Scheduler);
#endif
	ctrlServer.Init(Port_Ctrl);
	if( ! ctrlServer.Listen() )
	{
		cerr<<"Error : ctrlServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"Size of disconnected : "<<WIFI_MAX_DECONNECTED_CLIENT<<endl;

	if( CanLostPackets )
		cout<<"Packet loss : Enable"<<endl;
	else
		cout<<"Packet loss : disable"<<endl;

	cout<<"Scale : "<<Scale<<endl;

	//add master socket to set
#ifdef ENABLE_VHOST
	Scheduler.AddNode(wifiServerVTCP);
#endif
	Scheduler.AddNode(wifiServerITCP);
	Scheduler.AddNode(wifiServerSPY);
	Scheduler.AddNode(ctrlServer);

	while( true )
	{
		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		if( Scheduler.Wait() == SCHEDULER_ERROR )
		{
			cerr<<"Error : scheduler.Wait"<<endl;
			return 1;
		}
		else {

			//If something happened on the master socket ,
			//then its an incoming connection
#ifdef ENABLE_VHOST
			if( Scheduler.DescriptorHasAction(wifiServerVTCP) )
			{
				socket = wifiServerVTCP.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiServerVTCP.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				Scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client VHost : "; wifiServer->ShowInfoWifi(wifiServer->GetNumberClient()-1) ; cout<<endl;
			}
#endif

			if( Scheduler.DescriptorHasAction(wifiServerITCP) )
			{
				socket = wifiServerITCP.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiServerITCP.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				Scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client TCP : "; wifiServer->ShowInfoWifi(wifiServer->GetNumberClient()-1) ; cout<<endl;
			}

			if( Scheduler.DescriptorHasAction(wifiServerSPY) )
			{
				socket = wifiServerSPY.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiSpyServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				Scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Spy : "<<wifiServerSPY.GetReferenceOnInfoWifiByIndex(wifiServerSPY.GetNumberClient()-1)->GetCid()<<endl;
			}

			if( Scheduler.DescriptorHasAction(ctrlServer) )
			{
				ctrlServer.ReceiveOrder();
			}

			//else its some IO operation on some other socket

			ForwardData(false, wifiServer, &wifiServerSPY);
			ForwardData(true, &wifiServerSPY, wifiServer);
		}
	}

	return 0;
}

void help()
{
#ifdef ENABLE_VHOST
	cout<<"Usage: vwifi-server [-h] [-v] [-l] [-u] [-p PORT_VHOST] [-t PORT_TCP] [-s PORT_SPY] [-c PORT_CTRL]"<<endl;
	cout<<"                    [--help] [--version] [--lost-packets] [--use-port-in-hash] [--port-vhost PORT_VHOST] [--port-tcp PORT_TCP] [--port-spy PORT_SPY] [--port-ctrl PORT_CTRL]"<<endl;
	cout<<"         By default : PORT_VHOST="<< DEFAULT_WIFI_CLIENT_PORT_VHOST <<
								" PORT_TCP="  << DEFAULT_WIFI_CLIENT_PORT_INET <<
								" PORT_SPY="  << DEFAULT_WIFI_SPY_PORT <<
								" PORT_CTRL=" << DEFAULT_CTRL_PORT <<endl;
#else
	cout<<"Usage: vwifi-server [-h] [-v] [-l] [-u] [-t PORT_TCP] [-s PORT_SPY] [-c PORT_CTRL]"<<endl;
	cout<<"                    [--help] [--version] [--lost-packets] [--use-port-in-hash] [--port-tcp PORT_TCP] [--port-spy PORT_SPY] [--port-ctrl PORT_CTRL]"<<endl;
	cout<<"         By default : PORT_TCP="  << DEFAULT_WIFI_CLIENT_PORT_INET <<
								" PORT_SPY="  << DEFAULT_WIFI_SPY_PORT <<
								" PORT_CTRL=" << DEFAULT_CTRL_PORT <<endl;
#endif
}


int main(int argc, char** argv)
{
	int arg_idx = 1;
	while (arg_idx < argc)
	{
		if( ! strcmp("-v", argv[arg_idx]) || ! strcmp("--version", argv[arg_idx]) )
		{
			cout<<"Version : "<<VERSION<<endl;
			return 0;
		}
		if( ! strcmp("-h", argv[arg_idx]) || ! strcmp("--help", argv[arg_idx]) )
		{
			help();
			return 0;
		}
		if( ! strcmp("-l", argv[arg_idx]) || ! strcmp("--lost-packets", argv[arg_idx]) )
		{
			CanLostPackets=true;
		}
		else if( ! strcmp("-u", argv[arg_idx]) || ! strcmp("--use-port-in-hash", argv[arg_idx]) )
		{
			HashUsesPort=true;
		}
#ifdef ENABLE_VHOST
		else if( ( ! strcmp("-p", argv[arg_idx]) || ! strcmp("--port-vhost", argv[arg_idx]) ) && (arg_idx + 1) < argc && isPositiveInt(argv[arg_idx+1]) )
		{
			Port_VHOST = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
#endif
		else if( ( ! strcmp("-t", argv[arg_idx]) || ! strcmp("--port-tcp", argv[arg_idx]) ) && (arg_idx + 1) < argc && isPositiveInt(argv[arg_idx+1]) )
		{
			Port_TCP = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ( ! strcmp("-s", argv[arg_idx]) || ! strcmp("--port-spy", argv[arg_idx]) ) && (arg_idx + 1) < argc && isPositiveInt(argv[arg_idx+1]) )
		{
			Port_Spy = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ( ! strcmp("-c", argv[arg_idx]) || ! strcmp("--port-ctrl", argv[arg_idx]) ) && (arg_idx + 1) < argc && isPositiveInt(argv[arg_idx+1]) )
		{
			Port_Ctrl = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else
		{
			cerr<<"Error : problem with this parameter : "<< argv[arg_idx] <<endl;
			help();
			return 1;
		}

		arg_idx++;
	}

	return vwifi_server();
}
