sudo apt-get install libnl-3-dev libnl-genl-3-dev
sudo apt-get install pkg-config

gcc $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0) 


gcc vwifi.c nodes.c $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0) -o vwifi

sudo wpa_supplicant -Dnl80211 -iwlan1 -c wpa_supplicant.conf


sudo hostapd hostapd.conf


g++ vwifi-guest.cc cvwifiguest.cc $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0) -o vwifi -lpthread -DDEBUG
