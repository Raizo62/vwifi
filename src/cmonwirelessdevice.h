#include "cwirelessdevice.h"
#include <list>
#include <mutex>



/** Buffer size for netlink route interface list */
#define IFLIST_REPLY_BUFFER     4096

class MonitorWirelessDevice {



	bool _outsideloop { true } ;
	bool _started { false } ;
	int _inetsock ;
	
	std::mutex _startedmutex ;
	std::mutex _outsideloopmutex ;

	int main_loop();

	/**
	 *	@brief processes netlink route events from the kernel
	 *	Determines the event type for netlink route messages (del/new).
	 *	@return void
 	*/
	void recv_inet_event();
	bool outside_loop();
	void clean();

	/**
	 *	@brief processes RTM_NEWLINK messages
	 *	Detects when interfaces are created and modified
	 *	@param h - netlink message header
	 *	@return void
 	*/
	void new_net_interface(struct nlmsghdr *) ;

public:


	MonitorWirelessDevice();
	~MonitorWirelessDevice();

	void start();
	void stop();
	bool started(); 

	/*
	 * @brief returns a list of existing wireless network interfaces 
	 */
	std::list<WirelessDevice> &  get_wireless_devices_list();



};
