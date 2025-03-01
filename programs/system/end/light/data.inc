WIN_W = 440
WIN_H = 200
BOT_PANEL_H = 70
;---------------------------------------------------------------------
CANCEL_BUTTON_ID   = 1+BT_HIDE
HOME_BUTTON_ID     = 2
REBOOT_BUTTON_ID   = 3
POWEROFF_BUTTON_ID = 4
CHECKBOX_BUTTON_ID = 5+BT_HIDE
;---------------------------------------------------------------------
TEXT_HOME: db 'Home',0
TEXT_ENTER: db 'Enter',0
TEXT_END: db 'End',0
TEXT_CANCEL: db 'x',0
;---------------------------------------------------------------------

; Language support for locales: ru_RU (CP866), de_DE, it_IT, en_US

if lang eq ru_RU
    TEXT_WTITLE:  db '�����襭�� ࠡ���',0
    TEXT_RDSAVE1: db '���࠭��� ���������, ᤥ����� �',0
    TEXT_RDSAVE2: db '����� ࠡ��� � ��⥬� (Ctrl+S)',0
    TEXT_KERNEL:  db '���',0
    TEXT_REBOOT:  db '��१���㧪�',0
    TEXT_OFF:     db '�몫�祭��',0
    TEXT_SAVING:  db '���࠭���� ��ࠧ�...',0
else if lang eq de_DE
    TEXT_WTITLE:  db 'Shutdown computer',0
    TEXT_RDSAVE1: db 'Save all changes that were done',0
    TEXT_RDSAVE2: db 'during system work (Ctrl+S)',0
    TEXT_KERNEL:  db 'Kernel',0
    TEXT_REBOOT:  db 'Neustart',0
    TEXT_OFF:     db 'Beenden',0
    TEXT_SAVING:  db 'Saving RAM-drive...',0
else if lang eq it_IT
    TEXT_WTITLE:  db 'Shutdown computer',0
    TEXT_RDSAVE1: db 'Save all changes that were done',0
    TEXT_RDSAVE2: db 'during system work (Ctrl+S)',0
    TEXT_KERNEL:  db 'Kernel',0
    TEXT_REBOOT:  db 'Riavvio',0
    TEXT_OFF:     db 'Spegni',0
    TEXT_SAVING:  db 'Saving RAM-drive...',0
else ; Default to en_US
    TEXT_WTITLE:  db 'Shutdown computer',0
    TEXT_RDSAVE1: db 'Save all changes that were done',0
    TEXT_RDSAVE2: db 'during system work (Ctrl+S)',0
    TEXT_KERNEL:  db 'Kernel',0
    TEXT_REBOOT:  db 'Reboot',0
    TEXT_OFF:     db 'Power off',0
    TEXT_SAVING:  db 'Saving RAM-drive...',0
end if
;---------------------------------------------------------------------
rdsave:
        dd  SSF_START_APP,0,hide,0,0
        db  '/sys/rdsave',0
hide        db 'h',0
asettings   db 'settings',0
aautosave   db 'autosave',0
ini_file db  '/sys/settings/rdsave.ini',0
;---------------------------------------------------------------------
importLib:
library                                                 \
        libini, 'libini.obj'

import  libini, \
        ini_set_int  ,'ini_set_int', \
        ini_get_int  ,'ini_get_int'
;---------------------------------------------------------------------
checkbox_sharedname db 'CHECKBOX',0
checkbox_img dd ?
;---------------------------------------------------------------------
