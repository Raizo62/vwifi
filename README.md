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
make
```

* Old method :

```bash
sudo apt-get install pkg-config

gcc $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0)
gcc vwifi.c nodes.c $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0) -o vwifi
g++ vwifi-guest.cc cvwifiguest.cc $(pkg-config --cflags --libs libnl-3.0 libnl-genl-3.0) -o vwifi -lpthread -DDEBUG
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
 - GNS3 : add the option : `-device vhost-vsock-pci,id=vwifi0,guest-cid=%console-port%`

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

#### Guests

* Guest Wifi 1 :

```bash
hostapd tests/hostapd_wpa.conf
ip a a 10.0.0.1/8 dev wlan0
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

#### Guests

* Guest Wifi 1 :

```bash
hostapd tests/hostapd_open.conf
ip a a 10.0.0.1/8 dev wlan0
```

* Guest Wifi 2 :
```bash
ip link set up wlan0
iw wlan0 scan
iw dev wlan0 connect mac80211_open
ip a a 10.0.0.2/8 dev wlan0
ping 10.0.0.1
```

* Guest Wifi 3 :
```bash
iwconfig wlan0 mode Monitor
ip link set up wlan0
tcpdump -n -i wlan0 -w /hosthome/projects/vwifi_capture_wlan0.pcap
```

* Host :
```bash
tail -f -c +0b /home/user/projects/vwifi_capture_wlan0.pcap | wireshark -k -i -
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

## Others Tools

* nc-vsock : https://github.com/stefanha/nc-vsock
```bash
wget https://github.com/stefanha/nc-vsock/archive/master.zip
cd nc-vsock-master
make
```
