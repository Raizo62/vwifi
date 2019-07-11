#!/bin/bash

cd /hosthome/vwifi

if (( $# == 0 ))
then
	NbrWifi=2
else
	NbrWifi="$1"
fi

modprobe mac80211_hwsim radios=$NbrWifi

for((i=0;i<NbrWifi;i++))
do
        macchanger -a wlan$i
done

if [ -v TMUX ]
then
	# Already in tmux
	./vwifi-guest
else
	tmux new-session -s vwifi "./vwifi-guest" ; detach &> /dev/null

	tmux attach -t vwifi
fi
