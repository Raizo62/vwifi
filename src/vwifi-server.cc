#include <iostream> // cout

#include <string.h> // strcmp

#include "config.h"
#include "cwifiserver.h"
#include "cctrlserver.h"
#include "cselect.h"
#include "cdynbuffer.h"

using namespace std;

CDynBuffer Buffer; // Buffer to stock received values

void RemoveClient(CWifiServer* srv, bool srvIsSpy, TIndex i, TDescriptor socket, CSelect* scheduler)
{
	if( ! srvIsSpy )
	{
		cout<<"Client disconnected : "; srv->ShowInfoWifi(i) ; cout<<endl;
	}
	else
		cout<<"Spy disconnected"<<endl;

	srv->CloseClient(i);

	//del master socket to set
	scheduler->DelNode(socket);
}

void ForwardData(bool srcIsSpy, CWifiServer* src, CWifiServer* otherDst, CSelect* scheduler)
{
	TDescriptor socket;

	int valread;
	TPower power;

	for ( TIndex i = 0 ; i < src->GetNumberClient() ; )
	{
		socket = (*src)[i];

		if( ! src->IsEnable(i) )
		{
			RemoveClient(src, srcIsSpy , i, socket, scheduler);

			continue;
		}

		if( scheduler->DescriptorHasAction(socket) )
		{
			//Check if it was for closing , and also read the
			//incoming message

			// read the power
			valread = src->Read( socket , (char*)&power, sizeof(power));
			if ( valread <= 0 )
			{
				RemoveClient(src, srcIsSpy , i, socket, scheduler);

				continue;
			}

			// read the data
			valread = src->ReadBigData( socket , &Buffer);
			if ( valread <= 0 )
			{
				RemoveClient(src, srcIsSpy , i, socket, scheduler);

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

	CSelect scheduler;

	CListInfo<CInfoSocket> InfoSockets;
	CListInfo<CInfoWifi> InfoWifis;
	CListInfo<CInfoWifi> InfoWifisDeconnected;

	CWifiServer wifiGuestVHostServer(AF_VSOCK,&InfoSockets,&InfoWifis,&InfoWifisDeconnected);
	cout<<"CLIENT VHOST : ";
	wifiGuestVHostServer.Init(WIFI_CLIENT_PORT_VHOST);
	if( ! wifiGuestVHostServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestVHostServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	CWifiServer* wifiServer=&wifiGuestVHostServer; // or wifiGuestINETServer, it doesn't change anything

	CWifiServer wifiGuestINETServer(AF_INET,&InfoSockets,&InfoWifis,&InfoWifisDeconnected);
	cout<<"CLIENT TCP : ";
	wifiGuestINETServer.Init(WIFI_CLIENT_PORT_INET);
	if( ! wifiGuestINETServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestINETServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"SPY : ";
	CWifiServer wifiSpyServer(AF_INET);
	wifiSpyServer.SetPacketLoss(false);
	wifiSpyServer.Init(WIFI_SPY_PORT);
	if( ! wifiSpyServer.Listen(1) )
	{
		cerr<<"Error : wifiSpyServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"CTRL : ";
	CCTRLServer ctrlServer(&wifiGuestVHostServer, &wifiGuestINETServer, &wifiSpyServer,&scheduler);
	ctrlServer.Init(CTRL_PORT);
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
	scheduler.AddNode(wifiGuestVHostServer);
	scheduler.AddNode(wifiGuestINETServer);
	scheduler.AddNode(wifiSpyServer);
	scheduler.AddNode(ctrlServer);

	while( true )
	{
		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		if( scheduler.Wait() == SCHEDULER_ERROR )
		{
			cerr<<"Error : scheduler.Wait"<<endl;
			return 1;
		}
		else {

			//If something happened on the master socket ,
			//then its an incoming connection
			if( scheduler.DescriptorHasAction(wifiGuestVHostServer) )
			{
				socket = wifiGuestVHostServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestVHostServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client VHost : "; wifiServer->ShowInfoWifi(wifiServer->GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.DescriptorHasAction(wifiGuestINETServer) )
			{
				socket = wifiGuestINETServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestINETServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Client TCP : "; wifiServer->ShowInfoWifi(wifiServer->GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.DescriptorHasAction(wifiSpyServer) )
			{
				socket = wifiSpyServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiSpyServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Spy"<<endl;
			}

			if( scheduler.DescriptorHasAction(ctrlServer) )
			{
				ctrlServer.ReceiveOrder();
			}

			//else its some IO operation on some other socket

			ForwardData(false, wifiServer, &wifiSpyServer, &scheduler);
			ForwardData(true, &wifiSpyServer, wifiServer, &scheduler);
		}
	}

	return 0;
}

int main(int argc, char** argv)
{
	if( argc > 1 )
	{
		if( ! strcmp("-v", argv[1]) || ! strcmp("--version", argv[1]) )
		{
			cout<<"Version : "<<VERSION<<endl;
			return 0;
		}

		cerr<<"Error : unknown parameter"<<endl;
		return 1;
	}

	return vwifi_server();
}

