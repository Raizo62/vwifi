# What is this ?

Simulate Wi-Fi (802.11) between Linux Virtual Machines on Qemu/VirtualBox/...

* It can be used in Hypervisors (GNS3, QEmu, Virtualbox, VMware, Hyper-V, ...)
* The Wireless emulator is based on mac80211_hwsim linux driver
* Implements the packet loss simulation based on distance
* Emulates the node mobility in GNS3

* Tested:
    * with `hostapd` and `wpa_supplicant` with these configurations:
       * Open
       * WEP
       * WPA2
       * WPA-EAP
    * in the context of WPA2 attack with `Aircrack-NG` (Kali / Parrot-OS)
    * with OpenWRT as AP

* Remaining features to be implemented:

    * Integrate to other OS (Windows...)
    * Add obstacle models
    * Etc.

![Example](./screenshots/GNS3_Attack_with_KaliLinux.png)

# Build

## On Debian-based Linux distributions

## Dependencies

```bash
sudo apt-get install make g++
sudo apt-get install libnl-3-dev libnl-genl-3-dev
```

## Building

```bash
make update # Not necessary. To download and update the file mac80211_hwsim.h
make
make tools # To change the file mode bits of tools

sudo make install
```

## On OpenWRT

* See the wiki : [Install-on-OpenWRT-X86_64](https://github.com/Raizo62/vwifi/wiki/Install-on-OpenWRT-X86_64)

# Configuration

Explanations :
* The VMs and the server can communicate either with the VHOST protocol, or with the TCP protocol
* The ***vwifi-server*** accepts connection from ***vwifi-guest*** with TCP or VHOST protocols
* ***vwifi-host*** receives always all communications, even if the lost of packets is enable. It works only with TCP.
* To use TCP protocol, the Host and the VMs must be connected to a different IP network than that of the wifi

## With VHOST

### Host

* Shell :
    * Load the module VHOST
   ```bash
   # sudo rmmod vhost_vsock vmw_vsock_virtio_transport_common vsock # if necessary

   sudo modprobe vhost_vsock
   sudo chmod a+rw /dev/vhost-vsock
   ```
    * Start the ***vwifi-server***
   ```bash
   vwifi-server
   ```

* Hypervisor
  * QEmu : add the option : `-device vhost-vsock-pci,id=vwifi0,guest-cid=NUM` with NUM an identifier greater than  2
  * GNS3 (>= 2.2) : QEmu : add the option : `-device vhost-vsock-pci,id=vwifi0,guest-cid=%guest-cid%`

### Each Guest

* Create the wlan interfaces (on this example, 2 interfaces) :
```bash
sudo modprobe mac80211_hwsim radios=2
# sudo macchanger -a wlan0 # we advice to change the MAC address of the wlan (with macchanger, ip, ifconfig, ...)
```

* Connect all these wlan interfaces to the ***vwifi-server***
```bash
sudo vwifi-guest
```

* ***vwifi-guest*** displays "ID=-1". ***vwifi-server*** uses the cid to identify this guest.

## With TCP

* The Host and the VMs must be connected to a different IP network than that of the wifi (for example : 172.16.0.0/16)

### Host

* Start the ***vwifi-server***

```bash
vwifi-server
```

* We will suppose that the Host have the IP address : 172.16.0.1

### Each Guest

* Create the wlan interfaces (on this example, 2 interfaces) :
```bash
sudo modprobe mac80211_hwsim radios=2
# sudo macchanger -a wlan0 # we advice to change the MAC address of the wlan (with macchanger, ip, ifconfig, ...)
```

* Connect all these wlan interfaces to the ***vwifi-server***
```bash
sudo vwifi-guest 172.16.0.1
```

* ***vwifi-guest*** displays an ID which is an hashsum of the IP. It is used by ***vwifi-server*** to identify this guest.

# Capture packets from Host

## Configure Host

```bash
sudo modprobe mac80211_hwsim radios=1
sudo vwifi-host
```

## Capture

### With tcpdump

* Capture from wlan0
```bash
sudo tcpdump -n -I -i wlan0
```

### With wireshark

* Configure wlan0 to monitor mode
```bash
sudo ip link set wlan0 down
sudo iw wlan0 set monitor control
sudo ip link set wlan0 up
```

* Start Wireshark and capture from wlan0
```bash
sudo wireshark
```

# Control

## Host

* Show the list of connected guest (display : cid and coordinate x, y z)
```bash
vwifi-ctrl ls
```

* Set the new coordinate (11, 12, 13) of the guest with the cid 10
```bash
vwifi-ctrl set 10 11 12 13
```

* Enable the lost of packets
```bash
vwifi-ctrl loss yes
```

* Disable the lost of packets
```bash
vwifi-ctrl loss no
```

* Display the config of ***vwifi-server***
```bash
vwifi-ctrl status
```

* Display the distance in meters between the guest with the cid 10 and the guest with the cid 20
```bash
vwifi-ctrl distance 10 20
```

# Test Wifi

## Test 1 : WPA

### Packages needed on the guests for this test

```bash
sudo apt install hostapd wpasupplicant
```

### Guests

* Guest Wifi 1 :

```bash
sudo ip a a 10.0.0.1/8 dev wlan0

sudo hostapd tests/hostapd_wpa.conf
```

* Guest Wifi 2 :
```bash
sudo wpa_supplicant -Dnl80211 -iwlan0 -c tests/wpa_supplicant.conf

sudo ip a a 10.0.0.2/8 dev wlan0
ping 10.0.0.1
```

* Guest Wifi 3 :
```bash
sudo wpa_supplicant -Dnl80211 -iwlan0 -c tests/wpa_supplicant.conf

sudo ip a a 10.0.0.3/8 dev wlan0
ping 10.0.0.2
```

## Test 2 : Open

### Packages needed on the guests for this test

```bash
sudo apt install hostapd iw tcpdump
```

### Guests

* Guest Wifi 1 :

```bash
sudo ip a a 10.0.0.1/8 dev wlan0

sudo hostapd tests/hostapd_open.conf
```

* Guest Wifi 2 :
```bash
sudo ip link set up wlan0
sudo iw dev wlan0 connect mac80211_open

sudo ip a a 10.0.0.2/8 dev wlan0
ping 10.0.0.1
```

* Guest Wifi 3 :
```bash
sudo ip link set up wlan0
sudo tcpdump -n -e -I -i wlan0 -w /hosthome/projects/vwifi_capture_wlan0.pcap
```

### Host

```bash
tail -f -c +0b /home/user/projects/vwifi_capture_wlan0.pcap | wireshark -k -i -
```

## Test 3 : Ad-Hoc

### Packages needed on the guests for this test

```bash
sudo apt install iw
```

### Guests

* Guest Wifi 1 :
```bash
sudo ip link set up wlan0
sudo iw wlan0 set type ibss
sudo iw wlan0 ibss join MYNETWORK 2412 # frequency 2412 is channel 1

sudo ip a a 10.0.0.1/8 dev wlan0
```

* Guest Wifi 2 :
```bash
sudo ip link set up wlan0
sudo iw wlan0 set type ibss
sudo iw wlan0 ibss join MYNETWORK 2412 # frequency 2412 is channel 1

sudo ip a a 10.0.0.2/8 dev wlan0
ping 10.0.0.1
```

# Others Tools

* start-vwifi-guest.sh : do all the commands necessary to start ***vwifi-guest*** on a Guest
* fast-vwifi-update.sh : set with ***vwifi-ctrl*** the coordinates of each VMs which has the option `guest-cid=`, found in the open project of GNS3
* client.sh : configure the client wifi with Open or WPA
