#include <errno.h> // errno

#include <iostream> // cout & NULL

#include "cscheduler.h"

using namespace std;

CScheduler::CScheduler()
{
	Init();
}

void CScheduler::Init()
{
	//clear the socket set
	FD_ZERO(&Master);

	NumberNode=0;
}

bool CScheduler::AddNode(Descriptor descriptor)
{
	if ( NumberNode >= MAX_NODE )
		return ERROR_SCHEDULER;

	//add new socket to array of sockets
	cout<<"Adding to list of node as "<<NumberNode<<endl;
	ListNodes[NumberNode++] = descriptor;

	return 0;
}

void CScheduler::DelNode(Descriptor descriptor)
{
	for (int i = 0; i < NumberNode; i++)
		if( ListNodes[i] == descriptor )
		{
			NumberNode--;
			for(int j=i;j<NumberNode;j++)
				ListNodes[j] = ListNodes[j+1];
			return;
		}
}

Descriptor CScheduler::Wait()
{
	int maxDescriptor=-1;

	//clear the socket set
	FD_ZERO(&Master);

	//add child sockets to set
	for (int i = 0; i < NumberNode; i++)
	{
		FD_SET( ListNodes[i] , &Master);

		//highest file descriptor number, need it for the select function
		if ( ListNodes[i] > maxDescriptor )
			maxDescriptor=ListNodes[i];
	}

	//wait for an activity on one of the sockets , timeout is NULL ,
	//so wait indefinitely
	int activity=select( maxDescriptor + 1 , &Master , NULL , NULL , NULL);

	if ((activity < 0) && (errno!=EINTR))
		return ERROR_SCHEDULER;

	return activity;
}

bool CScheduler::NodeHasAction(Descriptor descriptor)
{
	return FD_ISSET( descriptor , &Master);
}


