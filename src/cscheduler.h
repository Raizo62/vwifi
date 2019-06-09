#ifndef _CSCHEDULER_H_
#define _CSCHEDULER_H_

#include <sys/socket.h>

#include "config.h"
#include "types.h" // Descriptor

const int SCHEDULER_ERROR=-1;

const int MAX_NODE = (2+(MAX_CLIENT_WIFI)); // 2 = socketWifi and ctrlWifi

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
