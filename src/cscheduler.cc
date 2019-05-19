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

	//clear the socket set
	FD_ZERO(&Dup);

	NumberNode=0;
	MaxDescriptor=-1;
}

void CScheduler::UpdateMaxDescriptor(Descriptor descriptor)
{
	//highest file descriptor number, need it for the select function
	if ( descriptor > MaxDescriptor )
		MaxDescriptor=descriptor;
}

bool CScheduler::AddNode(Descriptor descriptor)
{
	if ( NumberNode >= MAX_NODE )
		return SCHEDULER_ERROR;

	//add new socket to array of sockets
	cout<<"Adding to list of node as "<<NumberNode<<endl;
	ListNodes[NumberNode++] = descriptor;

	//add child sockets to set
	FD_SET( descriptor , &Master);

	//highest file descriptor number, need it for the select function
	UpdateMaxDescriptor(descriptor);

	return 0;
}

void CScheduler::DelNode(Descriptor descriptor)
{
	MaxDescriptor=-1;
	for (int i = 0; i < NumberNode; i++)
	{
		if( ListNodes[i] == descriptor )
		{
			FD_CLR(descriptor , &Master);

			NumberNode--;
			for(int j=i;j<NumberNode;j++)
			{
				ListNodes[j] = ListNodes[j+1];
				UpdateMaxDescriptor(ListNodes[j]);
			}
			return;
		}
		UpdateMaxDescriptor(ListNodes[i]);
	}
}

Descriptor CScheduler::Wait()
{
	/* back up master */
	Dup = Master;

	//wait for an activity on one of the sockets , timeout is NULL ,
	//so wait indefinitely
	cout<<"MaxDescriptor : "<<MaxDescriptor<<endl;
	int activity=select( MaxDescriptor + 1 , &Dup , NULL , NULL , NULL);

	if ((activity < 0) && (errno!=EINTR))
		return SCHEDULER_ERROR;

	return activity;
}

bool CScheduler::NodeHasAction(Descriptor descriptor)
{
	return FD_ISSET( descriptor , &Dup);
}


