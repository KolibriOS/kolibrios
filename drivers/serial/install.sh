#SHS
echo Installing serial driver for kterm...
cp /kolibrios/utils/kterm/serial.sys /sys/drivers/
/sys/loaddrv serial
echo Serial driver successfully installed!
