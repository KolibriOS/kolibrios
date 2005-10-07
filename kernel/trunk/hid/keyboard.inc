;// mike.dld [

VKEY_LSHIFT   = 0000000000000001b
VKEY_RSHIFT   = 0000000000000010b
VKEY_LCONTROL = 0000000000000100b
VKEY_RCONTROL = 0000000000001000b
VKEY_LALT     = 0000000000010000b
VKEY_RALT     = 0000000000100000b
VKEY_CAPSLOCK = 0000000001000000b
VKEY_NUMLOCK  = 0000000010000000b
VKEY_SCRLOCK  = 0000000100000000b

VKEY_SHIFT    = 0000000000000011b
VKEY_CONTROL  = 0000000000001100b
VKEY_ALT      = 0000000000110000b

uglobal
  align 4
  kb_state      dd 0
  ext_code      db 0

  keyboard_mode db 0
  keyboard_data db 0

  altmouseb     db 0
  ctrl_alt_del  db 0

  kb_lights     db 0
endg

align 4
irq1:

       call  restore_caller

       movzx eax,word[0x3004] ; top window process
       movzx eax,word[0xC400+eax*2]
       shl   eax,8
       mov   al,[0x800B4+eax]
       mov   [keyboard_mode],al

       in    al,0x60
       mov   [keyboard_data],al

       mov   ch,al
       cmp   al,0xE0
       je    @f
       cmp   al,0xE1
       jne   .normal_code
   @@: mov   [ext_code],al
       jmp   .no_key.end
   .normal_code:
       mov   cl,[ext_code]
       mov   [ext_code],0
       and   al,0x7F
   @@: cmp   al,0x2A
       jne   @f
       cmp   cl,0xE0
       je    .no_key.end
       mov   eax,VKEY_LSHIFT
       jmp   .no_key
   @@: cmp   al,0x36
       jne   @f
       cmp   cl,0xE0
       je    .no_key.end
       mov   eax,VKEY_RSHIFT
       jmp   .no_key
   @@: cmp   al,0x38
       jne   @f
       cmp   cl,0xE0
       je    .alt.r
       mov   eax,VKEY_LALT
       jmp   .no_key
   .alt.r:
       mov   eax,VKEY_RALT
       jmp   .no_key
   @@: cmp   al,0x1D
       jne   @f
       cmp   cl,0
       jne   .ctrl.r
       mov   eax,VKEY_LCONTROL
       jmp   .no_key
   .ctrl.r:
       cmp   cl,0xE1
       jne   .ctrl.r.2
       mov   [ext_code],cl
       jmp   .no_key.end
   .ctrl.r.2:
       mov   eax,VKEY_RCONTROL
       jmp   .no_key
   @@: cmp   al,0x3A
       jne   @f
       mov   bl,4
       mov   eax,VKEY_CAPSLOCK
       jmp   .no_key.xor
   @@: cmp   al,0x45
       jne   @f
       cmp   cl,0
       jne   .no_key.end
       mov   bl,2
       mov   eax,VKEY_NUMLOCK
       jmp   .no_key.xor
   @@: cmp   al,0x46
       jne   @f
       mov   bl,1
       mov   eax,VKEY_SCRLOCK
       jmp   .no_key.xor
   @@:
       test  ch,0x80
       jnz   .no_key.end
       movzx eax,ch              ; plain key
       mov   bl,[keymap+eax]
       mov   edx,[kb_state]
       test  dl,VKEY_CONTROL     ; ctrl alt del
       jz    .noctrlaltdel
       test  dl,VKEY_ALT
       jz    .noctrlaltdel
       cmp   bl,134+48
       jne   .noctrlaltdel
       mov   [ctrl_alt_del],1
       jmp   .no_key.end
   .noctrlaltdel:
       test  dl,VKEY_CONTROL     ; ctrl on ?
       jz    @f
       sub   bl,0x60
   @@: test  dl,VKEY_SHIFT       ; shift on ?
       jz    @f
       mov   bl,[keymap_shift+eax]
   @@: test  dl,VKEY_ALT         ; alt on ?
       jz    @f
       mov   bl,[keymap_alt+eax]
; alt mouse ?
       xor   edx,edx
       cmp   bl,178
       jnz   .no_alt.up
       mov   edx,5*65536
       jmp   .mouse.change
   .no_alt.up:
       cmp   bl,177
       jnz   .no_alt.down
       mov   edx,251*65536
       jmp   .mouse.change
   .no_alt.down:
       cmp   bl,176
       jnz   .no_alt.left
       mov   edx,251*256
       jmp   .mouse.change
   .no_alt.left:
       cmp   bl,179
       jnz   .no_alt.right
       mov   edx,5*256
       jmp   .mouse.change
   .no_alt.right:
       cmp   bl,' '
       jne   @f
       xor   [altmouseb],1
   .mouse.change:
       mov   byte[0xF604],1; ps2 data
       mov   byte[0xFB00],0; ps2 chunk count
       mov   word[0x2E0000+4096*12],3; mouse data count
       mov   dl,[altmouseb]
       mov   [0x2E0000+4096*12+0x10],edx
       mov   bl,0
   @@:
       cmp   [keyboard_mode],0 ; return from keymap
       jne   .no_key.end
       mov   [keyboard_mode_sys],0
       cmp   bl,0
       je    .no_key.end
       movzx eax,byte[0xF400]
       cmp   al,120
       jae   .no_key.end
       inc   al
       mov   [0xF400],al
       mov   [0xF400+eax],bl

       jmp   .no_key.end

   .no_key:
       test  ch,0x80
       jz    .no_key.down
       not   eax
       and   [kb_state],eax
       jmp   .no_key.end
   .no_key.xor:
       test  ch,0x80
       jnz   .no_key.end
       xor   [kb_state],eax
       xor   [kb_lights],bl
       call  set_lights
       jmp   .no_key.end
   .no_key.down:
       or    [kb_state],eax
   .no_key.end:
       cmp   [keyboard_mode],1 ; return scancode
       jne   .no_scancode
       mov   [keyboard_mode_sys],1
       movzx eax,byte[0xF400]
       cmp   al,120
       jae   .no_scancode
       inc   al
       mov   [0xF400],al
       mov   [0xF400+eax],ch
   .no_scancode:

   .exit.irq1:
       mov   [check_idle_semaphore],5

       mov   al,0x20                  ; ready for next irq
       out   0x20,al

       call  return_to_caller

       jmp   irq1

set_lights:
       mov   al,0xED
       call  kb_write
       mov   al,[kb_lights]
       call  kb_write
       ret

;// mike.dld ]
