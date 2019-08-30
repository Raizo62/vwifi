# vwifi

## What is this?

Virtual Wifi between Virtual Machines

## Dependencies

```bash
sudo apt-get install build-essential
sudo apt-get install libnl-3-dev libnl-genl-3-dev
```

## Building

```bash
make update # Not necessary. To download and update the file mac80211_hwsim.h
make
```

## Configuration

### Host

 - Linux :

```bash
sudo rmmod vhost_vsock vmw_vsock_virtio_transport_common vsock

sudo modprobe vhost_vsock
sudo chmod a+rw /dev/vhost-vsock

./vwifi-server
```

 - QEmu : add the option : `-device vhost-vsock-pci,id=vwifi0,guest-cid=NUM` with NUM an identifier greater than  2
 - GNS3 (>= 2.2) : add the option : `-device vhost-vsock-pci,id=vwifi0,guest-cid=%guest-cid%`

### Each Guest

```bash
cd /hosthome/vwifi
tmux
modprobe mac80211_hwsim radios=3
macchanger -a wlan0
./vwifi-guest
```

## Test Wifi

### Test 1 : WPA

#### Packages needed on the guests for this test

```bash
sudo apt install hostapd wpasupplicant
```

#### Guests

* Guest Wifi 1 :

```bash
ip a a 10.0.0.1/8 dev wlan0

hostapd tests/hostapd_wpa.conf
```

* Guest Wifi 2 :
```bash
wpa_supplicant -Dnl80211 -iwlan0 -c tests/wpa_supplicant.conf

ip a a 10.0.0.2/8 dev wlan0
ping 10.0.0.1
```

* Guest Wifi 3 :
```bash
wpa_supplicant -Dnl80211 -iwlan0 -c tests/wpa_supplicant.conf
ip a a 10.0.0.3/8 dev wlan0
ping 10.0.0.2
```

### Test 2 : Open

#### Packages needed on the guests for this test

```bash
sudo apt install hostapd iw tcpdump
```

#### Guests

* Guest Wifi 1 :

```bash
ip a a 10.0.0.1/8 dev wlan0

hostapd tests/hostapd_open.conf
```

* Guest Wifi 2 :
```bash
ip link set up wlan0
iw dev wlan0 connect mac80211_open

ip a a 10.0.0.2/8 dev wlan0
ping 10.0.0.1
```

* Guest Wifi 3 :
```bash
ip link set up wlan0
tcpdump -n -e -I -i wlan0 -w /hosthome/projects/vwifi_capture_wlan0.pcap
```

#### Host

```bash
tail -f -c +0b /home/user/projects/vwifi_capture_wlan0.pcap | wireshark -k -i -
```

### Test 3 : Ad-Hoc

#### Packages needed on the guests for this test

```bash
sudo apt install iw
```

#### Guests

* Guest Wifi 1 :
```bash
ip link set up wlan0
iw wlan0 set type ibss
iw wlan0 ibss join MYNETWORK 2412 # frequency 2412 is channel 1

ip a a 10.0.0.1/8 dev wlan0
```

* Guest Wifi 2 :
```bash
ip link set up wlan0
iw wlan0 set type ibss
iw wlan0 ibss join MYNETWORK 2412 # frequency 2412 is channel 1

ip a a 10.0.0.2/8 dev wlan0
ping 10.0.0.1
```

## Control

### Host

* Show the list of connected guest (display : cid and coordinate x, y z)
```bash
./vwifi-ctrl ls
```

* Set the new coordinate (11, 12, 13) of the guest with the cid 10
```bash
./vwifi-ctrl 10 11 12 13
```

## Tools

* start-vwifi-guest.sh : do all the commands necessary to start `vwifi-guest` on a Guest
* fast-vwifi-update.sh : set with `vwifi-ctrl` the coordinates of each VMs which has the option `guest-cid=`, found in the open project of GNS3
* client.sh : configure the client wifi with Open or WPA
