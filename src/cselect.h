#ifndef _CSELECT_H_
#define _CSELECT_H_

#include <sys/select.h> // fd_set
#include <sys/socket.h>
#include <vector> // vector

#include "types.h" // TDescriptor

const int SCHEDULER_ERROR=-1;

class CSelect
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

		CSelect();

		bool AddNode(TDescriptor descriptor);

		void DelNode(TDescriptor descriptor);

		TDescriptor Wait();

		TDescriptor Wait(const sigset_t *sigmask);

		bool DescriptorHasAction(TDescriptor descriptor);

		bool NodeHasAction(TIndex index);
};

#endif
