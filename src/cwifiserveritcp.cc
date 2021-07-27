#include"cwifiserveritcp.h"

CWifiServerITCP::CWifiServerITCP() : CWifiServer()
{
}

CWifiServerITCP::CWifiServerITCP(CListInfo<CInfoSocket>* infoSockets, CListInfo<CInfoWifi>* infoWifis, CListInfo<CInfoWifi>* infoWifisDeconnected) : CWifiServer(infoSockets, infoWifis, infoWifisDeconnected)
{
}

bool CWifiServerITCP::_Listen(TDescriptor& master, TPort port)
{
	return CSocketServerFunctionITCP::_Listen(master, port);
}

TDescriptor CWifiServerITCP::_Accept(TDescriptor master, TCID& cid)
{
	return CSocketServerFunctionITCP::_Accept(master, cid);
}
