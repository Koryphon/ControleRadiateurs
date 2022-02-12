### How to recover from setup problem

In my case, after I connected to the Omega access point and entered my Wifi network credentials, the installation procedure performed a firmware update. It then remained inactive. After a while I rebooted it manually but it did not connect to my WiFi network.

To get WiFi back up and running, I had to connect via the serial line. Once I plugged in my Mac, ``/dev/tty.usbserial-0001`` appeared. The following command connects to the Omega:

```sh
screen /dev/tty.usbserial-0001 115200
```

To exit from the ```screen```: ```ctrl-a``` then ```k```


### How to change hostname

The default name of the Omega, the one that appears in the terminal prompt and the one used as the Bonjour name, can be changed as follows:

```sh
uci set system.@system[0].hostname='yourPreferedName'
uci commit
reboot now
```

### How to change the access point name and the password

The access point characteristics of the Omega can be changed with the following command:

```sh
wifisetup -ap edit -ssid <your ssid> -encr psk2 -password <your pass>
```
