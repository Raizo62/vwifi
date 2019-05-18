#include <string> //string
#include <iostream> // cout

#include <string.h> //strlen

#include "vwifi-host-server.h"
#include "csocketserver.h"
#include "cscheduler.h"

using namespace std;

int main(int argc , char *argv[])
{
	Descriptor socket;

	int i , valread;

	char buffer[1025]; //data buffer of 1K

	CScheduler scheduler;

	CSocketServer socketServer;
	socketServer.Init(PORT);
	if( socketServer.Listen() )
	{
				cout<<"Error : socketServer.Listen"<<endl;
				exit(EXIT_FAILURE);
	}

	scheduler.AddNode(socketServer.GetMasterSocket());

	//a message
	string message = "ECHO Daemon v1.0 \r\n";

	//accept the incoming connection
	cout<<"Waiting for connections ..."<<endl;

	while(TRUE)
	{
			//clear the socket set
		scheduler.Init();

		//add master socket to set
		scheduler.AddNode(socketServer.GetMasterSocket());
		//add child sockets to set
		for ( i = 0 ; i < socketServer.GetNumberClient() ; i++)
			scheduler.AddNode(socketServer.GetSocketClient(i));

		//wait for an activity on one of the sockets , timeout is NULL ,
		//so wait indefinitely
		if( scheduler.Wait() == ERROR_SCHEDULER )
		{
			cout<<"Error : scheduler.Wait"<<endl;
		}

		//If something happened on the master socket ,
		//then its an incoming connection
		if( scheduler.NodeHasAction(socketServer.GetMasterSocket()) )
		{
			socket = socketServer.Accept();
			if ( socket == ERROR_SOCKET )
			{
				cout<<"Error : socketServer.Accept"<<endl;
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			cout<<"New connection , socket fd is : "<<socket<<" "; socketServer.ShowInfo(socket) ; cout<<endl;

			//send new connection greeting message
			if (  (socklen_t) socketServer.Send(socket,message.c_str(), message.length()) != message.length() )
			{
				cout<<"Error : socketServer.Send"<<endl;
			}

			cout<<"Welcome message sent successfully"<<endl;
		}

		//else its some IO operation on some other socket
		for ( i = 0 ; i < socketServer.GetNumberClient() ; i++)
		{
			socket = socketServer.GetSocketClient(i);

			if( scheduler.NodeHasAction(socket) )
			{
				//Check if it was for closing , and also read the
				//incoming message
				valread = socketServer.Read( socket , buffer, 1024);
				if ( valread == 0 )
				{
					//Somebody disconnected , get his details and print
					cout<<"Host disconnected , "; socketServer.ShowInfo(socket) ; cout<<endl;

					//Close the socket
					socketServer.CloseClient(i);
				}

				//Echo back the message that came in
				else
				{
					//set the string terminating NULL byte on the end
					//of the data read
					buffer[valread] = '\0';
					socketServer.Send(socket,buffer , strlen(buffer));
				}
			}
		}
	}

	return 0;
}

