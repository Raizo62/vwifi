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
	Descriptor socket;

	Number i;

	int valread;
	char buffer[1024]; //data buffer

	CScheduler scheduler;

	CWifiServer socketWifi;
	socketWifi.Init(WIFI_PORT);
	if( ! socketWifi.Listen(MAX_CLIENT_WIFI) )
	{
		cerr<<"Error : socketWifi.Listen"<<endl;
		exit(EXIT_FAILURE);
	}

	CCTRLServer ctrlWifi(&socketWifi);
	ctrlWifi.Init(CTRL_PORT);
	if( ! ctrlWifi.Listen(1) )
	{
		cerr<<"Error : ctrlWifi.Listen"<<endl;
		exit(EXIT_FAILURE);
	}



	//add master socket to set
	scheduler.AddNode(socketWifi);
	scheduler.AddNode(ctrlWifi);

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
			if( scheduler.NodeHasAction(socketWifi) )
			{
				socket = socketWifi.Accept();
				if ( socket == SOCKET_ERROR )
				{
					cerr<<"Error : socketWifi.Accept"<<endl;
					exit(EXIT_FAILURE);
				}

				//add child sockets to set
				scheduler.AddNode(socket);

				//inform user of socket number - used in send and receive commands
				cout<<"New connection from : "; socketWifi.ShowInfoClient(socketWifi.GetNumberClient()-1) ; cout<<endl;
			}

			if( scheduler.NodeHasAction(ctrlWifi) )
			{
				ctrlWifi.ReceiveOrder();
			}

			//else its some IO operation on some other socket
			for ( i = 0 ; i < socketWifi.GetNumberClient() ; i++)
			{
				socket = socketWifi[i];

				if( scheduler.NodeHasAction(socket) )
				{
					//Check if it was for closing , and also read the
					//incoming message
					valread = socketWifi.Read( socket , buffer, sizeof(buffer));
					if ( valread >= 0 )
					{
						if ( valread == 0 )
						{
							//Somebody disconnected , get his details and print
							cout<<"Host disconnected : "; socketWifi.ShowInfoClient(i) ; cout<<endl;

							//Close the socket
							socketWifi.CloseClient(i);

							//del master socket to set
							scheduler.DelNode(socket);
						}

						//Echo back the message that came in
						else
						{
							//set the string terminating NULL byte on the end
							//of the data read
							//buffer[valread] = '\0';
							//socketWifi.Send(socket,buffer , strlen(buffer));
							// send to all other clients
							if( socketWifi.GetNumberClient() > 1 )
							{
#ifdef _DEBUG
								cout<<"Forward "<<valread<<" bytes from "; socketWifi.ShowInfoClient(i); cout<<" to "<< socketWifi.GetNumberClient()-1 << " others clients" <<endl;
#endif
								socketWifi.SendAllOtherClients(i,buffer,valread);
							}
						}
					}
				}
			}
		}
	}

	return 0;
}

