#ifndef _WIRELESSDEVICE_H
#define _WIRELESSDEVICE_H

#include <net/ethernet.h>

#include <string> 
#include <iostream>


class WirelessDevice {


	std::string _name;
	int _index;
	int _iftype;
	//unsigned char _macaddr[ETH_ALEN];
	struct ether_addr _macaddr ;

	public:

	WirelessDevice();
	~WirelessDevice();
	WirelessDevice(std::string,int,int,const struct ether_addr &);
	friend std::ostream &   operator<< ( std::ostream & , WirelessDevice &);
        struct ether_addr getMacaddr() const  ;
	void setMacaddr(const struct  ether_addr &);	

};


#endif
