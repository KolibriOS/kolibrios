goto %config%
:dos1
rem c:\vc401\vc
lh keyrus
lh mmouse
lh C:\WINDOWS\COMMAND\mscdex /d:12345678
lh \dn\dn
bootgui=0
:dos2
rem essolo.com
lh keyrus
lh mmouse
lh \dn\dn
bootgui=0
:win
rem c:\essolo.com
set path=C:\WINDOWS;C:\WINDOWS\COMMAND;c:\;c:\windows\;c:\windows\comand\;c:\arc\;c:\dn\
C:\WINDOWS\COMMAND\DELTREE /y C:\WINDOWS\TEMP\*.*
mode con codepage prepare=((866) C:\WINDOWS\COMMAND\ega3.cpi)
mode con codepage select=866
keyb ru,,C:\WINDOWS\COMMAND\keybrd3.sys
goto continue
:kos
c:\kolibri\meosload.com
:l:\meosload.com
:continue
rem bootgui=1

