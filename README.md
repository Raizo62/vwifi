# vwifi

## What is this?

Virtual Wifi between Virtual Machines

## Dependencies

    $ sudo apt-get install build-essential
    $ sudo apt-get install libnl-3-dev libnl-genl-3-dev
    $ sudo apt-get install pkg-config

## Building

    $ make

    $ gcc $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)
    $ gcc vwifi.c nodes.c $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0) -o vwifi
    $ g++ vwifi-guest.cc cvwifiguest.cc $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0) -o vwifi -lpthread -DDEBUG

## Configuration

### Host

    $ sudo rmmod vhost_vsock vmw_vsock_virtio_transport_common vsock

    $ sudo modprobe vhost_vsock
    $ sudo chmod a+rw /dev/vhost-vsock

* QEmu : add the option : -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=NUM		NUM an identifier > 2
* GNS3 : add the option : -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=%console-port%

    $ ./vwifi-host-server

### Guest

    $ cd /hosthome/vwifi
    $ tmux
    $ modprobe mac80211_hwsim radios=2
    $ ./vwifi-guest

#### Test Wifi

* Guest Wifi 1
    $ hostapd tests/hostapd.conf
    $ ip a a 10.0.0.1/8 dev wlan0
* Guest Wifi 2
    $ wpa_supplicant -Dnl80211 -iwlan1 -c tests/wpa_supplicant.conf
    $ ip a a 10.0.0.2/8 dev wlan1

    $ sudo wpa_supplicant -Dnl80211 -iwlan1 -c wpa_supplicant.conf
    $ sudo hostapd hostapd.conf

## Tools

* nc-vsock : https://github.com/stefanha/nc-vsock
    $ wget https://github.com/stefanha/nc-vsock/archive/master.zip
    $ cd nc-vsock-master
    $ make
