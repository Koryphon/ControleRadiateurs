# Setup of the Raspberry Pi Zero WH

This document describes the configuration and deployment of the Raspberry Pi Zero W or WH for hosting the MQTT broker and heater control software.

The Rasberry Pi Zero WH board, as well as a Micro-SD card already loaded with the Raspbian Linux operating system, have been purchased from [Go Tronic](https://www.gotronic.fr).

## WiFi connection and SSH server activation

The goal here is to do without a screen and keyboard, which avoids buying a mini-HDMI cable and a Micro-USB to USB female cable. The configuration is quite simple. 

The first thing is to put a configuration file for the WiFi connection, called ```wpa_supplicant.conf```, at the root of the Micro-SD card:

```conf
country=FR
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
network={
  scan_ssid=1
  ssid="<your ssid>"
  psk="<your pass>"
  key_mgmt=WPA-PSK
}
```

Of course, ```<your ssid>``` and ```<your pass>``` must contain the WiFi credentials for your WiFi network.

The second thing is to activate ```ssh```. This just requires creating an empty file that we will simply call ```ssh``` at the root of the Micro-SD card:

```
touch /Volumes/boot/ssh
```

On reboot, the RPi should connect to your WiFi network and ssh should be active.

```
c++ -o ctrlheaters -std=c++17 -Wno-psabi *.cpp -lmosquittopp -lpthread
```

It seems that the original version of mosquitto on Raspberry Pi (at least with version 2.0.11 available at the time of writing) is problematic. Indeed, the PubSubClient library can't connect. So we have to compile a more recent version of Mosquitto, 2.0.14, and integrate it as a service on the Raspberry Pi. Mosquitto is easy to compile. You just need to install the missing packages and especially ```libsystemd-dev``` on one hand and to edit the ```config.mk``` file for, on line 64, change ```no``` to ```yes```:

```sh
# Build with systemd support. If enabled, mosquitto will notify systemd after                                                                            
# initialization. See README in service/systemd/ for more information.                                                                                   
# Setting to yes means the libsystemd-dev or similar package will need to be                                                                             
# installed.                                                                                                                                             
WITH_SYSTEMD:=yes
```

A simple make then does the trick. On Raspberry Pi Zero, this takes some time.

Since the Raspberry Pi Zero will be put un a box, it could be a good idea to monitor its temperature ti see if the cooling is ok. This can be done by the command

```
cat /sys/class/thermal/thermal_zone0/temp
```

that returns the millidegree celsius. Do not dream, the resolution is of the order of half a degree.