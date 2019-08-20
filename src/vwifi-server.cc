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

	CWifiServer wifiServer;
	wifiServer.Init(WIFI_PORT);
	if( ! wifiServer.Listen(MAX_DECONNECTED_CLIENT_WIFI) )
	{
		cerr<<"Error : wifiServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"CTRL : ";
	CCTRLServer ctrlServer(&wifiServer,&scheduler);
	ctrlServer.Init(CTRL_PORT);
	if( ! ctrlServer.Listen() )
	{
		cerr<<"Error : ctrlServer.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	cout<<"Size of disconnected : "<<MAX_DECONNECTED_CLIENT_WIFI<<endl;

#ifdef CAN_LOST_PACKET
	cout<<"Packet loss : Enable"<<endl;
#else
	cout<<"Packet loss : disable"<<endl;
#endif

	//add master socket to set
	scheduler.AddNode(wifiServer);
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
			if( scheduler.NodeHasAction(wifiServer) )
			{
				socket = wifiServer.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : wifiServer.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from : "; wifiServer.ShowInfoWifi(wifiServer.GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.NodeHasAction(ctrlServer) )
			{
				ctrlServer.ReceiveOrder();
			}

			//else its some IO operation on some other socket
			for ( i = 0 ; i < wifiServer.GetNumberClient() ; )
			{
				socket = wifiServer[i];

				if( ! wifiServer.IsEnable(i) )
				{
							//Somebody disconnected , get his details and print
							cout<<"Host disable : "; wifiServer.ShowInfoWifi(i) ; cout<<endl;

							//Close the socket
							wifiServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
				}

				if( scheduler.NodeHasAction(socket) )
				{
					//Check if it was for closing , and also read the
					//incoming message

					// read the power
					valread = wifiServer.Read( socket , (char*)&power, sizeof(power));
					if ( valread >= 0 )
					{
						if ( valread == 0 )
						{
							//Close the socket
							wifiServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
						}
					}

					// read the data
					valread = wifiServer.Read( socket , buffer, sizeof(buffer));
					if ( valread >= 0 )
					{
						if ( valread == 0 )
						{
							//Close the socket
							wifiServer.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);

							continue;
						}

						//Echo back the message that came in

						//set the string terminating NULL byte on the end
						//of the data read
						//buffer[valread] = '\0';
						//wifiServer.Send(socket,buffer , strlen(buffer));
						// send to all other clients
						if( wifiServer.GetNumberClient() > 1 )
						{
#ifdef _DEBUG
							cout<<"Forward "<<valread<<" bytes from "; wifiServer.ShowInfoWifi(i); cout<<" to "<< wifiServer.GetNumberClient()-1 << " others clients" <<endl;
#endif
							wifiServer.SendAllOtherClients(i,power,buffer,valread);
						}

					}
				}

				i++;
			}
		}
	}

	return 0;
}

