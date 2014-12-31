include 'AKODE/udata.inc'

align 4
StartupPath                     rb 1024

ThreadInfoBuffer                process_information

MainWindowWidth                 dd ?
MainWindowHeight                dd ?

WorldViewHeight                 dd ?

PressedKeys                     rd 128 * 2
Inventory                       rq INVENTORY_SIZE