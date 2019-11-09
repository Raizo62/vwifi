#ifndef _CSCHEDULER_H_
#define _CSCHEDULER_H_

#include <sys/socket.h>
#include <vector> // vector

#include "types.h" // TDescriptor

const int SCHEDULER_ERROR=-1;

class CScheduler
{
	private :

		//set of socket descriptors
		fd_set Master;
		fd_set Dup;

		TDescriptor MaxDescriptor;

		std::vector<TDescriptor> ListNodes;

		void UpdateMaxDescriptor(TDescriptor descriptor);

		void Init();

	public :

		CScheduler();

		bool AddNode(TDescriptor descriptor);

		void DelNode(TDescriptor descriptor);

		TDescriptor Wait();

		bool DescriptorHasAction(TDescriptor descriptor);
};

#endif
