#!/bin/bash

MODE="d" # dhcp , s : static
INET="wlan1"
CONN="o" # wpa : w , open : o

WPA_CONF_FILE_PATH="tests/wpa_supplicant.conf"
IP="10.0.0.2/8" 

help(){

echo "Help : $0 -m s|d -c o|w  interface IP_STATIC/MASQUE"

echo " -m : IP Configuration mode"
echo "      d : dhcp "
echo "      s : static"

echo " -c : Connection mode"
echo "      o : open "
echo "      w : wpa"


}



if (( $# == 0 ))
then

	help
	exit 1

fi



while getopts "m:c:" option ; do

case $option in 

m)      MODE="${OPTARG}"
	;;

c)      CONN="${OPTARG}"
	;;

*) 	help
	;;

esac

done

shift $(($OPTIND - 1)) 

INET=$1 
IP="$2" 


#######################

if [ $CONN == "w" ] ; then

	wpa_supplicant -Dnl80211 -i ${INET} -c ${WPA_CONF_FILE_PATH} & 

else
	ip link set up ${INET}
	iw dev ${INET} connect mac80211_open


fi

sleep 3 

if [ ${MODE} == "s" ] ; then

	ip a a ${IP} dev wlan0
else
	dhclient -v -i ${INET}

fi


