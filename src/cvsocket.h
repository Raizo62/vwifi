#ifndef _CSOCKET_
#define _CSOCKET_

const int SOCKET_ERROR=-1;
const int SOCKET_OK=0;

typedef int Descriptor;

class CVSocket
{
	protected :

		Descriptor Master;

		CVSocket();

		bool Configure();

	public :

		int GetMaster();

		virtual ssize_t Send(Descriptor descriptor, const char* data, ssize_t sizeOfData);

		virtual ssize_t Read(Descriptor descriptor, char* data, ssize_t sizeOfData);

		void ShowInfo(Descriptor descriptor);
};

#endif
