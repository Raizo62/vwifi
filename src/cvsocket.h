#ifndef _CSOCKET_
#define _CSOCKET_

const int SOCKET_ERROR=-1;

typedef int Descriptor;

class CVSocket
{
	protected :

		int Master;

		CVSocket();

		bool Configure();

	public :

		int GetMaster();

		ssize_t Send(Descriptor descriptor, const char* data, ssize_t sizeOfData);

		ssize_t Read(Descriptor descriptor, char* data, ssize_t sizeOfData);

		void ShowInfo(Descriptor descriptor);
};

#endif
