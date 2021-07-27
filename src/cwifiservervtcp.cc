#include"cwifiservervtcp.h"

CWifiServerVTCP::CWifiServerVTCP(CListInfo<CInfoSocket>* infoSockets, CListInfo<CInfoWifi>* infoWifis, CListInfo<CInfoWifi>* infoWifisDeconnected) : CWifiServer(infoSockets, infoWifis, infoWifisDeconnected)
{
}

bool CWifiServerVTCP::_Listen(TDescriptor& master, TPort port)
{
	return CSocketServerFunctionVTCP::_Listen(master, port);
}

TDescriptor CWifiServerVTCP::_Accept(TDescriptor master, TCID& cid)
{
	return CSocketServerFunctionVTCP::_Accept(master, cid);
}

