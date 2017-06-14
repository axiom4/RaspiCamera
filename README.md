RaspiCamera
======

## Index

1. [Install Raspbian](#1--install-raspbian)
2. [Configure Hostspot](#2-configure-hostspot)
3. [Install gphoto2](#3-install-gphoto2)
4. [Configure WebServer](#4-configure-webserver)
5. [Deploy RaspiCamera](#5-deploy-raspicamera)

# 1.  Install Raspbian

[Follow this link to raspberrypi.org!](https://www.raspberrypi.org/documentation/installation/installing-images/README.md)

# 2. Configure Hostspot

1. Intall *dnsmasq* and *hostapd*

```
# sudo apt-get install dnsmasq hostapd
```

2. Config */etc/network/interface*

Remove this:

```
# Remove
iface wlan0 inet manual
    wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf
```

Add this:

```
# Add lines
iface wlan0 inet static
    address 192.168.0.1
    netmask 255.255.255.0
    network 192.168.0.0
```

3. Create */etc/dnsmasq.conf*

```
interface=wlan0      # Use the require wireless interface - usually wlan0
  dhcp-range=192.168.0.2,192.168.0.20,255.255.255.0,24h
```

4. Create */etc/hostapd/hostapd.conf*

```
interface=wlan0
driver=nl80211
ssid=rpicamera
hw_mode=g
channel=7
wmm_enabled=0
macaddr_acl=0
auth_algs=1
ignore_broadcast_ssid=0
wpa=2
wpa_passphrase=[your wifi password]
wpa_key_mgmt=WPA-PSK
wpa_pairwise=TKIP
rsn_pairwise=CCMP
```

5. Modify */etc/default/hostapd*

```
# Add this line
DAEMON_CONF="/etc/hostapd/hostapd.conf"
```

6. Modify */etc/dhcpcd.conf*

```
# Add this line
denyinterfaces wlan0
```

7. Restart System

```
# sudo reboot
```

8. Join to *rpicamera* wifi network

# 3. Install gphoto2

TODO

# 4. Configure WebServer

TODO

# 5. Deploy RaspiCamera

TODO
