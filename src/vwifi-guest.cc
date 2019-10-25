#include "cwificlient.h"

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

enum STATE pstate = _state ;

void  signal_handler(int signal_num)
{

	switch(signal_num)
	{
		case SIGINT :
		case SIGTERM :
		case SIGQUIT :
			
			std::cout << signal_num << std::endl ;
			vwifi_guest.stop() ;
			_state = STOPPED ;
			break ;
		
		case  SIGTSTP :
			
			std::cout << signal_num << std::endl ;
			vwifi_guest.stop();
			_state = SUSPENDED ;
			break ;

		case SIGCONT :

			std::cout << signal_num << std::endl ;
			pstate = _state ;
			_state = STARTED ;
			if(!vwifi_guest.start()){
			
				std::cout << "Starting process aborted" << std::endl ;
				_state = pstate ;
				break ;
			}
			break;

		default :
			std::cerr << "Signal not handled" << std::endl ;
	
	}

	std::cout << "OUT SWITCH" << std::endl ;
}


int main (int argc , char ** argv){


	/* Handle signals */
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGHUP, SIG_IGN);
	signal(SIGTSTP, signal_handler);
	signal(SIGCONT, signal_handler);



	if(!vwifi_guest.start())
		std::cout << "Starting process aborted" << std::endl ;


	while(true){
	
		if (_state == STOPPED ){
		        
		       		std::cout << "its time to live" << std::endl ;	
				break ;
		}
		
		if ((_state == SUSPENDED )|| (_state == STARTED )){
	
				std::cout << "SUSPENDED or CONT" << std::endl ;	

				pause();
				continue;
		}
		
	}

	std::cout << "Good Bye (:-)" << std::endl ; 

	_exit(EXIT_SUCCESS);

}
