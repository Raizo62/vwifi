#include <errno.h> // errno
#include <iostream> // cout & NULL

#include <assert.h> // assert

#include "cselect.h"

using namespace std;

CSelect::CSelect()
{
	Init();
}

void CSelect::Init()
{
	//clear the socket set
	FD_ZERO(&Master);

	//clear the socket set
	FD_ZERO(&Dup);

	MaxDescriptor=-1;
}

void CSelect::UpdateMaxDescriptor(TDescriptor descriptor)
{
	//highest file descriptor number, need it for the select function
	if ( descriptor > MaxDescriptor )
		MaxDescriptor=descriptor;
}

bool CSelect::AddNode(TDescriptor descriptor)
{
	//add new socket to array of sockets
	ListNodes.push_back(descriptor);

	//add child sockets to set
	FD_SET( descriptor , &Master);

	//highest file descriptor number, need it for the select function
	UpdateMaxDescriptor(descriptor);

	return true;
}

void CSelect::DelNode(TDescriptor descriptor)
{
	MaxDescriptor=-1;
	for (auto node = ListNodes.begin() ; node != ListNodes.end(); ++node)
	{
		if( *node == descriptor )
		{
			FD_CLR(descriptor , &Master);

			ListNodes.erase(node);

			for (; node != ListNodes.end(); ++node)
				UpdateMaxDescriptor(*node);

			return;
		}
		UpdateMaxDescriptor(*node);
	}
}

TDescriptor CSelect::Wait()
{
	return Wait(NULL);
}

TDescriptor CSelect::Wait(const sigset_t *sigmask)
{
	/* back up master */
	Dup = Master;

	//wait for an activity on one of the sockets , timeout is NULL ,
	//so wait indefinitely
	int activity=pselect( MaxDescriptor + 1 , &Dup , NULL , NULL , NULL, sigmask);

	if ((activity < 0) && (errno!=EINTR))
		return SCHEDULER_ERROR;

	return activity;
}


bool CSelect::DescriptorHasAction(TDescriptor descriptor)
{
	return FD_ISSET( descriptor , &Dup);
}

bool CSelect::NodeHasAction(TIndex index)
{
	assert( index < ListNodes.size() );

	return FD_ISSET( ListNodes[index] , &Dup);
}
