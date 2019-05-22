#include "cvwifiguest.h"

#include <signal.h>
#include <unistd.h>
#include <iostream>

#include <memory>

VWifiGuest * vwifi_guest ;


void  signal_handler(int signal_num)
{
	std::cout << __func__ << std::endl ;

	vwifi_guest->stop() ;

}




int main (int argc , char ** argv){


	vwifi_guest = new VWifiGuest();

	/* Handle signals */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP, SIG_IGN);


	if(!vwifi_guest->start())
		std::cout << "Starting process aborted" << std::endl ;

	std::cout << "Good Bye (:-)" << std::endl ; 

	delete vwifi_guest ;
	_exit(EXIT_SUCCESS);

}
