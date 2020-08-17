#include "cwirelessdevice.h"
#include "cmonwirelessdevice.h"
#include <stdio.h>
#include <iostream>

#include <unistd.h>
#include <signal.h>


MonitorWirelessDevice monitor ;


void  signal_handler([[maybe_unused]] int signal_num)
{
	std::cout << __func__ << std::endl ;
	monitor.stop();
}



int main (){


	/* Handle signals */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP, SIG_IGN);



/*	struct ether_addr mac { 0xff, 0x00, 0x00,0x00, 0xfe, 0x00 };
	struct ether_addr mac3 { 0xaa, 0xbb, 0x00,0x00, 0xfe, 0x00 };

	WirelessDevice wdevice("wlan0",4,5,mac);
	std::cout << wdevice << std::endl ;

	WirelessDevice wdevice2 ;
	wdevice2 = wdevice;
	
	wdevice.setMacaddr(mac3);
	
	std::cout << wdevice2 << std::endl ;
	std::cout << wdevice << std::endl ;
*/

	monitor.start();

	pause();

	std::cout << "end" << std::endl ;

	return 0 ;

}
