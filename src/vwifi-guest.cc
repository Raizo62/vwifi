#include "cvwifiguest.h"

#include <signal.h>
#include <unistd.h>
#include <iostream>

#include <memory>

enum STATE {

	STARTED=1,
	STOPPED ,
	SUSPENDED 
};

VWifiGuest  vwifi_guest ;

enum STATE  _state = STOPPED ;


void  signal_handler(int signal_num)
{

	std::cout << "signal " << signal_num << " received" << std::endl ;

	switch(signal_num)
	{
		case SIGINT :
		case SIGTERM :
		case SIGQUIT :
			
			vwifi_guest.stop() ;
			_state = STOPPED ;
			break ;
		case  SIGTSTP :
			std::cout << "Suspended" << std::endl ;
			vwifi_guest.stop();
			_state = SUSPENDED ;
			break ;

		case SIGCONT :

			std::cout << "Resume" << std::endl ;
			if(!vwifi_guest.start())
				std::cout << "Starting process aborted" << std::endl ;
			_state = STARTED ;
			break;

		default :
			std::cerr << "Signal not handled" << std::endl ;
	
	}

}


int main (int argc , char ** argv){


	/* Handle signals */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTSTP, signal_handler);
	signal(SIGCONT, signal_handler);

	if(!vwifi_guest.init())
		std::cout << "Init process aborted" << std::endl ;


	if(!vwifi_guest.start())
		std::cout << "Starting process aborted" << std::endl ;


	while(true){
	
		if (_state == STOPPED )
		         
				break ;
		
		if ((_state == SUSPENDED )|| (_state == STARTED )){
		
				pause();
				continue;
		}
		
	}

	std::cout << "Good Bye (:-)" << std::endl ; 

	_exit(EXIT_SUCCESS);

}
