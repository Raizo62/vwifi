#!/bin/bash

DEFAULT_PREFIX_MAC_ADDRESS="74:F8:DB"

cd /hosthome/vwifi

if (( $# == 0 ))
then
	NbrWifi=2
else
	NbrWifi="$1"
fi

if [ -e /sys/module/mac80211_hwsim ]
then
	OldNbrWifi=$(cat /sys/module/mac80211_hwsim/parameters/radios)
	if (( NbrWifi != OldNbrWifi ))
	then
		rmmod mac80211_hwsim
	fi
fi

modprobe mac80211_hwsim radios=$NbrWifi

hexchars="0123456789ABCDEF"
for((i=0;i<NbrWifi;i++))
do
	middle=$( for i in {1..4} ; do echo -n ${hexchars:$(( $RANDOM % 16 )):1} ; done | sed -e 's/\(..\)/:\1/g' )
	MAC_ADDRESS="${DEFAULT_PREFIX_MAC_ADDRESS}${middle}:$(printf "%02d" $i)"
	echo "wlan$i : ${MAC_ADDRESS}"
	ip link set wlan$i addr ${MAC_ADDRESS}
done

if [ -v TMUX ]
then
	# Already in tmux
	./vwifi-guest
else
	tmux new-session -s vwifi "./vwifi-guest" ; detach &> /dev/null

	tmux attach -t vwifi
fi
