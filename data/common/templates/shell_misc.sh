#SHS

echo Hello, I am a command language interpreter example.
ver
sleep 200


echo 
echo ==============
echo runing palitra
echo ==============

/sys/media/palitra H 00AEC3D8 003A95BF
sleep 300
/sys/media/palitra H 007DCEDF 003C427F
sleep 200
/sys/media/palitra H 00FFFF9F 003CC6DF
sleep 200


echo 
echo ==============
echo runing fillscr
echo ==============

/sys/media/fillscr 105,145,200, 105,145,200, 105,145,200, 60,60,128, 82,102,164, 60,60,128, 60,60,128, 60,60,128, 60,60,128
sleep 200


echo 
echo ==============
echo runing @notify
echo ==============

/sys/@notify "Hello, I am a @notify app!"
sleep 300
/sys/@notify "@notify can show several lines.\nNotices are closed automatically in 5 sec."
sleep 300
/sys/@notify '@notify\nYou can also set an icon and a title.' -tI
sleep 300



exit