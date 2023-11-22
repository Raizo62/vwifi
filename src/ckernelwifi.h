#ifndef _CKERNELWIFI_H_
#define _CKERNELWIFI_H_

#include <string>
#include <mutex>

#include "config_hwsim.h"

#include "cwirelessdevice.h"
#include "cwirelessdevicelist.h"
#include "cmonwirelessdevice.h"
#include "cselect.h"
#include <pthread.h>

#include "cthread.h"
#include <condition_variable>

#include "cdynbuffer.h"

namespace ckernelwifi{

	class CallFromStaticFunc ;
}
class CKernelWifi : public intthread::AsyncTask {

	protected :

		pthread_t serverloop_id ;

		CSelect	Scheduler;
		bool 		_connected_to_server { false };
		std::mutex 	_mutex_connected_to_server ;
		std::condition_variable 	_cond_connected_to_server ;

		bool _being_initialized { false } ;

		bool _being_started { false } ;
		std::mutex _being_started_mutex ;

		WirelessDeviceList _list_winterfaces ;

		/** pointer for netlink socket */
		struct nl_sock * _netlink_socket { nullptr };

		/** pointer for netlink callback function */
		struct nl_cb * _cb { nullptr };

		/** For the family ID used by hwsim */
		int m_family_id { 1 };

		intthread::InterruptibleThread   hwsimloop_task ;
		intthread::InterruptibleThread   serverloop_task ;
 		intthread::InterruptibleThread   monitorloop_task ;
 		intthread::InterruptibleThread   winterface_update_loop_task ;
		intthread::InterruptibleThread   connection_to_server_loop_task ;

		bool _initialized { false } ;
		std::mutex _mutex_initialized ;

		MonitorWirelessDevice * monwireless = nullptr ;

		int init();
		int init_first();

		void mac_address_to_string(char *address, struct ether_addr *mac);

	public :

		static ckernelwifi::CallFromStaticFunc * forward ;

		/**
		* 	\brief Default Constructor
		*/
		CKernelWifi();

		/**
		* \brief Default Destructor
		*/
		~CKernelWifi();

		/**
		 * \brief start the all activity
		 */
		int start();

		/**
		 * \brief stop the all activity
		 */
		int stop();

		int process_messages(struct nl_msg *msg);

		/**
		 * \brief free dynamicly allocated memory and socket descriptors
		 */
		void clean_all();

		bool initialized();

		void connected_to_server(bool v);

		bool is_connected_to_server();

		bool reconnect_to_server() ;

		void manage_server_crash_loop() ;

		void being_started(bool v);

		bool is_being_started();

	protected :

		void cout_mac_address(struct ether_addr *src);

		/**
		 *	\brief Callback function to process messages received from kernel
		 *	It processes the frames received from hwsim via netlink messages.
		 *	These frames get sent via vsock to vwifi-server.
		 *	\param msg - pointer to netlink message
		 *	\param arg - pointer to additional args
		 *	\return success or failure
		 */
		static int process_messages_cb(struct nl_msg *msg, void *arg);

		/**
		*	\brief Send a register message to kernel via netlink
		*	This informs hwsim we wish to receive frames
		*	Taken from wmediumd
		*	\return void
		*/
		int send_register_msg();

		/**
		*	\brief Initialize netlink communications
		*	Taken from wmediumd
		*	\return void
		*/
		int init_netlink();
		int init_netlink_first();

		/**
		 * \brief start receiving hwsim netlink frame from hwsim driver
		 */
		void recv_msg_from_hwsim_loop_start();

		/**
		 *	\brief Send a tx_info frame to the kernel space. This frame indicates
		*	that the frame was transmitted/acked successfully. The ack is sent back
		*	to the driver with HWSIM_ATTR_ADDR_TRANSMITTER unmodified.
		*	This is derived form wmediumd.
		*	TODO: modify if we create more accurate acking.
		*	\param src - mac address of transmitting radio
		*	\param flags - falgs
		*	\param signal - signal strength
		 *	\param tx_attempts - number of transmit attempts
		*	\param cookie - unique identifier for frame
		*	\return success or failure
		*/
		int send_tx_info_frame_nl(struct ether_addr *src,
				unsigned int flags, int signal,
				struct hwsim_tx_rate *tx_attempts,
				unsigned long cookie);

		/**
		 * 	\brief  start receiving hwsim frame from vwifi-server loop
		 */
		void recv_msg_from_server_loop_start();

		static void recv_msg_from_server_signal_handle(int sig_num);

		/**
		 *	@brief this is meant to be a thread which detects removal of driver
		*	Used to suspend normal actions until driver is loaded
		*	@return void
		 */
		void  monitor_hwsim_loop();

		void winet_update_loop();

		/**
		*      \brief Send a cloned frame to the kernel space driver.
		*	This will send a frame to the driver using netlink.
		*	It is received by hwsim with hwsim_cloned_frame_received_nl()
		*	This is taken from wmediumd and modified. It is called after the
		*	message has been received from wmasterd.
		*	\param dst - mac address of receving radio
		*	\param data - frame data
		*	\param data_len - length of frame
		*	\param rate_idx - number of attempts
		*	\param signal - signal strength
		*	\param freq - frequency
		*	\return success or failure
		*/
		int send_cloned_frame_msg(struct ether_addr *dst, char *data, int data_len,int rate_idx, int signal, uint32_t freq);

		/**
		 * \brief handle messages received from server
		 */

		void recv_from_server();

		/**
		 * \brief callback from cmonitorwirelessdevice that is called to
		 * handle addding wireless inet
		 */
		void handle_new_winet_notification(WirelessDevice);

		/**
		 * \brief callback from cmonitorwirelessdevice that is called to
		 * handle deleting  wireless  inet
		 */
		void handle_del_winet_notification(WirelessDevice);

		/**
		 * \brief callback from cmonitorwirelessdevice that is called to
		 * handle initial wireless  inet
		 */
		void handle_init_winet_notification(WirelessDevice);

		/**
		 * \brief get permanent mac address of ifname interface
		 */
		bool get_pmaddr(struct ether_addr &,const char *ifname);

		/**
		 *\biref reconnecting to a server when detecting a socket disconnection
		 */
		void manage_server_crash();

	// virtual :

		virtual bool _Connect(int* id) = 0;

		virtual ssize_t _SendSignal(TPower* power, const char* buffer, int sizeOfBuffer) = 0;
		virtual ssize_t _RecvSignal(TPower* power, CDynBuffer* buffer) = 0;

		virtual void _Close() = 0;

};

/**
 * \namespace ckernelwifi
 *
 * A namespace is used here, since a class CallFromStaticFunc is defined in other files
 */
namespace ckernelwifi {

	/**
	 * \class CallFromStaticFunc
	 * \brief this class is an artifact used to call a member function from static function
	*/
	class CallFromStaticFunc {

	CKernelWifi * m_obj  ;

	public:

		explicit CallFromStaticFunc(CKernelWifi *   obj){

			m_obj = obj ;
		};

		int process_messages(struct nl_msg *msg) {

				// add exception to check null ptr
				m_obj->process_messages(msg);
				return 0 ;
		};
	};
}

#endif /* _CKERNELWIFI_H_ */

