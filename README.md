# bls-therapy-device
note: for local testing via phone, you need to use `mkcert -install` to install certificates so you can have https when serving the website over 192.168.1.x to your phone. If you dont do it then you cant use camera functionality in the website(its not allowed for non https.)

after doing `mkcert -install`, you need to create a new folder called certs `mkdir certs` and then run this:

`mkcert [your device's local ip starting with 192.168] localhost 127.0.0.1 ::1`

then rename the files to be:
`mv [192.168.1.x]+3.pem certs/dev-cert.pem`
`mv [192.168.1.x]+3-key.pem certs/dev-key.pem`

vite is already configured to use the certs.

NOTE: you need to go to **https://192.168.1.x:5173** not **http://192.168.1.x:5173**. http:// wont work. 

NOTE2: Brave wont work, use chrome(chromium) with no adblocker to be sure it works properly.

the same applies to localhost, you need to go to https://localhost:5173 not http://localhost:5173.


## I cant flash / get serial output etc.
- enable usb cdc on boot
- check your ports(change if necessary)
- check if you selected the right board(esp32c3)
- install the usb serial driver (forgot what its called)
- use different esp32c3
- use a simple sketch to see whats problematic etc.

## Arduino IDE Warning
File -> Preferences -> Compiler Warnings: Set it to `all`.

Also enable verbose output during compilation if you want.

If you want some debug info during runtime go to tools -> Core debug level: Set it to any level you want.


getting local network ip:
use `ipconfig` to get ip address on windows(ipv4 address)
use `ifconfig` to get ip address on linux



`idf.py build`

to get idf.py, you need to call `getidf` --> alias for export.sh in esp-idf folder.

available commands:

```bash
getidf
idf.py set-target esp32c3
idf.py build
idf.py flash
```

ls -lZ /dev/ttyACM0
idf.py -p /dev/ttyACM0 set-target esp32c3 menuconfig
sudo chmod a+rw /dev/ttyACM0
ls /dev/tty*
dmesg -w

idf.py -p /dev/ttyACM0 flash

## permission fix for reading /dev/ttyACM stuff
sudo chmod a+rw /dev/ttyACM0  --> temporary permission fix
