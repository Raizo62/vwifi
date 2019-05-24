#ifndef _CSOCKET_
#define _CSOCKET_

//#define _USE_VSOCK_

const int SOCKET_ERROR=-1;
const int SOCKET_OK=0;

typedef int Descriptor;

typedef int TypeSocket;
// AF_INET : use IP
// AF_VSOCK : use vsock

class CVSocket
{
	protected :

		Descriptor Master;
		TypeSocket	Type;

		CVSocket();

		CVSocket(TypeSocket type);

		bool Configure();

		virtual ssize_t Send(Descriptor descriptor, const char* data, ssize_t sizeOfData);

		virtual ssize_t Read(Descriptor descriptor, char* data, ssize_t sizeOfData);

	public :

		void ShowInfo(Descriptor descriptor);

		void Close();

		~CVSocket();

		operator int();
};

#endif
