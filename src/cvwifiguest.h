#ifndef _CVWIFIGUEST_H_
#define _CVWIFIGUEST_H_

#include "hwsim.h"

#include <string>
#include <mutex>

#include "csocketclient.h"

class CallFromStaticFunc ;

class VWifiGuest {


/** pointer for tcp or vsocket */
CSocketClient  _socket ;

/** pointer for netlink socket */
struct nl_sock * m_sock { nullptr };

/** pointer for netlink callback function */
struct nl_cb * m_cb { nullptr };

/** For the family ID used by hwsim */
int m_family_id { 1 };


bool m_initialized { false } ;
bool m_started  { false } ;

std::mutex m_mutex_ctrl_run ;
std::mutex m_mutex_init ;

void mac_address_to_string(char *address, struct ether_addr *mac);

public :


	static CallFromStaticFunc * forward ;
	
	/**
 	* 	@brief Default Constructor
 	*/	
	VWifiGuest();

	
	/**
 	* @brief Default Destructor
 	*/	
	~VWifiGuest();

	/**
	 * @brief start the all activity
	 */
	int start();

	/**
	 * @brief stop the all activity
	 */
	int stop();

	int process_messages(struct nl_msg *msg, void *arg);


private:
	
	/**
	 *	@brief Callback function to process messages received from kernel
	 *	It processes the frames received from hwsim via netlink messages.
	 *	These frames get sent via vsock to vwifi-server.
	 *	@param msg - pointer to netlink message
	 *	@param arg - pointer to additional args
	 *	@return success or failure
	 */	
	static int process_messages_cb(struct nl_msg *msg, void *arg);
	
	/**
 	*	@brief Send a register message to kernel via netlink
 	*	This informs hwsim we wish to receive frames
 	*	Taken from wmediumd
 	*	@return void
 	*/
	int send_register_msg();

	/**
 	*	@brief Initialize netlink communications
 	*	Taken from wmediumd
 	*	@return void
 	*/
	int init_netlink();

	/**
	 * 	@ start receiving hwsim netlink frame from hwsim driver
	 */
	void recv_msg_from_hwsim_loop_start();

	/**
 	*	@brief Set a tx_rate struct to not valid values
 	*	Taken from wmediumd.
 	*	TODO: modify if we ack more accurately.
 	*	@param tx_rate - struct to tract attempts and rates
 	*	@return void
 	*/
	void set_all_rates_invalid(struct hwsim_tx_rate *tx_rate);

	/**
	 *	@brief Send a tx_info frame to the kernel space. This frame indicates
 	*	that the frame was transmitted/acked successfully. The ack is sent back
 	*	to the driver with HWSIM_ATTR_ADDR_TRANSMITTER unmodified.
 	*	This is derived form wmediumd.
 	*	TODO: modify if we create more accurate acking.
 	*	@param src - mac address of transmitting radio
 	*	@param flags - falgs
 	*	@param signal - signal strength
	 *	@param tx_attempts - number of transmit attempts
 	*	@param cookie - unique identifier for frame
 	*	@return success or failure
 	*/
	int send_tx_info_frame_nl(struct ether_addr *src,
			unsigned int flags, int signal,
			struct hwsim_tx_rate *tx_attempts,
			unsigned long cookie);

	/**
	 * 	@brief  start receiving hwsim frame from vwifi-server loop
	 */
	void recv_msg_from_server_loop_start();

	/**
	 * @brief hadle messages received from server
	 */

	void recv_from_server();

	/**
	 * @brief free dynamicly allocated memory and socket descriptors
	 */
	void clean_all();

	
	/**
	 * @brief set m_initialized member using m_mutex_init 
	 */
	void setInitialized(int);


	/**
	 * @brief check the value  m_initialized member using m_mutex_init 
	 */
	bool check_if_netlink_initialized();
	
	
	/**
	 * @brief check the value  m_started member using m_mutex_ctr_run 
	 */
	bool check_if_started();


};

class CallFromStaticFunc {

	VWifiGuest * m_obj  ;

	public:
	

		CallFromStaticFunc(VWifiGuest *   obj){
		
			
			m_obj = obj ;
		
		};

		int process_messages(struct nl_msg *msg, void *arg) {
		
				// add exception to check null ptr
				m_obj->process_messages(msg , arg);
				return 0 ;
		};


};





#endif /* _CVWIFIGUEST_H_ */


