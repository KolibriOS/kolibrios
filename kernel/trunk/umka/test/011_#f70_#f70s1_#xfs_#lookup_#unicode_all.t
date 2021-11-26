umka_init
disk_add ../img/xfs_v4_unicode.img hd0 -c 0

stat80 /hd0/1/dir0
stat80 /hd0/1/dir0/
stat80 /hd0/1/дир❦
stat80 /hd0/1/дир❦/
stat80 /hd0/1/дир❦/дир11
stat80 /hd0/1/дир❦/дир11/
stat80 /hd0/1/❦❦❦
stat80 /hd0/1/❦❦❦/
stat80 /hd0/1/❦❦❦/д❦р22
stat80 /hd0/1/дир3

stat80 /hd0/1/dir0/file00
stat80 /hd0/1/dir0/file00/
stat80 /hd0/1/❦❦❦/д❦р22/❦❦
stat80 /hd0/1/❦❦❦/д❦р22/❦❦/
stat80 /hd0/1/❦❦❦/д❦р22/💗💗
stat80 /hd0/1/дир3/файл33
stat80 /hd0/1/дир3/файл33/

read80 /hd0/1/dir0/file00 0 100 -b
read80 /hd0/1/dir0/file00/ 0 100 -b
read80 /hd0/1/❦❦❦/д❦р22/❦❦ 0 100 -b
read80 /hd0/1/дир3/файл33 0 100 -b

ls70 /hd0/1/ -e utf8
ls70 /hd0/1/ -e utf16
ls70 /hd0/1/ -e cp866
ls70 /hd0/1/ -e default
ls70 /hd0/1/❦❦❦/ -e utf8
ls70 /hd0/1/❦❦❦/ -e utf16
ls70 /hd0/1/❦❦❦/ -e cp866
ls70 /hd0/1/❦❦❦/ -e default
ls70 /hd0/1/дир3/ -e utf8
ls70 /hd0/1/дир3/ -e utf16
ls70 /hd0/1/дир3/ -e cp866
ls70 /hd0/1/дир3/ -e default

ls80 /hd0/1/ -e utf8
ls80 /hd0/1/ -e utf16
ls80 /hd0/1/ -e cp866
ls80 /hd0/1/ -e default
ls80 /hd0/1/❦❦❦/ -e utf8
ls80 /hd0/1/❦❦❦/ -e utf16
ls80 /hd0/1/❦❦❦/ -e cp866
ls80 /hd0/1/❦❦❦/ -e default
ls80 /hd0/1/дир3/ -e utf8
ls80 /hd0/1/дир3/ -e utf16
ls80 /hd0/1/дир3/ -e cp866
ls80 /hd0/1/дир3/ -e default

ls80 /hd0/1/❦👩❦/ -e utf8
ls80 /hd0/1/❦👩❦/ -e utf16
ls80 /hd0/1/❦👩❦/ -e cp866
ls80 /hd0/1/❦👩❦/ -e default

disk_del hd0
