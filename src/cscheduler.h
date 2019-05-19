#ifndef _CSCHEDULER_
#define _CSCHEDULER_

#include <sys/socket.h>

const int ERROR_SCHEDULER=-1;

const int MAX_NODE = 30;

typedef int Descriptor;

class CScheduler
{
	private :

	//set of socket descriptors
	fd_set Master;

	Descriptor ListNodes[MAX_NODE];
	int NumberNode;

	public :

	CScheduler();

	void Init();

	bool AddNode(Descriptor descriptor);

	void DelNode(Descriptor descriptor);

	Descriptor Wait();

	bool NodeHasAction(Descriptor descriptor);
};

#endif
