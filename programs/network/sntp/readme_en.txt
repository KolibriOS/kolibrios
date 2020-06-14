Command line:
sntp host [-tz [-[+]]hh[:ss]] [-s]|[-st]|[-ss]]
host  Name of SNTP server
-tz - set time zone, default is GMT +0:00

Synchronization, default is disabled 
-s  - system date and time
-st - system time (hours, minutes and seconds) only
-ss - preserve current hour (synchronize minutes and seconds only)

Eg:
sntp pool.ntp.org -tz 1 -s
sntp 88.147.254.227 -tz 1 -ss

History