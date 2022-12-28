#include "cwirelessdevice.h"

#include <sys/socket.h>
#include <net/if_arp.h>

#include <cstring>

#include <regex>

const std::regex wlan("wlan[0-9]*");

WirelessDevice::WirelessDevice(){

}

WirelessDevice::~WirelessDevice(){

}

WirelessDevice::WirelessDevice(const std::string & name,int index ,int iftype ,const struct ether_addr & macaddr,int txpower):_name(name),_index(index),_iftype(iftype), _txpower(txpower), _macaddr(macaddr), _machwsim(macaddr)  {

//	_machwsim.ether_addr_octet[0] |= 0x40 ;

//	std::memcpy(&_macaddr,&macaddr,ETH_ALEN);

}

WirelessDevice::WirelessDevice(const std::string & name,int index ,int iftype ,const struct ether_addr & macaddr,const struct ether_addr & machwsim,int txpower):_name(name),_index(index),_iftype(iftype), _txpower(txpower), _macaddr(macaddr), _machwsim(machwsim) {

}

std::string WirelessDevice::getName() const {

	return _name ;
}

struct ether_addr WirelessDevice::getMacaddr() const  {

	return _macaddr ;

}

void  WirelessDevice::setMachwsim(const struct ether_addr & machwsim)  {

	_machwsim = machwsim ;
//	std::memcpy(&_macaddr,&macaddr,ETH_ALEN);
}

struct ether_addr WirelessDevice::getMachwsim() const  {

	return _machwsim ;
}

bool WirelessDevice::checkif_wireless_device(){

	if (std::regex_match(_name,wlan))
		return true ;
	else
		return false;
}

int WirelessDevice::getIndex() const {

	return _index ;
}

int WirelessDevice::getTxPower() const {

	return _txpower ;
}

// friend functions

std::ostream &   operator<< ( std::ostream & os , const WirelessDevice & wdevice ){

	char macstring[18];
	char machwsimstring[18];

	sprintf(macstring, "%02X:%02X:%02X:%02X:%02X:%02X",
		wdevice._macaddr.ether_addr_octet[0], wdevice._macaddr.ether_addr_octet[1], wdevice._macaddr.ether_addr_octet[2],
		wdevice._macaddr.ether_addr_octet[3], wdevice._macaddr.ether_addr_octet[4], wdevice._macaddr.ether_addr_octet[5]);

	sprintf(machwsimstring, "%02X:%02X:%02X:%02X:%02X:%02X",
		wdevice._machwsim.ether_addr_octet[0], wdevice._machwsim.ether_addr_octet[1], wdevice._machwsim.ether_addr_octet[2],
		wdevice._machwsim.ether_addr_octet[3], wdevice._machwsim.ether_addr_octet[4], wdevice._machwsim.ether_addr_octet[5]);

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

	os <<  "mac hwsim:" << machwsimstring << std::endl   ;

	os << "Tx-Power : " << wdevice._txpower / 100 << "." <<  wdevice._txpower % 100 << "dBm" << std::endl ;

	return os ;
}

