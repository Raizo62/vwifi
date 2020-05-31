#include <string> //string
#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "cwifiserver.h"
#include "cctrlserver.h"
#include "cscheduler.h"

using namespace std;

void ForwardData(CWifiServer* serverMaster, bool masterSendToOwnClients, CWifiServer* serverSecond, bool serverSecondForwardWithoutLoss, CWifiServer* serverThird, bool serverThirdIsWithoutCoordinate, CScheduler* scheduler)
{
	TDescriptor socket;

	int valread;
	TPower power;
	char buffer[1024]; //data buffer

	for ( TIndex i = 0 ; i < serverMaster->GetNumberClient() ; )
	{
		socket = (*serverMaster)[i];

		if( ! serverMaster->IsEnable(i) )
		{
					//Somebody disconnected

					//Close the socket
					cout<<"Guest disconnected : "; serverMaster->ShowInfoWifi(i) ; cout<<endl;
					serverMaster->CloseClient(i);

					//del master socket to set
					scheduler->DelNode(socket);

					continue;
		}

		if( scheduler->DescriptorHasAction(socket) )
		{
			//Check if it was for closing , and also read the
			//incoming message

			// read the power
			valread = serverMaster->Read( socket , (char*)&power, sizeof(power));
			if ( valread >= 0 )
			{
				if ( valread == 0 )
				{
					//Close the socket
					cout<<"Guest disconnected : "; serverMaster->ShowInfoWifi(i) ; cout<<endl;
					serverMaster->CloseClient(i);

					//del master socket to set
					scheduler->DelNode(socket);

					continue;
				}
			}

			// read the data
			valread = serverMaster->ReadBigData( socket , buffer, sizeof(buffer));
			if ( valread >= 0 )
			{
				if ( valread == 0 )
				{
					//Close the socket
					cout<<"Guest disconnected : "; serverMaster->ShowInfoWifi(i) ; cout<<endl;
					serverMaster->CloseClient(i);

					//del master socket to set
					scheduler->DelNode(socket);

					continue;
				}

				if( masterSendToOwnClients )
					if( serverMaster->GetNumberClient() > 1 )
					{
#ifdef _VERBOSE2
						cout<<"Server "<<serverMaster->GetPort()<<" forward "<<valread<<" bytes from own client "; serverMaster->ShowInfoWifi(i); cout<<" to "<< serverMaster->GetNumberClient()-1 << " others owns clients" <<endl;
#endif
						serverMaster->SendAllOtherClients(i,power,buffer,valread);
					}

				if( serverSecond->GetNumberClient() > 0 )
				{
					if( serverSecondForwardWithoutLoss )
					{
#ifdef _VERBOSE2
					cout<<"Server "<<serverSecond->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of "<< serverMaster->GetPort() << " to all "<< serverSecond->GetNumberClient() << " clients without loss" <<endl;
#endif
						serverSecond->SendAllClientsWithoutLoss(power,buffer,valread);
					}
					else
					{
#ifdef _VERBOSE2
						cout<<"Server "<<serverSecond->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of "<< serverMaster->GetPort() << " to all "<< serverSecond->GetNumberClient() << " clients" <<endl;
#endif
						CCoordinate coo=*(serverMaster->GetReferenceOnInfoWifiByIndex(i));
						serverSecond->SendAllClients(coo,power,buffer,valread);
					}
				}

				if( serverThird->GetNumberClient() > 0 )
				{
					if( serverThirdIsWithoutCoordinate )
					{
#ifdef _VERBOSE2
						cout<<"Server "<<serverThird->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of "<< serverMaster->GetPort() << " to all "<< serverThird->GetNumberClient() << " clients without loss" <<endl;
#endif
						serverThird->SendAllClientsWithoutLoss(power,buffer,valread);
					}
					else
					{
#ifdef _VERBOSE2
						cout<<"Server "<<serverThird->GetPort()<<" forward "<<valread<<" bytes from client "; serverMaster->ShowInfoWifi(i); cout<<" of of "<< serverMaster->GetPort() << " to all "<< serverSecond->GetNumberClient() << " clients" <<endl;
#endif
						CCoordinate coo=*(serverMaster->GetReferenceOnInfoWifiByIndex(i));
						serverThird->SendAllClients(coo,power,buffer,valread);
					}
				}
			}
		}

		i++;
	}
}

int main(int argc , char *argv[])
{
	TDescriptor socket;

	CScheduler scheduler;

	CWifiServer wifiGuestVHostServer(AF_VSOCK);
	cout<<"GUEST VHOST : ";
	wifiGuestVHostServer.Init(WIFI_GUEST_PORT_VHOST);
	if( ! wifiGuestVHostServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestVHostServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	CWifiServer wifiGuestINETServer(AF_INET);
	cout<<"GUEST TCP : ";
	wifiGuestINETServer.Init(WIFI_GUEST_PORT_INET);
	if( ! wifiGuestINETServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestINETServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"HOST : ";
	CWifiServer wifiHostServer(AF_INET);
	wifiHostServer.SetPacketLoss(false);
	wifiHostServer.Init(WIFI_HOST_PORT);
	if( ! wifiHostServer.Listen(1) )
	{
		cerr<<"Error : wifiHostServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"CTRL : ";
	CCTRLServer ctrlServer(&wifiGuestVHostServer, &wifiGuestINETServer, &wifiHostServer,&scheduler);
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
	scheduler.AddNode(wifiHostServer);
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
				cout<<"New connection from Guest VHost : "; wifiGuestVHostServer.ShowInfoWifi(wifiGuestVHostServer.GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.DescriptorHasAction(wifiGuestINETServer) )
			{
				socket = wifiGuestINETServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestVHostServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Guest TCP : "; wifiGuestINETServer.ShowInfoWifi(wifiGuestINETServer.GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.DescriptorHasAction(wifiHostServer) )
			{
				socket = wifiHostServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiHostServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Host"<<endl;
			}

			if( scheduler.DescriptorHasAction(ctrlServer) )
			{
				ctrlServer.ReceiveOrder();
			}

			//else its some IO operation on some other socket

			ForwardData(&wifiGuestVHostServer, true, &wifiGuestINETServer, false, &wifiHostServer, true, &scheduler);
			ForwardData(&wifiGuestINETServer, true, &wifiGuestVHostServer, false, &wifiHostServer, true, &scheduler);

			ForwardData(&wifiHostServer, false, &wifiGuestVHostServer, true, &wifiGuestINETServer, false, &scheduler);

		}
	}

	return 0;
}

