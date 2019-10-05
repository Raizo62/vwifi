#ifndef _CMONITORWIRELESSDEVICE_H_
#define _CMONITORWIRELESSDEVICE_H_


#include "cwirelessdevice.h"
#include <list>
#include <mutex>
#include <functional>

namespace monitorinet {
	class CallFromStaticFunc;
}


/** Buffer size for netlink route interface list */
#define IFLIST_REPLY_BUFFER     4096

typedef std::function<void(WirelessDevice)> CallbackFunction ;


typedef struct {
    struct nl_sock *nls;
    int nl80211_id;
    int err,err1 ;
    struct nl_cb* cb;
    struct nl_cb* cb1;

} WIFI;


class MonitorWirelessDevice {




	/* set to true when wireless interfaces are initialised */
	bool init_interfaces = false ;
	
	/* for netlink communication with 80211 module */
	WIFI wifi ;

	bool _outsideloop { true } ;
	bool _started { false } ;
	int _inetsock ;
	
	std::mutex _startedmutex ;
	std::mutex _outsideloopmutex ;

	int main_loop();

	/**
	 *	\brief processes netlink route events from the kernel
	 *	Determines the event type for netlink route messages (del/new).
	 *	\return void
 	*/
	void recv_inet_event();
	bool outside_loop();
	void clean();

	/**
	 *	\brief processes RTM_NEWLINK messages
	 *	Detects when interfaces are created and modified
	 *	\param  struct nlmsghdr * - netlink message header
	 *	\return void
 	*/
	void new_net_interface(struct nlmsghdr *) ;


	void del_net_interface(struct nlmsghdr *h);


	/**
	 * \brief callback to be set through setCallback function by another object
	 * this callback is called by new_net_interface function in order to send a created, deleted or changed wirelessdevice 
	 */
	CallbackFunction _newinet_cb ;

	/**
	 * \brief callback to be set through setCallback function by another object
	 * this callback is called by new_net_interface function in order to send a created, deleted or changed wirelessdevice 
	 */
	CallbackFunction _delinet_cb ;


	/**
	 * \brief callback to be set through setCallback function by another object
	 * this callback is called by recv_winterface_infos function in order to send infos of each existing wireless interface 
	 */
	CallbackFunction _initinet_cb ;

	static int recv_winterface_infos_cb(struct nl_msg *msg, void *arg);
	static int recv_winterface_extra_infos_cb(struct nl_msg *msg, void *arg);
	static int handle_iee80211_com_finish_cb(struct nl_msg *msg, void *arg);

	static monitorinet::CallFromStaticFunc * forward ;
public:


	MonitorWirelessDevice();
	~MonitorWirelessDevice();
	MonitorWirelessDevice(const MonitorWirelessDevice &) = delete ;
	MonitorWirelessDevice & operator=(const MonitorWirelessDevice&) = delete ;

	void start();
	void stop();
	bool started(); 


	/**
	 * \brief set new interface notification callback
	 * \param CallbackFunction - defined earlier as  typedef std::function<void(WirelessDevice)> CallbackFunction 
	 * \return void
	 */
	void setNewInetCallback(CallbackFunction);

	/**
	 * \brief set delete interface notification callback
	 * \param CallbackFunction - defined earlier as  typedef std::function<void(WirelessDevice)> CallbackFunction 
	 * \return void
	 */
	void setInitInetCallback(CallbackFunction);



	/**
	 * \brief set interface initial list notification callback
	 * \param CallbackFunction - defined earlier as  typedef std::function<void(WirelessDevice)> CallbackFunction 
	 * \return void
	 */
	void setDelInetCallback(CallbackFunction);

	
	/**
	 *\fn 	int get_interface_infos()
	 * \brief Uses nl80211 to initialize a list of wireless interfaces Processes multiple netlink messages containing interface data
	 *\return error codes
 	*/
	int get_winterface_infos(int);
	int get_winterface_extra_infos(int ifindex);

	int recv_winterface_infos(struct nl_msg *msg, void *arg);
	int recv_winterface_extra_infos(struct nl_msg *msg, void *arg);

	int handle_iee80211_com_finish(struct nl_msg *msg, void *arg);

       	int nl80211_init() ;

};




/**
 * \namespace monitorinet
 *
 * A namespace is used here, since a class CallFromStaticFunc is defined in other files
 */
namespace monitorinet {

	/**
	 * \class CallFromStaticFunc
	 * \brief this class is an artifact used to call a member function from static function
	*/
	class CallFromStaticFunc {

	MonitorWirelessDevice * m_obj  ;

	public:
	
		CallFromStaticFunc(MonitorWirelessDevice *   obj){
			
			m_obj = obj ;
		};

		int recv_winterface_infos(struct nl_msg *msg, void *arg) {
		
				// add exception to check null ptr
				m_obj->recv_winterface_infos(msg , arg);
				return 0 ;
		};

		int recv_winterface_extra_infos(struct nl_msg *msg, void *arg) {
		
				// add exception to check null ptr
				m_obj->recv_winterface_extra_infos(msg , arg);
				return 0 ;
		};


		int handle_iee80211_com_finish(struct nl_msg *msg, void *arg) {
		
				// add exception to check null ptr
				m_obj->handle_iee80211_com_finish(msg , arg);
				return 0 ;
		};

	};
}

#endif
