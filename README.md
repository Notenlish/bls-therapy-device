# bls-therapy-device
 
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
