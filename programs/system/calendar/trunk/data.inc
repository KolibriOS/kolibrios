; program data

curMonth dd ?
curYear dd ?
curDay dd ?

remainder dd ?
quotient dd ?

firstday  dd ?
Year dd   ?
Month dd  ?
day_sel   dd ?

datestr   dd  ?
leap_year dd ?
number	  dd ?

; colors
COL_WINDOW_BG    equ 0x54E7E7E7
COL_TOOLBAR_BG   equ 0x555555
COL_ACTIVE_TEXT  equ 0x10e7c750
COL_WEEKDAY_BG   equ 0xCCCDDD
COL_DATE_BUTTONS equ 0xF3F3F3
COL_TIME_BUTTONS equ 0x5555dd
COL_GO_TODAY_T   equ 0xd048c8
COL_MONTH_YEAR_B equ 0x555555
COL_DROPDOWN_BG  equ 0x666666
COL_DROPDOWN_T   equ 0x10CCCccc
COL_NEW_STYLE_T  equ 0xef3333

COL_DATE_INACTIVE_1 equ 0x388CAF
COL_DATE_INACTIVE_2 equ 0x75B9D5
COL_DATE_INACTIVE_3 equ 0x9ECBDF
COL_DATE_INACTIVE_4 equ 0xABD7E8

COL_DATE_ACTIVE_1   equ 0xA57F2C
COL_DATE_ACTIVE_2   equ 0xEFB52D
COL_DATE_ACTIVE_3   equ 0xFAD84C
COL_DATE_ACTIVE_4   equ 0xFAE995

COL_DATE_1   equ 0x8C8C8C
COL_DATE_2   equ 0xFFFfff
COL_DATE_3   equ 0xF2F2F2
COL_DATE_4   equ 0xF2F2F2

; definitions
WIN_W equ 275
WIN_H equ 326

DATE_BUTTON_WIDTH  equ 32
DATE_BUTTON_HEIGHT equ 30


LINE1	  equ 27 shl 16+16
B_MONTH_X equ 15 shl 16+158
B_Y	  equ LINE1
B_MONTH   equ 63 shl 16+27

BT_WBAR_Y equ 283 shl 16+43

B_WBAR_X  equ 1 shl 16+WIN_W-1
B_WBAR_Y  equ 64 shl 16+20
B_WEEK	  equ 34 shl 16+66
B_WX_SHIFT equ 32 shl 16

B_DROP	  equ B_MONTH+16
B_DAYS_Y  equ 106
B_DAYS_SHIFT equ 30

B_YEAR	  equ 204 shl 16+28

B_SPIN_X  equ 186 shl 16+12
B_SPIN	  equ 188 shl 16+28
spinner db '<      >'

B_TODAY_X equ 25 shl 16
B_TODAY_Y equ 48 shl 16+10
B_TODAY   equ 30 shl 16+50

B_NS_X	  equ 185 shl 16+75
B_NS_Y	  equ 48 shl 16+10
B_NS	  equ 190 shl 16+50

FOCUSABLE equ 4

; multilanguage interface - configuratable in lang.inc
day_count db 3,0,3,2,3,2,3,3,2,3,2,3
Fkeys	  db 210,211,212,213,214,215,216,217,208,209,228,159

month_name:
if lang eq ru
     db   8
     db   'Январь  '
     db   'Февраль '
     db   'Март    '
     db   'Апрель  '
     db   'Май     '
     db   'Июнь    '
     db   'Июль    '
     db   'Август  '
     db   'Сентябрь'
     db   'Октябрь '
     db   'Ноябрь  '
     db   'Декабрь '
else if lang eq de
     db   9
     db   'Januar   '
     db   'Februar  '
     db   'M+rz     '
     db   'April    '
     db   'Mai      '
     db   'Juni     '
     db   'Juli     '
     db   'August   '
     db   'September'
     db   'Oktober  '
     db   'November '
     db   'Dezember '
else if lang eq fr
     db   9
     db   'Janvier  '
     db   'Fevrier  '
     db   'Mars     '
     db   'Avril    '
     db   'Mai      '
     db   'Juin     '
     db   'Juliet   '
     db   'Aout     '
     db   'Septembre'
     db   'Octobre  '
     db   'Novembre '
     db   'Decembre '
else if lang eq fi
     db   9
     db   'Tammikuu '
     db   'Helmikuu '
     db   'Maaliskuu'
     db   'Huhtikuu '
     db   'Toukokuu '
     db   'Kes+kuu  '
     db   'Hein+kuu '
     db   'Elokuu   '
     db   'Syyskuu  '
     db   'Lokakuu  '
     db   'Marraskuu'
     db   'Joulukuu '
else if lang eq et
     db   9
     db   'Jaanuar  '
     db   'Veebruar '
     db   'Mфrts    '
     db   'Aprill   '
     db   'Mai      '
     db   'Juuni    '
     db   'Juuli    '
     db   'August   '
     db   'September'
     db   'Oktoober '
     db   'November '
     db   'Detsember'
else if lang eq it
     db   9
     db   'Gennaio  '
     db   'Febbraio '
     db   'Marzo    '
     db   'Aprile   '
     db   'Maggio   '
     db   'Giugno   '
     db   'Luglio   '
     db   'Agosto   '
     db   'Settembre'
     db   'Ottobre  '
     db   'Novembre '
     db   'Dicembre '
else
     db   9
     db   'January  '
     db   'February '
     db   'March    '
     db   'April    '
     db   'May      '
     db   'June     '
     db   'July     '
     db   'August   '
     db   'September'
     db   'October  '
     db   'November '
     db   'December '
end if
week_days:
if lang eq ru
     db   2
     db   1
     db   'Пн'
     db   'Вт'
     db   'Ср'
     db   'Чт'
     db   'Пт'
     db   'Сб'
     db   'Вс'
else if lang eq de
     db   2
     db   7
     db   'So'
     db   'Mo'
     db   'Di'
     db   'Mi'
     db   'Do'
     db   'Fr'
     db   'Sa'
else if lang eq fr
     db   3
     db   7
     db   'Dim'
     db   'Lun'
     db   'Mar'
     db   'Mer'
     db   'Jeu'
     db   'Ven'
     db   'Sam'
else if lang eq fi
     db   2
     db   7
     db   'Su'
     db   'Ma'
     db   'Ti'
     db   'Ke'
     db   'To'
     db   'Pe'
     db   'La'
else if lang eq et
     db   3
     db   7
     db   'Esm'
     db   'Tei'
     db   'Kol'
     db   'Nel'
     db   'Ree'
     db   'Lau'
     db   'P№h'
else if lang eq it
     db   3
     db   7
     db   'Dom'
     db   'Lun'
     db   'Mar'
     db   'Mer'
     db   'Gio'
     db   'Ven'
     db   'Sab'
else
     db   3
     db   7
     db   'Sun'
     db   'Mon'
     db   'Tue'
     db   'Wen'
     db   'Thi'
     db   'Fri'
     db   'Sat'
end if

title:
if lang eq ru
     db   'Календарь',0
else if lang eq ge
     db   'Kalender',0
else if lang eq fr
     db   'Calendrier',0
else if lang eq et
     db   'Kalender',0
else if lang eq it
     db   'Calendario',0
else
     db   'Calendar',0
end if

today_msg:
if lang eq ru
     db   'Сегодня'
else if lang eq ge
     db   'Heute'
else if lang eq fr
     db   "Aujourd'hui"
else if lang eq et
     db   'Tфna'
else if lang eq it
     db   'Oggi'
else
     db   'Today'
end if
today_end:
focus dd  3
new_style dd 1
dropped db 0

sys_text:
if lang eq ru
		db  'Время',0
else if lang eq it
		db  '  Ora',0
else if lang eq et
		db  ' Aeg ',0
else
		db  ' Time',0
end if
separator  db ':',0
plus  db  '+',0
minus db  '-',0
set_date_t:
if lang eq ru
		db  'Установить дату',0
else if lang eq it
		db   ' Impostazioni ',0
else if lang eq et
		db   ' Mффra kuupфev ',0
else
		db  '   Set date    ',0
end if

