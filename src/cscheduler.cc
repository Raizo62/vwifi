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

	MaxDescriptor=-1;
}

void CScheduler::AddNode(Descriptor descriptor)
{
	FD_SET( descriptor , &Master);

	//highest file descriptor number, need it for the select function
	if( descriptor > MaxDescriptor )
		MaxDescriptor = descriptor;
}

void CScheduler::DelNode(Descriptor descriptor, Descriptor maxDescriptor)
{
	FD_CLR( descriptor , &Master);

	//highest file descriptor number, need it for the select function
	MaxDescriptor = maxDescriptor;
}

Descriptor CScheduler::Wait()
{
	int activity=select( MaxDescriptor + 1 , &Master , NULL , NULL , NULL);

	if ((activity < 0) && (errno!=EINTR))
		return ERROR_SCHEDULER;

	return activity;
}

bool CScheduler::NodeHasAction(Descriptor descriptor)
{
	return FD_ISSET( descriptor , &Master);
}


