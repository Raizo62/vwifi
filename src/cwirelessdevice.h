#ifndef _WIRELESSDEVICE_H
#define _WIRELESSDEVICE_H

#include <net/ethernet.h>

#include <string> 
#include <iostream>


class WirelessDevice {


	std::string _name;
	int _index;
	int _iftype;
	unsigned char _macaddr[ETH_ALEN];


	public:

	WirelessDevice();
	~WirelessDevice();
	WirelessDevice(std::string,int,int,const unsigned char *);
	friend std::ostream &   operator<< ( std::ostream & , WirelessDevice &);
        const unsigned char * getMacaddr() const  ;
	void setMacaddr(const unsigned char *);	

};


#endif
