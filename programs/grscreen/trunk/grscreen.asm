;
;   NICE BACKGROUND
;
;   Compile with FASM for Menuet
;

;******************************************************************************
    use32
    org    0x0

    db     'MENUET01'      ; 8 byte id
    dd     0x01            ; header version
    dd     START           ; start of code
    dd     I_END           ; size of image
    dd     0x5000         ; memory for app
    dd     0x5000         ; esp
    dd     0x0 , 0x0       ; I_Param , I_Icon

include    "lang.inc"
include    "macros.inc"
;******************************************************************************

;GRADES       =    100         ; count of grades
;START_COLOR  =    0x8292B4      ;0x0078b000
;STEP         =    0x010101      ;0x00010100
;xxx          equ  sub         ; from dark to light

;******************************************************************************

db "MenuetOS RE #8",13,10

START:
   ; load system colors
   mcall 58, read_info

   ; set system colors
   mcall 48, 2, sc, sizeof.system_colors

   ; set stretch backgound
   mcall 15, 4, 2

   ; set wallpaper
   mcall 58, start_info

;jmp exit
;
;mov eax,image+3     ; generate image
;mov ecx,GRADES-1
;@@:
;mov ebx,[eax-3]
;xxx ebx,STEP
;mov [eax],ebx
;add eax,3
;dec ecx
;jnz @b

;mov eax,15          ; copy image to background memory
;mov ebx,5
;mov ecx,image
;xor edx,edx
;mov esi,(GRADES+1)*3
;int 0x40

;mov eax,15          ; set stretch backgound
;mov ebx,4
;mov ecx,2
;int 0x40

;mov eax,15          ; set background size
;mov ebx,1
;mov ecx,ebx
;mov edx,GRADES
;int 0x40

;mov eax,15          ; draw background
;mov ebx,3
;int 0x40

;exit:

; BEGIN_REDRAW_SCREEN
;   mcall 12, 1

;   mcall 14
;   mov   ecx, eax
;   shr   eax, 16
;   and   ecx, 0xFFFF
;   mov   ebx, eax
;   mov   edx, 0x01000000
;   mcall 0

;   mcall 12, 2
; END_REDRAW_SCREEN
   mcall 5,100
   mcall -1

;------------------------------------------------------------------------------

read_info:
  .mode        dd 0
  .start_block dd 0
  .blocks      dd 1
  .address     dd sc
  .workarea    dd work_area
  .path        db "/rd/1/myblue.dtp",0

start_info:
  .mode        dd 16
               dd 0
  .params      dd boot
               dd 0
  .workarea    dd work_area
  .path        db "/rd/1/jpegview",0

boot           db 'BOOT',0

;------------------------------------------------------------------------------

;image:
;dd START_COLOR

I_END:
;rd 256

sc  system_colors
 rb 512-40

align 32
work_area:

; EOF