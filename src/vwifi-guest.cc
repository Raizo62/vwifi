#include "cwificlient.h"

#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <string.h> // strcmp

#include <memory>

enum STATE {

	STARTED=1,
	STOPPED ,
	SUSPENDED 
};

CKernelWifi* wifiClient;

enum STATE  _state = STOPPED ;


void  signal_handler(int signal_num)
{

	switch(signal_num)
	{
		case SIGINT :
		case SIGTERM :
		case SIGQUIT :
			
			std::cout << signal_num << std::endl ;
			wifiClient->stop() ;
			_state = STOPPED ;
			break ;

		case SIGTSTP:

			std::cout << "This signal is ignored" << std::endl ;
			break ;
		

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
	//signal(SIGCONT, signal_handler);

	if( argc > 2 )
	{
		std::cerr<<"Error : too many parameters"<<std::endl;
		return 1;
	}

	if( argc == 2 )
	{
		if( ! strcmp("-v", argv[1]) || ! strcmp("--version", argv[1]) )
		{
			std::cout<<"Version : "<<VERSION<<std::endl;
			return 0;
		}

		wifiClient=new CWifiClient<CSocketClientINET>;
		((CWifiClient<CSocketClientINET>*)wifiClient)->Init(argv[1], WIFI_GUEST_PORT_INET);
	}
	else
	{
		wifiClient=new CWifiClient<CSocketClientVHOST>;
		((CWifiClient<CSocketClientVHOST>*)wifiClient)->Init(WIFI_GUEST_PORT_VHOST);
	}

	if(!wifiClient->start())
		std::cout << "Starting process aborted" << std::endl ;



	std::cout << "Good Bye (:-)" << std::endl ; 

	_exit(EXIT_SUCCESS);

}
