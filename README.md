RaspiCamera
======

## Index

1. [Install Raspbian](#1--install-raspbian)
2. [Configure Hostspot](#2-configure-hostspot)
3. [Install gphoto2](#3-install-gphoto2)
4. [Install libusb-1.0](#4-install-libusb-10)
5. [Configure WebServer](#5-configure-webserver)
6. [Deploy RaspiCamera](#6-deploy-raspicamera)

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

1. Updating sources

```
# sudo apt-get update -qq
```

2. Removing gphoto2 and libgphoto2 if exists

```
# sudo apt-get remove -y gphoto2 libgphoto2*
```

3. Installing dependencies

```
# sudo apt-get install -y build-essential libltdl-dev libusb-dev libexif-dev libpopt-dev libudev-dev pkg-config git automake autoconf autopoint gettext libtool wget libgd-dev libxml2-dev libreadline-dev libcdk5-dev 
```

4. Create arch binary home

As user *pi*

```
# cd $HOME
# mkdir $HOME/arch/bin 
# mkdir $HOME/arch/lib 
```

Edit *$HOME/.profile

```
# Append this lines
# set PATH so it includes user's private arch/bin if it exists
if [ -d "$HOME/arch/bin" ] ; then
    PATH="$HOME/arch/bin:$PATH"
fi
```

Create */etc/ld.so.conf.d/pihome.conf*

```
sudo echo "/home/pi/arch/lib" > /etc/ld.so.conf.d/pihome.conf
```

**Logoff and re-login to system**

5. Compiling and installing libgphoto2

```
# git clone --branch libgphoto2-2_5_14-release https://github.com/gphoto/libgphoto2.git
# cd libgphoto2/
# autoreconf --install --symlink
# ./configure --prefix=$HOME/arch
# make
# make install
# cd ..
```

6. Compiling and installing gphoto2

```
# git clone --branch gphoto2-2_5_14-release https://github.com/gphoto/gphoto2.git
# cd gphoto2
# autoreconf --install --symlink
# CFLAGS=$(gphoto2-config --cflags) LDFLAGS=$(gphoto2-config --libs) ./configure --prefix=$HOME/arch --with-libgphoto2=$HOME/arch
# make
# make install
# cd ..
```

7. Update libraries cache

```
# sudo ldconfig
```

8. Generating udev rules

```
# sudo $HOME/arch/lib/libgphoto2/print-camera-list udev-rules version 201 group plugdev mode 0660 > /etc/udev/rules.d/90-libgphoto2.rules
# sudo $HOME/arch/lib/libgphoto2/print-camera-list hwdb > /etc/udev/hwdb.d/20-gphoto.hwdb
```

9. Testing Gphoto2

```
# gphoto2 --version
```

# 4. Install libusb-1.0

1. Compiling and installing libusb-1.0

```
# git clone --branch v1.0.21 https://github.com/libusb/libusb.git
# cd libusb
# autoreconf --install --symlink
# ./configure --prefix=$HOME/arch --enable-system-log --enable-udev
# make
# make install
# cd ..
```

# 5. Configure WebServer

TODO

# 6. Deploy RaspiCamera

TODO
