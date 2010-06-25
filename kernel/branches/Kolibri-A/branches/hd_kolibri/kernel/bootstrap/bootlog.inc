boot_log:
         pushad

         mov   eax,10*65536
         mov   ax,word [boot_y]
         add   [boot_y],dword 10
         mov   ebx,0x80ffffff   ; ASCIIZ string with white color
         mov   ecx,esi
         mov   edi,1
         call  dtext

         mov   [novesachecksum],1000
         call  checkVga_N13

         cmp   [preboot_blogesc],byte 1
         je    .bll2

         cmp   esi,boot_tasking
         jne   .bll2
         ; begin ealex 04.08.05
;         in    al,0x61
;         and   al,01111111b
;         out   0x61,al
         ; end ealex 04.08.05
.bll1:   in    al,0x60    ; wait for ESC key press
         cmp   al,129
         jne   .bll1

.bll2:   popad

         ret
