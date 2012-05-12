; App written by randall ported to Kolibri and MenuetOS64 by macgub (www.macgub.hekko.pl). 
; Now it use static memory, it is mixed 32bit code and SSE instructions.

include '../../../macros.inc'

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     IMG_END                   ; size of image
               dd     I_END ;0x100000                ; memory for app
               dd     I_END ;0xbffff                 ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

START:                          ; start of execution

     call draw_window
     call Main
     call draw_from_buffer

still:


;     call Main


;            mov eax,7
;            mov ebx,screen
;            mov ecx,IMG_SIZE*65536+IMG_SIZE
;            mov edx,0*65536+0
;            int 0x40

;    mov  eax,23                 ; wait here for event
;    mov  ebx,timeout
;    int  0x40
 ;   mov eax,11                   ; check for event no wait
    mov eax,10                  ; wait for event
    int 0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button

    jmp  noclose

  red:                          ; redraw
    call draw_window
    call draw_from_buffer
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40
    shr  eax,8
    cmp  eax, 27
    jne  still
    mov  eax, -1
    int  0x40


  button:                       ; button
    mov  eax,17                 ; get id
    int  0x40

    cmp  ah,1                   ; button id=1 ?
    jne  noclose

    mov  eax,-1                 ; close this program
    int  0x40
  noclose:

    jmp  still


draw_from_buffer:

            mov eax,7
            mov ebx,screen
            mov ecx,IMG_SIZE*65536+IMG_SIZE
            mov edx,0*65536+0
            int 0x40
ret

;-------------------------------------------------------------------------------
; NAME:         XORWOW
; DESC:         Pseudo random number generator.
; OUT:          eax         [0;2^32-1]
;-------------------------------------------------------------------------------
macro           XORWOW      {
                mov         edx,[g_xorwow_x]    ; edx = x
                shr         edx,2               ; edx = x >> 2
                xor         edx,[g_xorwow_x]    ; t = x ^ (x >> 2)
                mov         eax,[g_xorwow_y]    ; eax = y
                mov         [g_xorwow_x],eax    ; x = y
                mov         eax,[g_xorwow_z]    ; eax = z
                mov         [g_xorwow_y],eax    ; y = z
                mov         eax,[g_xorwow_w]    ; eax = w
                mov         [g_xorwow_z],eax    ; z = w
                mov         eax,[g_xorwow_v]    ; eax = v
                mov         [g_xorwow_w],eax    ; w = v
                mov         edi,eax             ; edi = v
                shl         edi,4               ; edi = v << 4
                xor         edi,eax             ; edi = (v ^ (v << 4))
                mov         eax,edx             ; eax = t
                shl         eax,1               ; eax = t << 1
                xor         eax,edx             ; eax = (t ^ (t << 1))
                xor         eax,edi             ; eax = (v ^ (v << 4)) ^ (t ^ (t << 1))
                mov         [g_xorwow_v],eax    ; v = eax
                add         [g_xorwow_d],362437 ; d += 362437
                mov         eax,[g_xorwow_d]    ; eax = d
                add         eax,[g_xorwow_v]    ; eax = d + v
}
;-------------------------------------------------------------------------------
; NAME:         RANDOM
; DESC:         Returns pseudo random number in the range [-0.5;0.5).
; OUT:          xmm0.x      [-0.5;0.5)
;-------------------------------------------------------------------------------
macro           RANDOM {
                XORWOW
                cvtsi2ss    xmm0,eax
                mulss       xmm0,[g_rand_scale]
}
;-------------------------------------------------------------------------------

;-------------------------------------------------------------------------------
; NAME:         GenerateSequence
; IN:           xmm0.x      re (c0.x)
; IN:           xmm1.x      im (c0.y)
; IN:           edi         array size
; IN/OUT:       esi         pointer to the allocated array
; OUT:          eax         generated sequence size
;-------------------------------------------------------------------------------
align 16
GenerateSequence:
                xor         eax,eax     ; eax is index loop
                xorps       xmm4,xmm4   ; xmm4 is c.x
                xorps       xmm5,xmm5   ; xmm5 is c.y
.Loop:
                ; cn.x = c.x * c.x - c.y * c.y + c0.x
                movaps      xmm2,xmm4
                movaps      xmm3,xmm5
                mulss       xmm2,xmm4
                mulss       xmm3,xmm5
                subss       xmm2,xmm3
                addss       xmm2,xmm0
                movaps      xmm6,xmm2   ; xmm6 is cn.x
                ; cn.y = 2.0 * c.x * c.y + c0.y
                movaps      xmm7,xmm4
                mulss       xmm7,xmm5
                addss       xmm7,xmm7
                addss       xmm7,xmm1   ; xmm7 is cn.y
                ; store cn
                movd        dword [esi+eax*8],xmm6
                movd        dword [esi+eax*8+4],xmm7
                ; if (cn.x * cn.x + cn.y * cn.y > 10.0) return eax;
                movaps      xmm2,xmm6
                movaps      xmm3,xmm7
                mulss       xmm2,xmm6
                mulss       xmm3,xmm7
                addss       xmm2,xmm3
                ucomiss     xmm2,[g_max_dist]
                ja          .EndLoop
                movaps      xmm4,xmm6   ; c.x = cn.x
                movaps      xmm5,xmm7   ; c.y = cn.y
                ; continue loop
                inc         eax
                cmp         eax,edi
                jb          .Loop
                ; return 0
                xor         eax,eax
.EndLoop:
                ret
;-------------------------------------------------------------------------------
; NAME:         main
; DESC:         Program main function.
;-------------------------------------------------------------------------------
align 16
Main:
img_ptr         equ         ebp-8
seq_ptr         equ         ebp-16
pixel           equ         ebp-24
r13dd           equ         ebp-64
r12dd           equ         ebp-68
r15dd           equ         ebp-72

                push        ebp
                mov         ebp,esp
                sub         esp,128
                ;  mem for the sequence
                lea         eax,[sequence]
                mov         [seq_ptr],eax
                ;  mem for the image
                lea         eax,[screen]
                mov         [img_ptr],eax
                ; begin loops
                mov         dword[r13dd],0         ; .LoopIterations counter
.LoopIterations:
                mov         dword[r12dd],0         ; .LoopOneMillion counter
.LoopOneMillion:
                RANDOM
                mulss       xmm0,[g_range]
                movaps      xmm1,xmm0
                RANDOM
                mulss       xmm0,[g_range]
                mov         edi,SEQ_SIZE
                mov         esi,[seq_ptr]
                call        GenerateSequence  ; eax = n sequence size
                test        eax,eax
                jz          .LoopSequenceEnd
                xor         ecx,ecx           ; ecx = i = 0 loop counter
         ;       mov         r9dd,[seq_ptr]      ; r9 = sequence base address
         ;       mov         r8dd,[img_ptr]      ; r8 = image base address
                movss       xmm2,[g_img_size]
                movaps      xmm3,xmm2
                mulss       xmm3,[g_0_5]      ; xmm3 = (g_img_size)/2
                movss       xmm4,[g_zoom]
                mulss       xmm4,xmm2         ; xmm4 = g_zoom * g_img_size
                movss       xmm5,[g_offsetx]  ; xmm5 = g_offsetx
                movss       xmm6,[g_offsety]  ; xmm6 = g_offsety
.LoopSequence:
                cmp         ecx,eax           ; i < n
                je          .LoopSequenceEnd
                movd        xmm0,[sequence+ecx*8]   ; load re
                movd        xmm1,[sequence+ecx*8+4] ; load im
                addss       xmm0,xmm5         ; xmm0 = re+g_offsetx
                addss       xmm1,xmm6         ; xmm1 = im+g_offsety
                mulss       xmm0,xmm4         ; xmm0 = (re+g_offsetx)*g_img_size*g_zoom
                mulss       xmm1,xmm4         ; xmm1 = (im+g_offsety)*g_img_size*g_zoom
                addss       xmm0,xmm3         ; xmm0 = (re+g_offsetx)*g_img_size*g_zoom+g_img_size/2
                addss       xmm1,xmm3         ; xmm1 = (im+g_offsety)*g_img_size*g_zoom+g_img_size/2
                cvtss2si    edi,xmm0          ; edi = x = int(xmm0.x)
                cvtss2si    esi,xmm1          ; esi = y = int(xmm1.x)
                cmp         edi,0
                jl          @f
                cmp         edi,IMG_SIZE
                jge         @f
                cmp         esi,0
                jl          @f
                cmp         esi,IMG_SIZE
                jge         @f
                imul        esi,esi,IMG_SIZE
                add         esi,edi
                add         dword [screen+esi*4],1
@@:
                inc         ecx
                jmp         .LoopSequence
.LoopSequenceEnd:
                ; continue .LoopOneMillion
                add         dword[r12dd],1
                cmp         dword[r12dd],1000000
                jb          .LoopOneMillion
                ; continue .LoopIterations
                add         dword[r13dd],1
                cmp         dword[r13dd],ITERATIONS
                jb          .LoopIterations
                ; find max value
                mov         dword[r12dd],0
                xor         eax,eax      ; eax = i = loop counter
.LoopMax:
                push        ecx
                mov         ecx,[r12dd]
                cmp         dword [screen+eax*4],ecx
                cmova       ecx,dword [screen+eax*4]
                mov         [r12dd],ecx
                pop         ecx
                inc         eax
                cmp         eax,IMG_SIZE*IMG_SIZE
                jb          .LoopMax
                ; find min value
        ;        mov         r13d,r12d   ; r13d = min_val = max_val
                push        dword[r12dd]
                pop         dword[r13dd]
                xor         eax,eax     ; eax = i = loop counter
.LoopMin:
                push        ecx
                mov         ecx,[r13dd]
                cmp         dword [screen+eax*4],ecx

                cmovb       ecx,dword [screen+eax*4]
                mov         [r13dd],ecx
                pop         ecx
                inc         eax
                cmp         eax,IMG_SIZE*IMG_SIZE
                jb          .LoopMin
                ; write image pixels
                mov         byte [pixel+3],255
       ;         mov         r14,[img_ptr]   ; r14 = image base address
       ;         xor         r15d,r15d       ; r15d = i = loop counter
                mov         dword[r15dd],0
                cvtsi2ss    xmm0,[r12dd]       ; load max_value
                cvtsi2ss    xmm1,[r13dd]       ; load min_value
                movaps      xmm2,xmm0
                subss       xmm2,xmm1       ; xmm2 = r = max_value - min_value
                xor         ecx,ecx
.LoopWrite:
                mov         eax,[screen+ecx*4] ; eax = image_value
                sub         eax,[r13dd]        ; eax = image_value - min_value
                cvtsi2ss    xmm0,eax        ; xmm0 = float(image_value - min_value)
                addss       xmm0,xmm0       ; xmm0 = 2.0f * float(image_value - min_value)
                divss       xmm0,xmm2       ; xmm0 = 2.0f * float(image_value - min_value) / r
                minss       xmm0,[g_1_0]    ; clamp to 1.0
                maxss       xmm0,[g_0_0]    ; clamp to 0.0
                mulss       xmm0,[g_255_0]  ; convert to 0 - 255
                cvtss2si    eax,xmm0
                ; write pixel data
                mov         [screen+ecx*3],eax
                inc         ecx
                cmp         ecx,IMG_SIZE*IMG_SIZE
                jb          .LoopWrite
                mov         esp,ebp
                pop         ebp
                ret
       ;         restore     img_ptr,seq_ptr,pixel
;-------------------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
draw_window:

    mcall 12, 1                                   ; function 12:tell os about windowdraw
    
	mcall 48, 4                                   ;get skin width
	lea	ecx, [50*65536+IMG_SIZE+4+eax]            ; [y start] *65536 + [y size] + [skin_height]
	mcall	0,<50,IMG_SIZE+9>,,0x74000000,,labelt ;draw window

    mcall 12, 2                                   ; function 12:tell os about windowdraw

    ret



;-------------------------------------------------------------------------------
align 1
labelt:
 db  'buddhabrot',0
labelen:

align 4
g_xorwow_x      dd          123456789
g_xorwow_y      dd          362436069
g_xorwow_z      dd          521288629
g_xorwow_w      dd          88675123
g_xorwow_v      dd          5783321
g_xorwow_d      dd          6615241
g_rand_scale    dd          2.3283064e-10 ; 1.0 / 2^32

IMG_SIZE=600
SEQ_SIZE=50
ITERATIONS=100
g_img_size      dd          600.0
g_offsetx       dd          0.5
g_offsety       dd          0.0
g_zoom          dd          0.4

g_max_dist      dd          10.0
g_range         dd          4.2
g_0_5           dd          0.5
g_0_0           dd          0.0
g_1_0           dd          1.0
g_255_0         dd          255.0

IMG_END:
;--------------------
sequence:
   rb          SEQ_SIZE*8
screen:
   rb          IMG_SIZE*IMG_SIZE*4
memStack:
   rd          1024
I_END: