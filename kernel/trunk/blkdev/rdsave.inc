sysfn_saveramdisk:    ; 18.6 = SAVE FLOPPY IMAGE (HD version only)
     cmp  ebx,1
     jnz  img_save_hd_1
     mov  edx,bootpath             ; path = '/KOLIBRI    '
     jmp  img_save_hd_3
 img_save_hd_1:
     cmp  ebx,2
     jnz  img_save_hd_2
     mov  edx,bootpath2             ; path = 0 (root dir)
     jmp  img_save_hd_3
 img_save_hd_2:
     cmp  ebx,3
     jnz  exit_for_anyone
     mov edx,[0x3010]
     mov edx,[edx+TASKDATA.mem_start]
     add edx,ecx
 img_save_hd_3:
     call    reserve_hd1
     call    reserve_hd_channel
     call restorefatchain      ; restore FAT !!!
     mov  eax,image_save
     mov  ebx,1440*1024        ; size 1440 Kb
     mov  ecx,0x100000         ; address of image
     call file_write
     call    free_hd_channel
     mov  [esp+36],eax
     ret
