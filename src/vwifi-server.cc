#include <iostream> // cout

#include <string.h> // strcmp

#include "config.h"
#include "cwifiserver.h"
#include "cctrlserver.h"
#include "cselect.h"
#include "cdynbuffer.h"

using namespace std;

CDynBuffer Buffer; // Buffer to stock received values

TPort Port_VHOST = DEFAULT_WIFI_CLIENT_PORT_VHOST;
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
		cout<<"Spy disconnected"<<endl;

	srv->CloseClient(i);

	//del master socket to set
	Scheduler.DelNode(socket);
}

void ForwardData(bool srcIsSpy, CWifiServer* src, CWifiServer* otherDst)
{
	TDescriptor socket;

	int valread;
	TPower power;

	for ( TIndex i = 0 ; i < src->GetNumberClient() ; )
	{
		socket = (*src)[i];

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

	CWifiServer wifiGuestVHostServer(AF_VSOCK,&infoSockets,&infoWifis,&infoWifisDeconnected);
	cout<<"CLIENT VHOST : ";
	wifiGuestVHostServer.Init(Port_VHOST);
	if( ! wifiGuestVHostServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestVHostServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	CWifiServer* wifiServer=&wifiGuestVHostServer; // or wifiGuestINETServer, it doesn't change anything

	CWifiServer wifiGuestINETServer(AF_INET,&infoSockets,&infoWifis,&infoWifisDeconnected);
	cout<<"CLIENT TCP : ";
	wifiGuestINETServer.Init(Port_TCP);
	if( ! wifiGuestINETServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestINETServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"SPY : ";
	CWifiServer wifiSpyServer(AF_INET);
	wifiSpyServer.SetPacketLoss(false);
	wifiSpyServer.Init(Port_Spy);
	if( ! wifiSpyServer.Listen(1) )
	{
		cerr<<"Error : wifiSpyServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"CTRL : ";
	CCTRLServer ctrlServer(&wifiGuestVHostServer, &wifiGuestINETServer, &wifiSpyServer,&Scheduler);
	ctrlServer.Init(Port_Ctrl);
	if( ! ctrlServer.Listen() )
	{
		cerr<<"Error : ctrlServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"Size of disconnected : "<<WIFI_MAX_DECONNECTED_CLIENT<<endl;

	if( wifiGuestVHostServer.CanLostPackets() )
		cout<<"Packet loss : Enable"<<endl;
	else
		cout<<"Packet loss : disable"<<endl;

	//add master socket to set
	Scheduler.AddNode(wifiGuestVHostServer);
	Scheduler.AddNode(wifiGuestINETServer);
	Scheduler.AddNode(wifiSpyServer);
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
			if( Scheduler.DescriptorHasAction(wifiGuestVHostServer) )
			{
				socket = wifiGuestVHostServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestVHostServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				Scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client VHost : "; wifiServer->ShowInfoWifi(wifiServer->GetNumberClient()-1) ; cout<<endl;
			}

			if( Scheduler.DescriptorHasAction(wifiGuestINETServer) )
			{
				socket = wifiGuestINETServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestINETServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				Scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client TCP : "; wifiServer->ShowInfoWifi(wifiServer->GetNumberClient()-1) ; cout<<endl;
			}

			if( Scheduler.DescriptorHasAction(wifiSpyServer) )
			{
				socket = wifiSpyServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiSpyServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				Scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Spy"<<endl;
			}

			if( Scheduler.DescriptorHasAction(ctrlServer) )
			{
				ctrlServer.ReceiveOrder();
			}

			//else its some IO operation on some other socket

			ForwardData(false, wifiServer, &wifiSpyServer);
			ForwardData(true, &wifiSpyServer, wifiServer);
		}
	}

	return 0;
}

void help()
{
	cout<<"Usage: vwifi-server [-h] [-v] [-p PORT_VHOST] [-t PORT_TCP] [-s PORT_SPY] [-c PORT_CTRL]"<<endl;
	cout<<"                    [--help] [--version] [--port-vhost PORT_VHOST] [--port-tcp PORT_TCP] [--port-spy PORT_SPY] [--port-ctrl PORT_CTRL]"<<endl;
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
		if( ( ! strcmp("-p", argv[arg_idx]) || ! strcmp("--port-vhost", argv[arg_idx]) ) && (arg_idx + 1) < argc)
		{
			Port_VHOST = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ( ! strcmp("-t", argv[arg_idx]) || ! strcmp("--port-tcp", argv[arg_idx]) ) && (arg_idx + 1) < argc)
		{
			Port_TCP = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ( ! strcmp("-s", argv[arg_idx]) || ! strcmp("--port-spy", argv[arg_idx]) ) && (arg_idx + 1) < argc)
		{
			Port_Spy = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else if( ( ! strcmp("-c", argv[arg_idx]) || ! strcmp("--port-ctrl", argv[arg_idx]) ) && (arg_idx + 1) < argc)
		{
			Port_Ctrl = stoi(argv[arg_idx+1]);
			arg_idx++;
		}
		else
		{
			cerr<<"Error : unknown parameter : "<< argv[arg_idx] <<endl;
			help();
			return 1;
		}

		arg_idx++;
	}

	return vwifi_server();
}

