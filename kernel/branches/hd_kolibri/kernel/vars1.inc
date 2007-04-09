iglobal
  firstapp   db  '/hd0/1/kolibri/bin/LAUNCHER',0
  vrr_m      db  '/hd0/1/kolibri/bin/VRR_M',0

  char  dd 0,0,0
   dd 2560
   dd  0x3F600 - std_application_base_address
   db '/hd0/1/kolibri/FONTS/CHAR.MT',0
  char2 	dd 0,0,0
			dd 2560
			dd  0x3EC00 - std_application_base_address
			db '/hd0/1/kolibri/FONTS/CHAR2.MT',0
  bootpath   db  '/BOOT       '
  bootpath2  db  0
  vmode 	dd 0,0,0
			dd 0x8000
			dd 0x760000 - std_application_base_address
			db '/hd0/1/kolibri/drivers/VMODE.MDR',0
endg
