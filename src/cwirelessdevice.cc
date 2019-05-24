#include "cwirelessdevice.h"

#include <cstring>

WirelessDevice::WirelessDevice(){

}
 


WirelessDevice::~WirelessDevice(){

}

WirelessDevice::WirelessDevice(std::string name,int index ,int iftype ,const unsigned char * macaddr):_name(name),_index(index),_iftype(iftype){


	std::size_t size = sizeof macaddr ;

	if ( size  > ETH_ALEN )
		size = ETH_ALEN ;

	std::memcpy(_macaddr,macaddr,size); 

}


const unsigned char * WirelessDevice::getMacaddr() const  {


	return _macaddr ;

}


void  WirelessDevice::setMacaddr(const unsigned char * macaddr)  {

	
	std::size_t size = sizeof macaddr ;

	if ( size  > ETH_ALEN )
		size = ETH_ALEN ;

	std::memcpy(_macaddr,macaddr,size); 
}




// friend functions

std::ostream &   operator<< ( std::ostream & os , WirelessDevice & wdevice ){

	os << "name: " << wdevice._name << "," 
		<< " index: " << wdevice._index << "," << " iftype:" <<  wdevice._iftype << "," << " mac: " << wdevice._macaddr  ;
	return os ;
}


