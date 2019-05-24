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

* QEmu : add the option : -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=NUM		NUM an identifier > 2
* GNS3 : add the option : -device vhost-vsock-pci,id=vhost-vsock-pci0,guest-cid=%console-port%

## Test Wifi

    $ sudo wpa_supplicant -Dnl80211 -iwlan1 -c wpa_supplicant.conf
    $ sudo hostapd hostapd.conf
