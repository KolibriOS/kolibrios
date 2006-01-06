MouseSearch_PS2:

     pusha
     
     mov     bl,0xa8                 ; enable mouse cmd
     call    kb_cmd
     cmp     ah,1
     je      @@DataInputError

     mov     bl,0xd4                 ; for mouse
     call    kb_cmd
     cmp     ah,1
     je      @@DataInputError 
     mov     al,0xeb                 ; 
     call    kb_write
     cmp     ah,1
     je      @@DataInputError
     call    kb_read                 ; Acknowledge
     call    kb_read
     mov     [ps2_mouse_detected],0
     test    al,8
     jz      @f
     mov     [ps2_mouse_detected],1
  @@:   
     call    kb_read                 ; 
     call    kb_read                 ; 

     mov     bl,0x20                 ; get command byte
     call    kb_cmd
     cmp     ah,1
     je      @@DataInputError
     call    kb_read
     cmp     ah,1
     je      @@DataInputError
     or      al,3                    ; enable interrupt
     mov     bl,0x60                 ; write command
     push    eax
     call    kb_cmd
     pop     eax
     call    kb_write
     cmp     ah,1
     je      @@DataInputError

     mov     bl,0xd4                 ; for mouse
     call    kb_cmd
     cmp     ah,1
     je      @@DataInputError
     mov     al,0xf4                 ; enable mouse device
     call    kb_write
     cmp     ah,1
     je      @@DataInputError
     call    kb_read           ; read status return
     cmp     ah,1
     je      @@DataInputError
     cmp     AL,0FAh          
     jnz     @@DataInputError ;нет подтверждения

@@DataInputError:
     cmp     [ps2_mouse_detected],0
     je      @f
     mov     esi,boot_setmouse_type
     call    boot_log
  @@:
     popa

