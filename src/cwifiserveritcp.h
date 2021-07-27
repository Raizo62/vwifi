#ifndef _CWIFISERVERITCP_H_
#define _CWIFISERVERITCP_H_

#include "csocketserverfunctionitcp.h"
#include "cwifiserver.h"

class CWifiServerITCP : public CWifiServer, public CSocketServerFunctionITCP
{
	public:

		CWifiServerITCP();

		CWifiServerITCP(CListInfo<CInfoSocket>* infoSockets, CListInfo<CInfoWifi>* infoWifis, CListInfo<CInfoWifi>* infoWifisDeconnected);

	private:

		bool _Listen(TDescriptor& master, TPort port);

		TDescriptor _Accept(TDescriptor master, TCID& cid);
};

#endif
