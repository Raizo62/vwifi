#include <string> //string
#include <iostream> // cout

#include <string.h> //strlen

#include "config.h"
#include "cwifiserver.h"
#include "cctrlserver.h"
#include "cscheduler.h"

using namespace std;

int main(int argc , char *argv[])
{
	TDescriptor socket;

	TIndex i;

	int valread;
	TPower power;
	char buffer[1024]; //data buffer

	CScheduler scheduler;

	CWifiServer wifiGuestServer;
	cout<<"GUEST : ";
	wifiGuestServer.Init(WIFI_GUEST_PORT);
	if( ! wifiGuestServer.Listen(WIFI_MAX_DECONNECTED_CLIENT) )
	{
		cerr<<"Error : wifiGuestServer.Listen"<<endl;
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
	CCTRLServer ctrlServer(&wifiGuestServer,&wifiHostServer,&scheduler);
	ctrlServer.Init(CTRL_PORT);
	if( ! ctrlServer.Listen() )
	{
		cerr<<"Error : ctrlServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"Size of disconnected : "<<WIFI_MAX_DECONNECTED_CLIENT<<endl;

	if( wifiGuestServer.CanLostPackets() )
		cout<<"Packet loss : Enable"<<endl;
	else
		cout<<"Packet loss : disable"<<endl;

	//add master socket to set
	scheduler.AddNode(wifiGuestServer);
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
			if( scheduler.DescriptorHasAction(wifiGuestServer) )
			{
				socket = wifiGuestServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiGuestServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from Guest : "; wifiGuestServer.ShowInfoWifi(wifiGuestServer.GetNumberClient()-1) ; cout<<endl;
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
			for ( i = 0 ; i < wifiGuestServer.GetNumberClient() ; )
			{
				socket = wifiGuestServer[i];

				if( ! wifiGuestServer.IsEnable(i) )
				{
							//Somebody disconnected

							//Close the socket
							cout<<"Guest disconnected : "; wifiGuestServer.ShowInfoWifi(i) ; cout<<endl;
							wifiGuestServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
				}

				if( scheduler.DescriptorHasAction(socket) )
				{
					//Check if it was for closing , and also read the
					//incoming message

					// read the power
					valread = wifiGuestServer.Read( socket , (char*)&power, sizeof(power));
					if ( valread >= 0 )
					{
						if ( valread == 0 )
						{
							//Close the socket
							cout<<"Guest disconnected : "; wifiGuestServer.ShowInfoWifi(i) ; cout<<endl;
							wifiGuestServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
						}
					}

					// read the data
					valread = wifiGuestServer.Read( socket , buffer, sizeof(buffer));
					if ( valread >= 0 )
					{
						if ( valread == 0 )
						{
							//Close the socket
							cout<<"Guest disconnected : "; wifiGuestServer.ShowInfoWifi(i) ; cout<<endl;
							wifiGuestServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
						}

						//Echo back the message that came in

						//set the string terminating NULL byte on the end
						//of the data read
						//buffer[valread] = '\0';
						//wifiGuestServer.Send(socket,buffer , strlen(buffer));
						// send to all other clients
						if( wifiGuestServer.GetNumberClient() > 1 )
						{
#ifdef _DEBUG
							cout<<"Forward "<<valread<<" bytes from "; wifiGuestServer.ShowInfoWifi(i); cout<<" to "<< wifiGuestServer.GetNumberClient()-1 << " others clients" <<endl;
#endif
							wifiGuestServer.SendAllOtherClients(i,power,buffer,valread);
						}
#ifdef _DEBUG
						cout<<"Forward "<<valread<<" bytes from "; wifiGuestServer.ShowInfoWifi(i); cout<<" to Host"<<endl;
#endif
						wifiHostServer.SendAllClientsWithoutLoss(power,buffer,valread);

					}
				}

				i++;
			}

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
					valread = wifiHostServer.Read( socket , buffer, sizeof(buffer));
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
				}

				i++;
			}
		}
	}

	return 0;
}

