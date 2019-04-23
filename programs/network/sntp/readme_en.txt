Command line:
sntp host [-tz [-[+]]hh[:ss]] [-s]|[-st]|[-ss]]
host  Name of SNTP server
-tz - set time zone, default is GMT +0:00

Syncronization, defautl is disabled 
-s  - system date and time
-st -	system time (hours, mitutes and seconds) only
-ss - safe current hour (syncronize mitutes and seconds only)

Eg:
sntp pool.ntp.org -tz 1 -s
sntp 88.147.254.227 -tz 1 -ss

History