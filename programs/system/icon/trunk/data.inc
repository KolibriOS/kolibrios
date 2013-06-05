;------------------------------------------------------------------------------
tl	dd 8
yw:	dd 51
ya	dd 0
cur_btn	dd 40
;------------------------------------------------------------------------------
; DATA AREA
bcolor dd 0x335599
;------------------------------------------------------------------------------
icon_table:
 times 4  db 'xxxx  xxxx'
 times 2  db '          '
 times 1  db '          '
 times 2  db 'xxxx  xxxx'
; times 1  db '          '
;------------------------------------------------------------------------------
icons_reserved:
	times 9  db '          '
;------------------------------------------------------------------------------
if lang eq ru
text:
	db 255,255,255,0,   '   íÖäëí                                       '
	db 255,255,255,0,   ' èêéÉêÄååÄ                                     '
	db 255,255,255,0,   ' èÄêÄåÖíêõ                                     '
	db 255,255,255,0,   '                 èêàåÖçàíú                     '
	db 255,255,255,0,   '      ÑéÅÄÇàíú              ìÑÄãàíú            '
	db 0,0,0,0,         'çÄÜåàíÖ çÄ èéáàñàû àäéçäà Ñãü êÖÑÄäíàêéÇÄçàü   '
	db                  'x' ; <- END MARKER, DONT DELETE

add_text	db 'çÄÜåàíÖ çÄ èéáàñàû çÖàëèéãúáìÖåéâ àäéçäà     ',0
rem_text	db 'çÄÜåàíÖ çÄ èéáàñàû àëèéãúáìÖåéâ àäéçäà       ',0
title		db 'å•≠•§¶•‡ ®™Æ≠Æ™',0

else if lang eq ge
text:
	db 255,255,255,0,   '   TITLE                                       '
	db 255,255,255,0,   '  APP NAME                                     '
	db 255,255,255,0,   ' PARAMETER                                     '
	db 255,255,255,0,   '                ANWENDEN                       '
	db 255,255,255,0,   '     HINZUFUEGEN              ENTFERNEN        '
	db 0,0,0,0,         'AUF BUTTON KLICKEN, UM ICON ZU EDITIEREN       '
	db                  'x' ; <- END MARKER, DONT DELETE

add_text	db 'AUF UNBENUTZTE ICONPOSITION KLICKEN          ',0
rem_text	db 'ICON ANKLICKEN; DAS GELOESCHT WERDEN SOLL    ',0
title		db 'Icon Manager',0

else if lang eq it
text:
	db 255,255,255,0,   'Titolo                                         '
	db 255,255,255,0,   'Nome app                                       '
	db 255,255,255,0,   'Parametri                                      '
	db 255,255,255,0,   '           Applica modifiche                   '
	db 255,255,255,0,   '      Aggiungi               Rimuovi           '
	db 0,0,0,0,         'Clicca sull icona per effettuare modifiche     '
	db                  'x' ; <- END MARKER, DONT DELETE

add_text	db 'Clicca su una posizione non usata            ',0
rem_text	db 'Cliccare sull icona da rimuovere             ',0
title		db 'Gestore Icone',0

else
text:
	db 255,255,255,0,   '   TITLE                                       '
	db 255,255,255,0,   '  APP NAME                                     '
	db 255,255,255,0,   ' PARAMETERS                                    '
	db 255,255,255,0,   '                APPLY CHANGES                  '
	db 255,255,255,0,   '      ADD ICON              REMOVE ICON        '
	db 0,0,0,0,         'CLICK BUTTON ON ICON POSITION FOR EDIT         '
	db                  'x' ; <- END MARKER, DONT DELETE
add_text	db 'CLICK ON A NOT USED POSITION                 ',0
rem_text	db 'CLICK ICON POSITION; YOU WANT TO DELETE      ',0
title		db 'Icon Manager',0

end if
;------------------------------------------------------------------------------
arrows	db '</>'
;------------------------------------------------------------------------------
iconname:
	db ICON_APP,0
;------------------------------------------------------------------------------
icon_default:
	db 'AA-CALC    -004-/RD/1/CALC                    '
	db '-                              *'
	db 13,10
;------------------------------------------------------------------------------
rep_text:
if lang eq ru
	db 'áçÄóäà    -     àá    , ÇõÅêÄç #'
;else if lang eq it
;	db 'Icone     -     di    , Selezionate'
else
	db 'ICONS     -     OF    , SELECTED'
end if

rep_text_len:
;------------------------------------------------------------------------------
align 4
finfo_start:
	dd 7
	dd 0
.params	dd 0
	dd 0
	dd 0
	db 0
	dd finfo.path
;------------------------------------------------------------------------------
align 4
finfo:
	dd 5
	dd 0
	dd 0
.size	dd 0
.point	dd procinfo
.path:
	db ICON_STRIP,0
	rb 31-($-.path)
;------------------------------------------------------------------------------
align 4
positions	dd 3,16,47
str_lens	db 8,30,30

current_icon	dd icon_data

iconlst	db ICONS_DAT,0

;image	dd image_area
;------------------------------------------------------------------------------
; not change this section!!!
; start section
;------------------------------------------------------------------------------
align 4
image_file     dd 0 ;+0
raw_pointer    dd 0 ;+4
return_code    dd 0 ;+8
img_size       dd 0 ;+12
deflate_unpack dd 0 ;+16	; not use for scaling
raw_pointer_2  dd 0 ;+20	; not use for scaling
;------------------------------------------------------------------------------
; end section
;------------------------------------------------------------------------------
system_dir_CnvPNG	db '/sys/lib/cnv_png.obj',0
system_dir_UNPACK	db '/sys/lib/archiver.obj',0

er_message_found_lib1	db 'cnv_png.obj - Not found!',0
er_message_import1	db 'cnv_png.obj - Wrong import!',0

err_message_found_lib2	db 'archiver.obj - Not found!',0
err_message_import2	db 'archiver.obj - Wrong import!',0

ihead_f_i:
ihead_f_l        db 'Errors of plugins:',0

;plugins_directory               db 'plugins/',0
plugins_directory               db 0
;------------------------------------------------------------------------------
l_libs_start:

library01	l_libs	system_dir_CnvPNG+9,path,library_path,system_dir_CnvPNG,\
er_message_found_lib1,ihead_f_l,cnv_png_import,er_message_import1,ihead_f_i,plugins_directory

library02	l_libs	system_dir_UNPACK+9,path,library_path,system_dir_UNPACK,\
err_message_found_lib2,ihead_f_l,UNPACK_import,err_message_import2,ihead_f_i,plugins_directory

end_l_libs:
;---------------------------------------------------------------------
align	4
cnv_png_import:
.Start		dd aCP_Start
.Version	dd aCP_Version
.Check		dd aCP_Check
.Assoc		dd aCP_Assoc
	dd 0
	dd 0
aCP_Start	db 'START',0
aCP_Version	db 'version',0
aCP_Check	db 'Check_Header',0
aCP_Assoc	db 'Associations',0
;---------------------------------------------------------------------
align 4
UNPACK_import:
;unpack_Version				dd aUnpack_Version
;unpack_PluginLoad			dd aUnpack_PluginLoad
;unpack_OpenFilePlugin		dd aUnpack_OpenFilePlugin
;unpack_ClosePlugin			dd aUnpack_ClosePlugin
;unpack_ReadFolder			dd aUnpack_ReadFolder
;unpack_SetFolder			dd aUnpack_SetFolder
;unpack_GetFiles				dd aUnpack_GetFiles
;unpack_GetOpenPluginInfo	dd aUnpack_GetOpenPluginInfo
;unpack_Getattr				dd aUnpack_Getattr
;unpack_Open					dd aUnpack_Open
;unpack_Read					dd aUnpack_Read
;unpack_Setpos				dd aUnpack_Setpos
;unpack_Close				dd aUnpack_Close
;unpack_DeflateUnpack		dd aUnpack_DeflateUnpack
unpack_DeflateUnpack2		dd aUnpack_DeflateUnpack2
        dd      0
        dd      0

;aUnpack_Version        db      'version',0
;aUnpack_PluginLoad     db      'plugin_load',0
;aUnpack_OpenFilePlugin db      'OpenFilePlugin',0
;aUnpack_ClosePlugin    db      'ClosePlugin',0
;aUnpack_ReadFolder     db      'ReadFolder',0
;aUnpack_SetFolder      db      'SetFolder',0
;aUnpack_GetFiles       db      'GetFiles',0
;aUnpack_GetOpenPluginInfo db   'GetOpenPluginInfo',0
;aUnpack_Getattr        db      'getattr',0
;aUnpack_Open           db      'open',0
;aUnpack_Read           db      'read',0
;aUnpack_Setpos         db      'setpos',0
;aUnpack_Close          db      'close',0
;aUnpack_DeflateUnpack  db      'deflate_unpack',0
aUnpack_DeflateUnpack2 db      'deflate_unpack2',0
;------------------------------------------------------------------------------
