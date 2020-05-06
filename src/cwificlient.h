#ifndef _CWIFICLIENT_H_
#define _CWIFICLIENT_H_

#include "hwsim.h"

#include <string>
#include <mutex>

#include "csocketclient.h"
#include "cwirelessdevice.h"
#include "cwirelessdevicelist.h"
#include "cmonwirelessdevice.h"
#include "cscheduler.h"
#include <pthread.h>

#include "cthread.h"
#include <condition_variable>

namespace cwificlient{

	class CallFromStaticFunc ;
}
class CBaseWifiClient : public intthread::AsyncTask {	

	protected :

		 
		pthread_t serverloop_id ;

		CScheduler	Scheduler;

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


		bool _initialized { false } ;
		std::mutex _mutex_initialized ;
	
		MonitorWirelessDevice * monwireless = nullptr ;

		int init();

		void mac_address_to_string(char *address, struct ether_addr *mac);

	public :


		static cwificlient::CallFromStaticFunc * forward ;

		/**
		* 	\brief Default Constructor
		*/
		CBaseWifiClient();


		/**
		* \brief Default Destructor
		*/
		~CBaseWifiClient();

		/**
		 * \brief start the all activity
		 */
		int start();

		/**
		 * \brief stop the all activity
		 */
		int stop();

		int process_messages(struct nl_msg *msg, void *arg);

		/**
		 * \brief free dynamicly allocated memory and socket descriptors
		 */
		void clean_all();

		bool initialized();

	protected :

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

		/**
		 * \brief start receiving hwsim netlink frame from hwsim driver
		 */
		void recv_msg_from_hwsim_loop_start();


		/**
		*	\brief Set a tx_rate struct to not valid values
		*	Taken from wmediumd.
		*	TODO: modify if we ack more accurately.
		*	\param tx_rate - struct to tract attempts and rates
		*	\return void
		*/
		void set_all_rates_invalid(struct hwsim_tx_rate *tx_rate);

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
		 * \brief hadle messages received from server
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
		 *\biref reconnecting to a server when detecting a socket disconncetion (read or write return -2 (SOCKET_DISCONNECT))
		 */
		void manage_server_crash();

	// virtual :

		virtual bool Connect()=0;

		virtual ssize_t Send(const char* data, ssize_t sizeOfData)=0;
		virtual ssize_t SendBigData(const char* data, ssize_t sizeOfData)=0;

		virtual ssize_t Read(char* data, ssize_t sizeOfData)=0;
		virtual ssize_t ReadBigData(char* data, ssize_t sizeOfData)=0;

		virtual void StopReconnect(bool status)=0;

		virtual void Close()=0;

};

template <typename TypeCSocketClient>
class CWifiClient : public CBaseWifiClient, public TypeCSocketClient
{
		bool Connect() {
			if ( ! TypeCSocketClient::Connect() )
				return false;
			return Scheduler.AddNode(*this);
		};

		ssize_t Send(const char* data, ssize_t sizeOfData){ return TypeCSocketClient::Send(data, sizeOfData); };
		ssize_t SendBigData(const char* data, ssize_t sizeOfData){ return TypeCSocketClient::SendBigData(data, sizeOfData); };

		ssize_t Read(char* data, ssize_t sizeOfData){ return TypeCSocketClient::Read(data, sizeOfData); };
		ssize_t ReadBigData(char* data, ssize_t sizeOfData){ return TypeCSocketClient::ReadBigData(data, sizeOfData); };

		void StopReconnect(bool status){ TypeCSocketClient::StopReconnect(status); };

		void Close(){ TypeCSocketClient::Close(); };
};

/**
 * \namespace cwificlient
 *
 * A namespace is used here, since a class CallFromStaticFunc is defined in other files
 */
namespace cwificlient {

	/**
	 * \class CallFromStaticFunc
	 * \brief this class is an artifact used to call a member function from static function
	*/
	class CallFromStaticFunc {

	CBaseWifiClient * m_obj  ;

	public:

		CallFromStaticFunc(CBaseWifiClient *   obj){

			m_obj = obj ;
		};

		int process_messages(struct nl_msg *msg, void *arg) {

				// add exception to check null ptr
				m_obj->process_messages(msg , arg);
				return 0 ;
		};
	};
}


#endif /* _CWIFICLIENT_H_ */


