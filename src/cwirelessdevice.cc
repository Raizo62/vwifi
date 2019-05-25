#include "cwirelessdevice.h"

#include <cstring>

WirelessDevice::WirelessDevice(){

}
 


WirelessDevice::~WirelessDevice(){

}

WirelessDevice::WirelessDevice(std::string name,int index ,int iftype ,const struct ether_addr & macaddr):_name(name),_index(index),_iftype(iftype){



	std::memcpy(&_macaddr,&macaddr,ETH_ALEN); 

}


struct ether_addr WirelessDevice::getMacaddr() const  {


	return _macaddr ;

}


void  WirelessDevice::setMacaddr(const struct ether_addr & macaddr)  {


	std::memcpy(&_macaddr,&macaddr,ETH_ALEN); 

}




// friend functions

std::ostream &   operator<< ( std::ostream & os , WirelessDevice & wdevice ){


	char macstring[18];

	sprintf(macstring, "%02X:%02X:%02X:%02X:%02X:%02X",
		wdevice._macaddr.ether_addr_octet[0], wdevice._macaddr.ether_addr_octet[1], wdevice._macaddr.ether_addr_octet[2],
		wdevice._macaddr.ether_addr_octet[3], wdevice._macaddr.ether_addr_octet[4], wdevice._macaddr.ether_addr_octet[5]);

	os << "name: " << wdevice._name << "," 
		<< " index: " << wdevice._index << "," << " iftype:" <<  wdevice._iftype << "," << " mac:" << macstring << std::endl   ;
	return os ;
}



