;
;   Fisheye Raycasting Engine Etc. FREE3D for MENUETOS by Dieter Marfurt
;   Version 0.4 (requires some texture-files to compile (see Data Section))
;   dietermarfurt@angelfire.com - www.melog.ch/mos_pub/
;   Don't hit me - I'm an ASM-Newbie... since years :)
;
;   Compile with FASM for Menuet (requires .INC files - see DATA Section)
;
;   Willow - greatly srinked code size by using GIF texture and FPU to calculate sine table
;
;   !!!! Don't use GIF_LITE.INC in your apps - it's modified for FREE3D !!!!
;
;   Heavyiron - new 0-function of drawing window from kolibri (do not work correctly with menuet)

TEX_SIZE equ 64*64*4
ceil = sinus+16*1024
wall = ceil+TEX_SIZE*1
wall2 = ceil+TEX_SIZE*2
wall3 = ceil+TEX_SIZE*3
wall4 = ceil+TEX_SIZE*4
wall5 = ceil+TEX_SIZE*5
wall6 = ceil+TEX_SIZE*6
wall7 = ceil+TEX_SIZE*7
APP_MEM equ 0x200000

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     APP_MEM;0x100000        ; memory for app
               dd     APP_MEM;0x100000        ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon
include 'lang.inc'
include '..\..\..\macros.inc'
COLOR_ORDER equ OTHER
include 'gif_lite.inc'

START:                          ; start of execution
		mov  esi,textures
		mov  edi,ceil-8
		call ReadGIF
		mov  esi,sinus
		mov  ecx,360*10
		fninit
		fld  [sindegree]
	.sinlp:
		fst  st1
		fsin
		fmul [sindiv]
		fistp dword[esi]
		add  esi,4
		fadd [sininc]
		loop .sinlp
    call draw_window            ; at first, draw the window
    call draw_stuff

gamestart:
;   ******* MOUSE CHECK *******
;    mov eax,37    ; check mouse (use mouse over window to navigate)
;    mov ebx,2     ; check mousebuttons
;    mcall
;    cmp eax,0    ; only use mouse when button down
;    je noneed    ; deactivated cause of disappear-bug etc.
    mov eax,37
    mov ebx,1     ; check mouseposition
    mcall

    mov ebx,eax
    shr eax,16
    and eax,0x0000FFFF  ; mousex
    and ebx,0x0000FFFF  ; mousey

    cmp eax,5  ; mouse out of window ?
    jb check_refresh  ; it will prevent an app-crash
    cmp ebx,22
    jb check_refresh
    cmp eax, 640
    jg check_refresh
    cmp ebx,501
    jg check_refresh

    cmp eax,315 ; navigating?
    jb m_left
    cmp eax,325 ;
    jg m_right
continue:
    cmp ebx,220 ;
    jb s_up
    cmp ebx,260 ;
    jg s_down
;   ******* END OF MOUSE CHECK *******
check_refresh:

;    mov eax,23  ; wait for system event with 10 ms timeout
;    mov ebx,1   ; thats max 100 FPS
    mov eax,11 ; ask no wait for full speed
    mcall

    cmp  eax,1                  ; window redraw request ?
    je   red2
    cmp  eax,2                  ; key in buffer ?
    je   key2
    cmp  eax,3                  ; button in buffer ?
    je   button2

    mov edi,[mouseya] ; check flag if a refresh has to be done
    cmp edi,1
    jne gamestart
    mov [mouseya],dword 0
    call draw_stuff


    jmp gamestart

; END OF MAINLOOP

red2:                          ; redraw
    call draw_window
    call draw_stuff
    jmp  gamestart

key2:                          ; key
    mov  eax,2
    mcall
    cmp  al,1
    je   gamestart     ; keybuffer empty

    cmp ah,27    ; esc=End App
    je finish

    cmp  ah,178  ; up
    je   s_up
    cmp  ah,177  ; down
    je   s_down
    cmp  ah,176  ; left
    je   s_left
    cmp  ah,179  ; right
    je   s_right

    jmp gamestart ; was any other key


s_up:             ; walk forward (key or mouse)
    mov eax,[vpx]
    mov ebx,[vpy]


    mov ecx,[vheading]
    mov edi,[sinus+ecx*4]

    mov edx,[vheading]
;    imul edx,4
;    add edx,sinus
;    add edx,3600
    lea edx, [sinus+3600+edx*4]
    cmp edx,eosinus ;cosinus taken from (sinus plus 900) mod 3600
    jb ok200
    sub edx,14400
    ok200:
    mov esi,[edx]
;    sal esi,1  ; edit walking speed here
;    sal edi,1

    add eax,edi ; newPx
    add ebx,esi ; newPy
    mov edi,eax ; newPx / ffff
    mov esi,ebx ; newPy / ffff
    sar edi,16
    sar esi,16
    mov ecx,esi
    sal ecx,5 ; equal *32
;    add ecx,edi
;    add ecx,grid
    lea ecx, [grid+ecx+edi]
    cmp [ecx],byte 0  ; collision check
    jne cannotwalk0
    mov [vpx],eax
    mov [vpy],ebx
    mov [mouseya],dword 1 ; set refresh flag
cannotwalk0:
    jmp check_refresh

s_down:                    ; walk backward
    mov eax,[vpx]
    mov ebx,[vpy]

    mov ecx,[vheading]
    mov edi,[sinus+ecx*4]

    mov edx,[vheading]
;    imul edx,4
;    add edx,sinus
;    add edx,3600
    lea edx, [sinus+3600+edx*4]
    cmp edx,eosinus ;cosinus taken from (sinus plus 900) mod 3600
    jb ok201
    sub edx,14400
    ok201:

    mov esi,[edx]
;    sal esi,1  ; edit walking speed here
;    sal edi,1

    sub eax,edi ; newPx
    sub ebx,esi ; newPy
    mov edi,eax ; newPx / ffff
    mov esi,ebx ; newPy / ffff
    sar edi,16
    sar esi,16
    mov ecx,esi
    sal ecx,5
;    add ecx,edi
;    add ecx,grid
    lea ecx, [grid+ecx+edi]
    cmp [ecx],byte 0
    jne cannotwalk1
    mov [vpx],eax
    mov [vpy],ebx
    mov [mouseya],dword 1
cannotwalk1:
    jmp check_refresh

s_left:                                   ; turn left (key)
    mov edi,[vheading]  ; heading
    add edi,50
    cmp edi,3600
    jb ok_heading0
    sub edi,3600
    ok_heading0:
    mov [vheading],edi
    mov [mouseya],dword 1
    jmp check_refresh

s_right:                                  ; turn right
    mov edi,[vheading]
    sub edi,50
    cmp edi,-1
    jg ok_heading1
    add edi,3600
    ok_heading1:
    mov [vheading],edi
    mov [mouseya],dword 1
    jmp check_refresh

m_left:                                   ; turn left (mouse)
    mov edi,[vheading]  ; heading
    mov ecx,315
    sub ecx,eax
    sar ecx,2
    add edi,ecx
    cmp edi,3600
    jb ok_heading2
    sub edi,3600
    ok_heading2:
    mov [vheading],edi
    mov [mouseya],dword 1
    jmp continue    ; allow both: walk and rotate

m_right:                                  ; turn right
    mov edi,[vheading]
    sub eax,325
    sar eax,2
    sub edi,eax
    cmp edi,-1
    jg ok_heading3
    add edi,3600
    ok_heading3:
    mov [vheading],edi
    mov [mouseya],dword 1
    jmp continue



  button2:                       ; button
    mov  eax,17                  ; get id
    mcall
    cmp  ah,1                   ; button id=1 ?
    jne  gamestart

; eo GAME mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
 finish:
    mov  eax,-1                 ; close this program
    mcall


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,50*65536+649         ; [x start] *65536 + [x size]
    mov  ecx,50*65536+504         ; [y start] *65536 + [y size]
    mov  edx,0x34ffffff            ; color of work area RRGGBB,8->color gl
    mov  edi,title
    mcall

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret

;   *********************************************
;   *******       COMPUTE 3D-VIEW        ********
;   *********************************************
draw_stuff:


mov [step1],dword 1
;mov [step64],dword 64
    mov esi,[vheading]
    add esi,320
    mov [va],esi
    mov eax,[vheading]
    sub eax,320
    mov [vacompare],eax
;------------------------------------ CAST 640 PIXEL COLUMNS ---------------
; FOR A=320+heading to -319+heading step -1 (a is stored in [va])
;---------------------------------------------------------------------------
;    mov edx,5
    mov [vx1],dword 0  ;5  ;edx        ; init x1 ... pixelcolumn
for_a:
mov edx,[vx1]
mov [vx1b],edx
sub [vx1b],dword 320
    mov edx,[va]  ; a2
    cmp edx,-1   ; a2 is a mod 3600
    jg ok1
    add edx,3600
ok1:
    cmp edx,3600
    jb ok2
    sub edx,3600
ok2:

; get stepx and stepy
;    mov ecx,edx
;    imul ecx,4
;    add ecx,sinus     ; pointer to stepx
    lea ecx, [sinus+edx*4]
    mov esi,[ecx]
    sar esi,4         ; accuracy
    mov [vstepx],esi  ; store stepx

;    mov esi,edx
;    imul esi,4
;    add esi,sinus  ; pointer to stepy
;    add esi,3600
    lea esi, [sinus+3600+edx*4]
    cmp esi,eosinus ;cosinus taken from ((sinus plus 900) mod 3600)
    jb ok202
    sub esi,14400
    ok202:

    mov ecx,[esi]
    sar ecx,4
    mov [vstepy],ecx ; store stepy


    mov eax,[vpx]    ; get Camera Position
    mov ebx,[vpy]
    mov [vxx],eax    ; init caster position
    mov [vyy],ebx

    mov edi,0        ; init L (number of raycsting-steps)
    mov [step1],dword 1  ; init Caster stepwidth for L

 ;  raycast a pixel column.................................
raycast:
    add edi,[step1]  ; count caster steps
;jmp nodouble ; use this to prevent blinking/wobbling textures: much slower!

    cmp edi,32
    je double
    cmp edi,512
    je double
    cmp edi,1024
    je double
    jmp nodouble

    double:
    mov edx,[step1]
    sal edx,1
    mov [step1],edx

    mov edx,[vstepx]
    sal edx,1
    mov [vstepx],edx

    mov edx,[vstepy]
    sal edx,1
    mov [vstepy],edx

nodouble:

    mov eax,32000 ; 3600 ; determine Floors Height based on distance
    mov edx,0
    mov ebx,edi

    div ebx
    mov esi,eax
    mov [vdd],esi
    mov edx,260
    sub edx,esi
    mov [vh],edx

    cmp edx,22
    jb no_nu_pixel
    cmp edx,259
    jg no_nu_pixel ; draw only new pixels
    cmp edx,[h_old]
    je no_nu_pixel

    mov eax,[vxx] ; calc floor pixel
    mov ebx,[vyy]

    and eax,0x0000FFFF
    and ebx,0x0000FFFF

    shr eax,10
    shr ebx,10    ; pixel coords inside Texture x,y 64*64
    mov [xfrac],eax
    mov [yfrac],ebx



    ; plot floor pixel !!!!
    mov [vl],edi      ; save L
    mov [ytemp],esi ; remember L bzw. H

    mov edi,[yfrac] ; get pixel color of this floor pixel
    sal edi,8
    mov esi,[xfrac]
    sal esi,2
;    add edi,esi
;    add edi,wall ; in fact its floor, just using the wall texture :)
    lea edi, [wall+edi+esi]

    mov edx,[edi]
    mov [remesi],esi

    ;**** calculate pixel adress:****
    mov esi,[ytemp]
    add esi,240
    imul esi,1920
;    add esi,[vx1]
;    add esi,[vx1]
;    add esi,[vx1]
;    add esi,0x80000
    mov eax, [vx1]
    lea eax, [eax+eax*2]
    lea esi, [0x80000+eax+esi]

    cmp esi,0x80000+1920*480
    jg foff0
    cmp esi,0x80000
    jb foff0
    ; now we have the adress of the floor-pixel color in edi
    ; and the adress of the pixel in the image in esi

    mov edx,[edi]
    ;******************** custom distance DARKEN Floor

    mov eax,[vdd]

; jmp nodark0 ; use this to deactivate darkening floor (a bit faster)

    cmp eax,80
    jg nodark0
    ;                split rgb

    mov [blue],edx
    and [blue],dword 255

    shr edx,8
    mov [green],edx
    and [green],dword 255

    shr edx,8
    mov [red],edx
    and [red],dword 255

    mov eax,81    ; darkness parameter
    sub eax,[vdd]
    sal eax,1

;                        reduce rgb
    sub [red],eax
    cmp [red], dword 0
    jg notblack10
    mov [red],dword 0
    notblack10:

    sub [green],eax
    cmp [green],dword 0
    jg notblack20
    mov [green],dword 0
    notblack20:

    mov edx,[blue]
    sub [blue],eax
    cmp [blue],dword 0
    jg notblack30
    mov [blue],dword 0
    notblack30:

    shl dword [red],16  ; reassemble rgb
    shl dword [green],8
    mov edx,[red]
    or edx,[green]
    or edx,[blue]

nodark0:
;   eo custom darken floor


    mov eax,edx
    mov [esi],eax ; actually draw the floor pixel

 ; paint "forgotten" pixels

    mov edx,[lasty]
    sub edx,1920
    cmp esi,edx
    je foff0
    mov [esi+1920],eax

    sub edx,1920
    cmp esi,edx
    je foff0
    mov [edx+1920],eax

    sub edx,1920
    cmp esi,edx
    je foff0
    mov [edx+1920],eax

foff0:
mov [lasty],esi
;**** end of draw floor pixel ****

    mov esi,[remesi]
    mov edi,[vl] ; restore L

no_nu_pixel:


    mov esi,[vh]
    mov [h_old],esi

    mov eax,[vxx]
    mov ebx,[vyy]

    add eax,[vstepx]  ; casting...
    add ebx,[vstepy]

    mov [vxx],eax
    mov [vyy],ebx

    sar eax,16
    sar ebx,16

    mov [vpxi],eax    ; casters position in Map Grid
    mov [vpyi],ebx

    mov edx,ebx
;    imul edx,32
    shl edx,5
;    add edx,grid
;    add edx,eax
    lea edx, [grid+edx+eax]

    cmp [edx],byte 0   ; raycaster reached a wall? (0=no)
    jne getout
    cmp edi,10000        ; limit view range
    jb raycast
;................................................
getout:
    mov eax,[edx]      ; store Grid Wall Value for Texture Selection
    mov [vk],eax

 call blur  ; deactivate this (blurs the near floor) : a bit faster

; simply copy floor to ceil pixel column here
;jmp nocopy ; use this for test purposes

    pusha
    mov eax,0x80000+1920*240
    mov ebx,0x80000+1920*240

copyfloor:
    sub eax,1920
    add ebx,1920

;    mov ecx,0
;    add ecx,[vx1]
;    add ecx,[vx1]
;    add ecx,[vx1]
    mov ecx, [vx1]
    lea ecx, [ecx+ecx*2]

;    mov edx,ecx
;    add ecx,eax
;    add edx,ebx
    lea edx, [ecx+ebx]
    add ecx,eax

    mov esi,[edx]
    mov [ecx],esi

    cmp eax,0x80000
    jg copyfloor

    popa
; *** end of copy floor to ceil
;nocopy:
;__________________________________________________________________________


; draw this pixelrows wall
    mov [vl],edi

    mov edi,260
    sub edi,[vdd]
    cmp edi,0
    jg ok3
    xor edi,edi
    ok3:
    mov [vbottom],edi  ; end wall ceil (or window top)

    mov esi,262
    add esi,[vdd]  ; start wall floor

    xor edi,edi

; somethin is wrong with xfrac,so recalc...

    mov eax,[vxx]
    and eax,0x0000FFFF
    shr eax,10
    mov [xfrac],eax

    mov eax,[vyy]
    and eax,0x0000FFFF
    shr eax,10
    mov [yfrac],eax

    pixelrow:

; find each pixels color:

    add edi,64
    sub esi,1
    cmp esi, 502  ; dont calc offscreen-pixels
    jg speedup

    xor edx,edx
    mov eax, edi
    mov ebx,[vdd]
;    add ebx,[vdd]
    add ebx, ebx
    div ebx
    and eax,63
    mov [ytemp],eax   ; get y of texture for wall

    mov eax,[xfrac]
    add eax,[yfrac]

    and eax,63
    mov [xtemp],eax   ; get x of texture for wall

    ; now prepare to plot that wall-pixel...
    mov [remedi],edi

    mov edi,[ytemp]
    sal edi,8
    mov edx,[xtemp]
    sal edx,2
    add edi,edx

    mov eax,[vk] ; determine which texture should be used
    and eax,255

    cmp eax,1
    jne checkmore1
    add edi,ceil
    jmp foundtex
    checkmore1:

    cmp eax,2
    jne checkmore2
    add edi,wall
    jmp foundtex
    checkmore2:

    cmp eax,3
    jne checkmore3
    add edi,wall2
    jmp foundtex
    checkmore3:

    cmp eax,4
    jne checkmore4
    add edi,wall3
    jmp foundtex
    checkmore4:

    cmp eax,5
    jne checkmore5
    add edi,wall4
    jmp foundtex
    checkmore5:

    cmp eax,6
    jne checkmore6
    add edi,wall5
    jmp foundtex
    checkmore6:

    cmp eax,7
    jne checkmore7
    add edi,wall6
    jmp foundtex
    checkmore7:

    cmp eax,8
    jne checkmore8
    add edi,wall7
    jmp foundtex
    checkmore8:

    foundtex:

    mov edx,[edi]    ; get pixel color inside texture

; ***pseudoshade south-west
jmp east ; activate this for southwest pseudoshade : a bit slower + blink-bug
    mov edi,[yfrac]
    mov [pseudo],dword 0 ; store flag for custom distance darkening
    cmp edi,[xfrac]
    jge east
    and edx,0x00FEFEFE
    shr edx,1
    mov [pseudo],dword 1
east:

 call dark_distance ; deactivate wall distance darkening: a bit faster

; ******* DRAW WALL PIXEL *******
    mov eax,esi
;    sub eax,22
    lea eax, [esi-22]
    imul eax,1920
;    add eax,[vx1]
;    add eax,[vx1]
;    add eax,[vx1]
;    add eax,0x80000
    mov ebx, [vx1]
    lea ebx, [ebx+ebx*2]
    lea eax, [eax+0x80000+ebx]

    cmp eax,0x80000+1920*480
    jg dont_draw
    cmp eax,0x80000
    jb dont_draw
    mov [eax],edx ; actually set the pixel in the image
; *** eo draw wall pixel
dont_draw:
    mov edi,[remedi]
speedup:
    cmp esi,[vbottom]  ; end of this column?
    jg pixelrow

    mov edi,[vl]  ; restoring
    mov eax,[vx1] ; inc X1
    add eax,1
    mov [vx1],eax

    ;*** NEXT A ***
    mov esi,[va]
    sub esi,1
    mov [va],esi
    cmp esi,[vacompare]
    jg for_a
    ;*** EO NEXT A ***
;---------------------------------------------------------------------------


; **** put image !!!!!****
; ***********************
    mov eax,7
    mov ebx,0x80000
    mov ecx,640*65536+480
    xor edx,edx
    mcall

    ret

blur:

pusha
mov eax,0x080000+360*1920

copyfloor2:
    add eax,1920
;    mov ebx,eax
;    add ebx,[vx1]
;    add ebx,[vx1]
;    add ebx,[vx1]
    mov ebx,[vx1]
    lea ebx, [ebx+ebx*2]
    add ebx, eax


    mov ecx,[ebx-15]
    and ecx,0x00FEFEFE
    shr ecx,1
    mov edx,[ebx-12]
    and edx,0x00FEFEFE
    shr edx,1
    add edx,ecx
    and edx,0x00FEFEFE
    shr edx,1

     mov ecx,[ebx-9]
     and ecx,0x00FEFEFE
     shr ecx,1
     add edx,ecx

      and edx,0x00FEFEFE
      shr edx,1

      mov ecx,[ebx-6]
      and ecx,0x00FEFEFE
      shr ecx,1
      add edx,ecx

       and edx,0x00FEFEFE
       shr edx,1

       mov ecx,[ebx-3]
       and ecx,0x00FEFEFE
       shr ecx,1
       add edx,ecx

        and edx,0x00FEFEFE
        shr edx,1

        mov ecx,[ebx]
        and ecx,0x00FEFEFE
        shr ecx,1
        add edx,ecx

    mov [ebx],edx

    cmp eax,0x80000+478*1920
    jb copyfloor2

popa

ret



; ******* Darken by Distance *******
dark_distance:

; color must be in edx, wall height in [vdd]

    mov eax,[vdd]
    cmp eax,50
    jg nodark
    ;                split rgb

    mov [blue],edx
    and [blue],dword 255

    shr edx,8
    mov [green],edx
    and [green],dword 255

    shr edx,8
    mov [red],edx
    and [red],dword 255

    mov eax,51    ; darkness parameter
    sub eax,[vdd]
    cmp [pseudo],dword 1
    je isdarkside
    sal eax,2
isdarkside:

;                        reduce rgb
    sub [red],eax
    cmp [red], dword 0
    jg notblack10b
    mov [red],dword 0
    notblack10b:

    sub [green],eax
    cmp [green],dword 0
    jg notblack20b
    mov [green],dword 0
    notblack20b:

    mov edx,[blue]
    sub [blue],eax
    cmp [blue],dword 0
    jg notblack30b
    mov [blue],dword 0
    notblack30b:

    shl dword [red],16 ; reassemble rgb
    shl dword [green],8
    mov edx,[red]
    or edx,[green]
    or edx,[blue]
    mov eax,edx

nodark:

    ret


; DATA AREA

;ceil=ceil
;wall=wall floor
;2 corner stone
;3 leaf mosaic
;4 closed window
;5 greek mosaic
;6 old street stones
;7 maya wall

grid:  ; 32*32 Blocks, Map: 0 = Air, 1 to 8 = Wall
db 2,1,2,1,2,1,2,1,2,1,2,1,1,1,1,1,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
db 1,0,0,0,1,0,0,0,0,0,0,3,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8
db 5,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8
db 1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,1,0,0,0,0,3,3,3,3,0,0,0,0,0,0,8
db 5,0,1,2,3,4,5,6,7,8,2,1,3,3,3,0,5,0,2,1,2,3,0,0,0,0,0,0,0,0,0,8
db 1,0,0,0,0,0,0,0,0,0,2,3,0,0,0,0,5,0,0,0,0,3,0,0,0,0,0,0,0,0,0,8
db 5,0,0,0,1,0,0,4,0,0,0,1,0,0,0,0,5,0,0,0,0,3,3,0,3,3,0,0,0,0,0,8
db 1,1,0,1,1,1,1,4,1,0,1,3,0,0,0,0,5,2,1,2,0,3,0,0,0,3,0,0,0,0,0,8
db 5,0,0,0,1,0,0,0,0,0,0,1,0,3,3,3,5,0,0,0,0,3,0,0,0,3,0,0,0,0,0,8
db 1,0,0,0,1,0,0,5,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,3,0,0,0,0,0,8
db 5,0,0,0,0,0,0,5,0,0,0,1,0,0,0,0,5,0,0,0,0,3,0,0,0,0,0,0,0,0,0,8
db 1,4,4,4,4,4,4,4,4,4,4,3,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,8,8
db 2,2,2,2,2,2,8,8,8,8,8,8,8,8,8,0,0,0,6,6,0,7,7,7,7,7,7,7,7,7,8,8
db 1,0,0,0,1,0,0,0,0,0,0,3,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1
db 5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,2,2,2,2,0,0,0,0,3,3,3,3,3,1
db 1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,6,0,0,0,0,2,0,0,0,0,3,0,0,0,0,1
db 5,0,2,3,2,3,2,3,2,3,2,1,0,0,0,0,6,0,2,2,0,2,0,0,0,0,3,0,5,5,0,1
db 1,0,0,0,0,0,0,4,0,0,0,3,0,0,0,0,6,0,0,2,0,2,0,2,0,0,3,0,0,0,0,1
db 5,0,0,0,1,0,0,4,0,0,0,1,0,0,0,0,6,0,0,2,2,2,0,2,0,0,3,3,3,3,0,1
db 1,1,0,1,1,1,1,4,1,0,1,3,7,7,7,0,6,0,0,0,0,0,0,2,0,0,0,0,0,3,0,1
db 5,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,6,0,0,0,0,2,2,2,0,0,0,0,0,3,0,1
db 1,0,0,0,1,0,0,5,0,0,0,3,0,0,0,0,6,0,0,0,0,2,0,0,0,0,0,0,0,0,0,1
db 5,0,0,0,0,0,0,5,0,0,0,1,0,0,0,0,6,0,5,1,0,2,0,0,4,4,0,4,4,0,0,1
db 1,4,1,4,1,4,1,4,1,4,1,3,0,0,0,0,6,0,0,5,0,2,0,0,0,4,0,4,0,0,0,1
db 1,0,0,0,0,0,0,4,0,0,0,3,0,3,3,3,6,0,0,1,0,1,0,0,4,4,0,4,4,0,0,1
db 5,0,0,0,1,0,0,4,0,0,0,1,0,0,0,0,6,0,0,5,0,1,0,4,4,0,0,0,4,4,0,1
db 1,1,0,1,1,1,1,4,1,0,1,3,0,0,0,0,6,0,0,1,0,1,0,4,0,0,0,0,0,4,0,1
db 5,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,6,0,0,5,0,1,0,4,0,0,0,0,0,4,0,1
db 1,0,0,0,1,0,0,5,0,0,0,3,0,0,0,0,6,1,5,1,0,1,0,4,4,0,0,0,4,4,0,1
db 5,0,0,0,0,0,0,5,0,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,4,4,4,4,4,0,0,1
db 1,4,1,4,1,4,1,4,1,4,1,3,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1
db 2,1,2,1,2,1,2,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1

vpx:
dd 0x0001FFFF ; initial player position * 0xFFFF
 vpy:
dd 0x0001FFFF

title    db   'FISHEYE RAYCASTING ENGINE ETC. FREE3D',0

sindegree dd 0.0
sininc    dd 0.0017453292519943295769236907684886
sindiv    dd 6553.5
textures:
	file 'texture.gif'

align 4

col1:
 dd ?;-
; misc raycaster vars:
vxx:
 dd ?;-
vyy:
 dd ?;-
vl:
 dd ?;-
vstepx:
 dd ?;-
vstepy:
 dd ?;-
vxxint:
 dd ?;-
vyyint:
 dd ?;-
vk:
 dd ?;-
va:
 dd ?;-
va2:
 dd ?;-
vdd:
 dd ?;-
vx1:
 dd ?;-
vx1b:
 dd ?;-
vh:
 dd ?;-
vdt:
 dd ?;-
vheading: ; initial heading: 0 to 3599
 dd ?;-
vacompare:
 dd ?;-
vpxi:
 dd ?;-
vpyi:
 dd ?;-
wtolong:
 dw ?,?;-,?;-

xtemp:
 dd ?;-
ytemp:
 dd ?;-
xfrac:
 dd ?;-
yfrac:
 dd ?;-
h_old:
 dd ?;-
vbottom:
 dd ?;-
mouseya:
 dd ?;-
remeax:
 dd ?;-
remebx:
 dd ?;-
remecx:
 dd ?;-
remedx:
 dd ?;-
remedi:
 dd ?;-
remesi:
 dd ?;-
red:
 dd ?;-
green:
 dd ?;-
blue:
 dd ?;-
pseudo:
 dd ?;-
step1:
 dd ?;-
step64:
 dd ?;-
lasty:
 dd ?;-

I_END:
IncludeUGlobals
sinus rd 360*10
eosinus:
