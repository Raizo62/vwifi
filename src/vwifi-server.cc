#include <string> //string
#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "cwifiserver.h"
#include "cctrlserver.h"
#include "cscheduler.h"

using namespace std;

void ForwardData(CWifiServer* serverMaster, CWifiServer* serverSecond, CWifiServer* host, CScheduler* scheduler)
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

				//Echo back the message that came in

				//set the string terminating NULL byte on the end
				//of the data read
				//buffer[valread] = '\0';
				//wifiGuestVHostServer.Send(socket,buffer , strlen(buffer));
				// send to all other clients
				if( serverMaster->GetNumberClient() > 1 )
				{
#ifdef _DEBUG
					cout<<"Forward Main "<<serverMaster->GetType() <<" : "<<valread<<" bytes from "; serverMaster->ShowInfoWifi(i); cout<<" to "<< serverMaster->GetNumberClient()-1 << " others clients" <<endl;
#endif
					serverMaster->SendAllOtherClients(i,power,buffer,valread);
				}
				if( serverSecond->GetNumberClient() > 0 )
				{
#ifdef _DEBUG
					cout<<"Forward Second "<<serverMaster->GetType() <<" : "<<valread<<" bytes from "; serverSecond->ShowInfoWifi(i); cout<<" to "<< serverSecond->GetNumberClient() << " others clients" <<endl;
#endif
					CCoordinate coo=*(serverMaster->GetReferenceOnInfoWifiByIndex(i));
					serverSecond->SendAllClients(coo,power,buffer,valread);
				}
#ifdef _DEBUG
				cout<<"Forward "<<valread<<" bytes from "; wifiGuestVHostServer.ShowInfoWifi(i); cout<<" to Host"<<endl;
#endif
				host->SendAllClientsWithoutLoss(power,buffer,valread);

			}
		}

		i++;
	}
}

int main(int argc , char *argv[])
{
	TDescriptor socket;

	TIndex i;

	int valread;
	TPower power;
	char buffer[1024]; //data buffer

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
			ForwardData(&wifiGuestVHostServer, &wifiGuestINETServer, &wifiHostServer, &scheduler);
			ForwardData(&wifiGuestINETServer, &wifiGuestVHostServer, &wifiHostServer, &scheduler);

						//else its some IO operation on some other socket
			for ( i = 0 ; i < wifiHostServer.GetNumberClient() ; )
			{
				socket = wifiHostServer[i];

				if( ! wifiHostServer.IsEnable(i) )
				{
							//Somebody disconnected

							//Close the socket
							cout<<"Host disconnected"<<endl;
							wifiHostServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
				}

				if( scheduler.DescriptorHasAction(socket) )
				{
					//Check if it was for closing , and also read the
					//incoming message

					// read the power
					valread = wifiHostServer.Read( socket , (char*)&power, sizeof(power));
					if ( valread >= 0 )
					{
						if ( valread == 0 )
						{
							//Close the socket
							cout<<"Host disconnected"<<endl;
							wifiHostServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
						}
					}

					// read the data
					valread = wifiHostServer.ReadBigData( socket , buffer, sizeof(buffer));
					if ( valread >= 0 )
					{
						if ( valread == 0 )
						{
							//Close the socket
							cout<<"Host disconnected"<<endl;
							wifiHostServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
						}
					}

					if( wifiGuestVHostServer.GetNumberClient() > 0 )
					{
#ifdef _DEBUG
						cout<<"Forward "<<valread<<" bytes from Host to Guests"<<endl;
#endif
						wifiGuestVHostServer.SendAllClientsWithoutLoss(power,buffer,valread);
					}
					if( wifiGuestINETServer.GetNumberClient() > 0 )
					{
#ifdef _DEBUG
						cout<<"Forward "<<valread<<" bytes from Host to Guests"<<endl;
#endif
						wifiGuestINETServer.SendAllClientsWithoutLoss(power,buffer,valread);
					}
				}

				i++;
			}
		}
	}

	return 0;
}

