#include "cwirelessdevice.h"

#include <iostream>

int main (int agc, char ** argv){


	unsigned char addr[1000] = "ffaabc048844" ;
	WirelessDevice wdevice("wlan0",4,5,addr);
	std::cout << wdevice << std::endl ;

	WirelessDevice wdevice1(wdevice);
	
	std::cout << wdevice1 << std::endl ;

	std::cout << "MAC :" <<  wdevice.getMacaddr() << std::endl ;

	unsigned char addr1[1000] = "ggggggaaaaa" ;

	wdevice.setMacaddr(addr1);

	std::cout << "MAC :" <<  wdevice.getMacaddr() << std::endl ;

	return 0 ;



}
