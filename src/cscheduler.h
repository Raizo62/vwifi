#ifndef _CSCHEDULER_
#define _CSCHEDULER_

#include <sys/socket.h>

const int ERROR_SCHEDULER=-1;

typedef int Descriptor;

class CScheduler
{
	private :

	//set of socket descriptors
	fd_set Master;

	//highest file descriptor number, need it for the select function
	int MaxDescriptor;

	public :

	CScheduler();

	void Init();

	void AddNode(Descriptor descriptor);

	Descriptor Wait();

	bool NodeHasAction(Descriptor descriptor);
};

#endif
