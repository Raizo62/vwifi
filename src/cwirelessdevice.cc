#include "cwirelessdevice.h"

#include <sys/socket.h>
#include <linux/if_arp.h>

#include <cstring>

#include <regex>


WirelessDevice::WirelessDevice(){

}
 


WirelessDevice::~WirelessDevice(){

}

WirelessDevice::WirelessDevice(std::string name,int index ,int iftype ,const struct ether_addr & macaddr):_name(name),_index(index),_iftype(iftype){

	_macaddr = macaddr ;
//	std::memcpy(&_macaddr,&macaddr,ETH_ALEN); 

}



WirelessDevice::WirelessDevice(std::string name,int index ,int iftype ,const struct ether_addr & macaddr,const struct ether_addr & machwsim):_name(name),_index(index),_iftype(iftype){


	_macaddr = macaddr ;
	_machwsim = machwsim ;

}


struct ether_addr WirelessDevice::getMacaddr() const  {


	return _macaddr ;

}


void  WirelessDevice::setMacaddr(const struct ether_addr & macaddr)  {

	_macaddr = macaddr ;
//	std::memcpy(&_macaddr,&macaddr,ETH_ALEN); 

}

void  WirelessDevice::setMachwsim(const struct ether_addr & machwsim)  {

	_machwsim = machwsim ;
//	std::memcpy(&_macaddr,&macaddr,ETH_ALEN); 
}

struct ether_addr WirelessDevice::getMachwsim() const  {

	return _machwsim ;
}


bool WirelessDevice::checkif_wireless_device(){


	std::regex wlan("wlan[0-9]*");
	if (std::regex_match(_name,wlan))
		return true ;
	else
		return false;
}

int WirelessDevice::getIndex() const {


	return _index ;
}	


// friend functions

std::ostream &   operator<< ( std::ostream & os , WirelessDevice & wdevice ){


	char macstring[18];

	sprintf(macstring, "%02X:%02X:%02X:%02X:%02X:%02X",
		wdevice._macaddr.ether_addr_octet[0], wdevice._macaddr.ether_addr_octet[1], wdevice._macaddr.ether_addr_octet[2],
		wdevice._macaddr.ether_addr_octet[3], wdevice._macaddr.ether_addr_octet[4], wdevice._macaddr.ether_addr_octet[5]);

	os << "name: " << wdevice._name << std::endl ;

	os << "index: " << wdevice._index << std::endl ;
	
	
	/* ARPHRD_ETHER normal, ARPHRD_IEEE80211_RADIOTAP as monitor */
	if (wdevice._iftype == ARPHRD_ETHER)

		os <<  "iftype: ARPHRD_ETHER" << std::endl ;

	else if (wdevice._iftype == ARPHRD_IEEE80211_RADIOTAP)

		os << "iftype: ARPHRD_IEEE80211_RADIOTAP" << std::endl;

	else

		os << "iftype: UNKNOWN" << std::endl;


        os <<  "mac:" << macstring << std::endl   ;
	
	return os ;
}



