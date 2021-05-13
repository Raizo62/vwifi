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

		bool ListInfoSelfManaged;
		CListInfo<CInfoSocket>* InfoSockets;

		TDescriptor Accept(struct sockaddr_in& address);

		TDescriptor GetSocketClient(TIndex index) const;

	public :

		explicit CSocketServer(CListInfo<CInfoSocket>* infoSockets = NULL);

		CSocketServer(TSocket type, CListInfo<CInfoSocket>* infoSockets = NULL);

		CSocketServer( const CSocketServer & socketServer );

		~CSocketServer();

		void Init(TPort port);

		TPort GetPort() const;

		virtual bool Listen();

		virtual TDescriptor Accept();

		TIndex GetNumberClient() const;

		bool IsEnable(TIndex index);

		void DisableClient(TIndex index);

		void CloseClient(TIndex index);

		ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData) override;
		ssize_t SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData) override;

		ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData) override;
		ssize_t ReadBigData(TDescriptor descriptor, CDynBuffer* data) override;

		TDescriptor operator[] (TIndex index);
};

#endif
