/**
 * \file cwirelessdevicelist.h
 * \brief manage std::list of WirelessDevice objects
 * \author 
 * \version
 */

#ifndef _CWIRELESSDEVICELIST_H_
#define _CWIRELESSDEVICELIST_H_

#include <map>
#include <mutex>
#include <vector>

#include "cwirelessdevice.h"

/**
 * \class WirelessDeviceList
 * \brief This class represents a std::list of WirelessDevice objects
 */
class WirelessDeviceList {

	/**
	 * \brief manage threads access to wdevices_list 
	 */
	std::mutex _listaccess ;

	/**
	 * \brief list of existing wireless network interfaces 
	 */
	std::map<int,WirelessDevice>  _wdevices_list;

	public:

	WirelessDeviceList();
	~WirelessDeviceList();

	/**
	 * \fn add_device(int)
	 * \brief Add to _wdevices_list a wireless network device
	 * \param wdevice  Wireless network device
	 * \return void  
	 */
	void add_device(const WirelessDevice & wdevice);
	
	/**
	 * \fn delete_device(int)
	 * \brief Remove from _wdevice_list a wireless network device by index
	 * \param index Ifindex of wireless network device
	 * \return void  
	 */
	void delete_device(int index);

	/**
	 * \fn list_devices()
	 * \brief Get a list of all wireless network devices
	 * \return std::vector List of all devices    
	 */
	std::vector<WirelessDevice> & list_devices() ;

	/**
	 * \brief << operator overidden 
	 */
	friend std::ostream &   operator<< ( std::ostream & , WirelessDeviceList &);



};

#endif
