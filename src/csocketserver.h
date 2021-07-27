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

		TDescriptor GetSocketClient(TIndex index) const;

		TDescriptor Accept(TCID& cid);
		TDescriptor Accept();

		explicit CSocketServer(CListInfo<CInfoSocket>* infoSockets = NULL);

		CSocketServer(TSocket type, CListInfo<CInfoSocket>* infoSockets = NULL);

		CSocketServer( const CSocketServer & socketServer );

		~CSocketServer();

		CSocketServer& operator=(const CSocketServer& socketServer);

		TPort GetPort() const;

		void DisableClient(TIndex index);

		void CloseClient(TIndex index);

		ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData) override;
		ssize_t SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData) override;

		ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData) override;
		ssize_t ReadBigData(TDescriptor descriptor, CDynBuffer* data) override;

	// virtual :

		virtual bool _Listen(TDescriptor& master, TPort port) = 0;
		virtual TDescriptor _Accept(TDescriptor master, TCID& cid) = 0;

	public :

		TIndex GetNumberClient() const;

		TDescriptor operator[] (TIndex index);

		bool IsEnable(TIndex index);

		void Init(TPort port);

		bool Listen();
};

#endif
