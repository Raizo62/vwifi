#ifndef _CSOCKETSERVER_H_
#define _CSOCKETSERVER_H_

#include "csocket.h"
#include "cinfosocket.h"
#include "clistinfo.h"
#include "types.h" // TIndex

class CSocketServer : public CSocket
{
	protected :

		TPort Port;

		bool ListInfoSocketSelfManaged;
		CListInfo<CInfoSocket>* InfoSockets;

		TDescriptor Accept(struct sockaddr_in& address);

		TDescriptor GetSocketClient(TIndex index);

	public :

		CSocketServer(CListInfo<CInfoSocket>* infoSockets = NULL);

		CSocketServer(TSocket type, CListInfo<CInfoSocket>* infoSockets = NULL);

		~CSocketServer();

		void Init(TPort port);

		TPort GetPort();

		virtual bool Listen();

		virtual TDescriptor Accept();

		TIndex GetNumberClient();

		bool IsEnable(TIndex index);

		void DisableClient(TIndex index);

		void CloseClient(TIndex index);

		ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData);
		ssize_t SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData);

		ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData);
		ssize_t ReadBigData(TDescriptor descriptor, CDynBuffer* data);

		TDescriptor operator[] (TIndex index);
};

#endif
