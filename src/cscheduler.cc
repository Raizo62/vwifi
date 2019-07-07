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

void CScheduler::UpdateMaxDescriptor(TDescriptor descriptor)
{
	//highest file descriptor number, need it for the select function
	if ( descriptor > MaxDescriptor )
		MaxDescriptor=descriptor;
}

bool CScheduler::AddNode(TDescriptor descriptor)
{
	//add new socket to array of sockets
	ListNodes.push_back(descriptor);
	NumberNode++;

	//add child sockets to set
	FD_SET( descriptor , &Master);

	//highest file descriptor number, need it for the select function
	UpdateMaxDescriptor(descriptor);

	return true;
}

void CScheduler::DelNode(TDescriptor descriptor)
{
	MaxDescriptor=-1;
	for (TIndex i = 0; i < NumberNode; i++)
	{
		if( ListNodes[i] == descriptor )
		{
			FD_CLR(descriptor , &Master);

			ListNodes.erase (ListNodes.begin()+i);
			NumberNode--;

			for(TIndex j=i;j<NumberNode;j++)
				UpdateMaxDescriptor(ListNodes[j]);

			return;
		}
		UpdateMaxDescriptor(ListNodes[i]);
	}
}

TDescriptor CScheduler::Wait()
{
	/* back up master */
	Dup = Master;

	//wait for an activity on one of the sockets , timeout is NULL ,
	//so wait indefinitely
	int activity=select( MaxDescriptor + 1 , &Dup , NULL , NULL , NULL);

	if ((activity < 0) && (errno!=EINTR))
		return SCHEDULER_ERROR;

	return activity;
}

bool CScheduler::NodeHasAction(TDescriptor descriptor)
{
	return FD_ISSET( descriptor , &Dup);
}


