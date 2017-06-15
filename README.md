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

1. Updating sources

```
apt-get update -qq
```

2. Removing gphoto2 and libgphoto2 if exists

```
apt-get remove -y gphoto2 libgphoto2*
```

3. Installing dependencies

```
apt-get install -y build-essential libltdl-dev libusb-dev libexif-dev libpopt-dev libudev-dev pkg-config git automake autoconf autopoint gettext libtool wget
```

4. Creating temporary folder

```
mkdir gphoto2-temp-folder
cd gphoto2-temp-folder
```

5. Downloading libgphoto2

```
git clone --branch  libgphoto2-2-2_5_14-release https://github.com/gphoto/libgphoto2.git
cd libgphoto2/
```

6. Compiling and installing libgphoto2

```
autoreconf --install --symlink
./configure
make -j "$cores"
make install
cd ..
```

7. Downloading gphoto2

```
git clone  gphoto2-2-2_5_14-release https://github.com/gphoto/gphoto2.git
cd gphoto2
```

8. Compiling and installing gphoto2

```
autoreconf --install --symlink
./configure
make -j "$cores"
make install
cd ..
```

9. Linking libraries

```
ldconfig
```

10. Generating udev rules, see http://www.gphoto.org/doc/manual/permissions-usb.html

```
udev_version=$(udevd --version)

if   [ "$udev_version" -ge "201" ]
then
  udev_rules=201
elif [ "$udev_version" -ge "175" ]
then
  udev_rules=175
elif [ "$udev_version" -ge "136" ]
then
  udev_rules=136
else
  udev_rules=0.98
fi

/usr/local/lib/libgphoto2/print-camera-list udev-rules version $udev_rules group plugdev mode 0660 > /etc/udev/rules.d/90-libgphoto2.rules

if   [ "$udev_rules" = "201" ]
then
  echo
  echo "------------------------------------------------------------------------"
  echo "Generating hwdb file in /etc/udev/hwdb.d/20-gphoto.hwdb. Ignore the NOTE"
  echo "------------------------------------------------------------------------"
  echo
  /usr/local/lib/libgphoto2/print-camera-list hwdb > /etc/udev/hwdb.d/20-gphoto.hwdb
fi
```


11. Removing temp files

```
cd ..
rm -r gphoto2-temp-folder
```

12. Finished!! Enjoy it!

```
gphoto2 --version
```

# 4. Configure WebServer

TODO

# 5. Deploy RaspiCamera

TODO
