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
	int _txpower ;
	struct ether_addr _macaddr ;
	struct ether_addr _machwsim = {0x00,0X00,0x00,0X00,0x00,0X00};

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
	WirelessDevice(const std::string &,int,int,const struct ether_addr &,const struct ether_addr &,int);

	/**
	 * \fn WirelessDevice(std::string n,int i,int t,const struct ether_addr & m);
	 * \biref Constructor
	 * \param n -  interface name
	 * 	  i -  index
	 * 	  t -  type
	 * 	  m -  wireless net device mac address
	 */
	WirelessDevice(const std::string &,int,int,const struct ether_addr &,int);

	friend std::ostream &   operator<< ( std::ostream & , WirelessDevice &);

	struct ether_addr getMacaddr() const  ;
	struct ether_addr getMachwsim() const  ;

	void setMacaddr(const struct  ether_addr &);
	void setMachwsim(const struct  ether_addr &);

	void setName(const std::string &);
	std::string getName() const ;

	bool checkif_wireless_device();
	int getIndex() const ;
	int getTxPower() const ;

};


#endif
