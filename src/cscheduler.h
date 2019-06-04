#ifndef _CSCHEDULER_
#define _CSCHEDULER_

#include <sys/socket.h>

#include "config.h"

const int SCHEDULER_ERROR=-1;

const int MAX_NODE = (2+(MAX_CLIENT)); // 2 = socketWifi and ctrlWifi

typedef int Descriptor;

class CScheduler
{
	private :

	//set of socket descriptors
	fd_set Master;
	fd_set Dup;

	Descriptor MaxDescriptor;

	Descriptor ListNodes[MAX_NODE];
	int NumberNode;

	void UpdateMaxDescriptor(Descriptor descriptor);

	public :

	CScheduler();

	void Init();

	bool AddNode(Descriptor descriptor);

	void DelNode(Descriptor descriptor);

	Descriptor Wait();

	bool NodeHasAction(Descriptor descriptor);
};

#endif
