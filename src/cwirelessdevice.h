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
	struct ether_addr _machwsim ;

	public:

	WirelessDevice();
	~WirelessDevice();

	/**
	 * \fn WirelessDevice(std::string n,int i,int t,const struct ether_addr & m,const struct ether_addr & h);
	 * \biref Constructor
	 * \param n -  interface name
	 * 	  i -  index
	 * 	  t -  type
	 * 	  m -  wireless net device mac address
	 * 	  h -  wireless net device mac address in hwsim driver 
	 */
	WirelessDevice(std::string,int,int,const struct ether_addr &,const struct ether_addr &);
	
	/**
	 * \fn WirelessDevice(std::string n,int i,int t,const struct ether_addr & m);
	 * \biref Constructor
	 * \param n -  interface name
	 * 	  i -  index
	 * 	  t -  type
	 * 	  m -  wireless net device mac address
	 */
	WirelessDevice(std::string,int,int,const struct ether_addr &);

	
	friend std::ostream &   operator<< ( std::ostream & , WirelessDevice &);
        struct ether_addr getMacaddr() const  ;
	 struct ether_addr getMachwsim() const  ;
	void setMacaddr(const struct  ether_addr &);
	void setMachwsim(const struct  ether_addr &);

	bool checkif_wireless_device();
	int getIndex() const ;

};


#endif
