
class MonitorNetDevice {


	int start();
	int stop();


	/*
	 * @brief returns a list of existing wireless network interfaces 
	 */
	std::list<WirelessDevice> &  get_wireless_devices_list();



}
