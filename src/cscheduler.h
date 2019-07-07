#ifndef _CSCHEDULER_H_
#define _CSCHEDULER_H_

#include <sys/socket.h>
#include <vector> // vector

#include "config.h" // MAX_CLIENT_WIFI
#include "types.h" // TDescriptor

const int SCHEDULER_ERROR=-1;

const int MAX_NODE = (2+(MAX_CLIENT_WIFI)); // 2 = socketWifi and ctrlWifi

class CScheduler
{
	private :

		//set of socket descriptors
		fd_set Master;
		fd_set Dup;

		TDescriptor MaxDescriptor;

		std::vector<TDescriptor> ListNodes;
		TIndex NumberNode;

		void UpdateMaxDescriptor(TDescriptor descriptor);

	public :

		CScheduler();

		void Init();

		bool AddNode(TDescriptor descriptor);

		void DelNode(TDescriptor descriptor);

		TDescriptor Wait();

		bool NodeHasAction(TDescriptor descriptor);
};

#endif
