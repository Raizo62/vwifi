#ifndef _CWIFISERVERVTCP_H_
#define _CWIFISERVERVTCP_H_

#include "csocketserverfunctionvtcp.h"
#include "cwifiserver.h"

class CWifiServerVTCP : public CWifiServer, public CSocketServerFunctionVTCP
{
	public:

		CWifiServerVTCP(CListInfo<CInfoSocket>* infoSockets, CListInfo<CInfoWifi>* infoWifis, CListInfo<CInfoWifi>* infoWifisDeconnected);

	private :

		bool _Listen(TDescriptor& master, TPort port) override;

		TDescriptor _Accept(TDescriptor master, TCID& cid) override;
};

#endif
