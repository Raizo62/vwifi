#include "cwirelessdevice.h"

#include <iostream>

int main (int agc, char ** argv){


	struct ether_addr mac { 0xff, 0x00, 0x00,0x00, 0xfe, 0x00 };
	
	WirelessDevice wdevice("wlan0",4,5,mac);
	std::cout << wdevice << std::endl ;

 	return 0 ;



}
