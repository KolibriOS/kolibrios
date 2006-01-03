;
;   Fisheye Raycasting Engine Etc. FREE3D for MENUETOS by Dieter Marfurt
;   Version 0.4 (requires some texture-files to compile (see Data Section))
;   dietermarfurt@angelfire.com - www.melog.ch/mos_pub/
;   Don't hit me - I'm an ASM-Newbie... since years :)
;
;   Compile with FASM for Menuet (requires .INC files - see DATA Section)
;
   
use32
   
               org    0x0
   
               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x300000                ; memory for app
               dd     0x7fff0                 ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon
   
include 'lang.inc'   
START:                          ; start of execution
   
    call draw_window            ; at first, draw the window
    call draw_stuff
   
gamestart:
;   ******* MOUSE CHECK *******
;    mov eax,37    ; check mouse (use mouse over window to navigate)
;    mov ebx,2     ; check mousebuttons
;    int 0x40
;    cmp eax,0    ; only use mouse when button down
;    je noneed    ; deactivated cause of disappear-bug etc.
    mov eax,37
    mov ebx,1     ; check mouseposition
    int 0x40
   
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
    int  0x40
   
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
    int  0x40
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
    imul ecx,4
    add ecx,sinus
    mov edi,[ecx]
   
    mov edx,[vheading]
    imul edx,4
    add edx,sinus
    add edx,3600
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
    add ecx,edi
    add ecx,grid
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
    imul ecx,4
    add ecx,sinus
    mov edi,[ecx]
   
    mov edx,[vheading]
    imul edx,4
    add edx,sinus
    add edx,3600
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
    add ecx,edi
    add ecx,grid
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
    int  0x40
    cmp  ah,1                   ; button id=1 ?
    jne  gamestart
   
; eo GAME mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
 finish:
    mov  eax,-1                 ; close this program
    int  0x40
   
   
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
   
   
draw_window:
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40
   
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,50*65536+649         ; [x start] *65536 + [x size]
    mov  ecx,50*65536+504         ; [y start] *65536 + [y size]
    mov  edx,0x02ffffff            ; color of work area RRGGBB,8->color gl
    mov  esi,0x80777777            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x00777777            ; color of frames    RRGGBB
    int  0x40
   
                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ddeeff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40
   
                                   ; CLOSE BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,(649-19)*65536+12     ; [x start] *65536 + [x size]
    mov  ecx,5*65536+12            ; [y start] *65536 + [y size]
    mov  edx,1                     ; button id
    mov  esi,0x777777              ; button color RRGGBB
    int  0x40
   
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40
   
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
    mov ecx,edx
    imul ecx,4
    add ecx,sinus     ; pointer to stepx
    mov esi,[ecx]
    sar esi,4         ; accuracy
    mov [vstepx],esi  ; store stepx
   
    mov esi,edx
    imul esi,4
    add esi,sinus  ; pointer to stepy
    add esi,3600
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
    add edi,esi
    add edi,wall ; in fact its floor, just using the wall texture :)
    mov edx,[edi]
    mov [remesi],esi
   
    ;**** calculate pixel adress:****
    mov esi,[ytemp]
    add esi,240
    imul esi,1920
    add esi,[vx1]
    add esi,[vx1]
    add esi,[vx1]
    add esi,0x80000
   
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
    add edx,grid
    add edx,eax
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
   
    mov ecx,0
    add ecx,[vx1]
    add ecx,[vx1]
    add ecx,[vx1]
   
    mov edx,ecx
    add ecx,eax
    add edx,ebx
   
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
    add ebx,[vdd]
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
    sub eax,22
    imul eax,1920
    add eax,[vx1]
    add eax,[vx1]
    add eax,[vx1]
    add eax,0x80000
   
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
    mov edx,5*65536+20
    int 0x40
   
    ret
   
blur:
   
pusha
mov eax,0x080000+360*1920
   
copyfloor2:
    add eax,1920
    mov ebx,eax
    add ebx,[vx1]
    add ebx,[vx1]
    add ebx,[vx1]
   
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
   
   
sinus:
dd 0,11,23,34,46,57,69,80,92,103
dd 114,126,137,149,160,172,183,194,206,217
dd 229,240,252,263,274,286,297,309,320,332
dd 343,354,366,377,389,400,411,423,434,446
dd 457,469,480,491,503,514,526,537,548,560
dd 571,583,594,605,617,628,640,651,662,674
dd 685,696,708,719,731,742,753,765,776,787
dd 799,810,821,833,844,855,867,878,889,901
dd 912,923,935,946,957,969,980,991,1003,1014
dd 1025,1036,1048,1059,1070,1082,1093,1104,1115,1127
dd 1138,1149,1161,1172,1183,1194,1206,1217,1228,1239
dd 1250,1262,1273,1284,1295,1307,1318,1329,1340,1351
dd 1363,1374,1385,1396,1407,1418,1430,1441,1452,1463
dd 1474,1485,1496,1508,1519,1530,1541,1552,1563,1574
dd 1585,1597,1608,1619,1630,1641,1652,1663,1674,1685
dd 1696,1707,1718,1729,1740,1751,1762,1773,1784,1795
dd 1806,1817,1828,1839,1850,1861,1872,1883,1894,1905
dd 1916,1927,1938,1949,1960,1971,1982,1992,2003,2014
dd 2025,2036,2047,2058,2069,2079,2090,2101,2112,2123
dd 2134,2144,2155,2166,2177,2188,2198,2209,2220,2231
dd 2241,2252,2263,2274,2284,2295,2306,2316,2327,2338
dd 2349,2359,2370,2381,2391,2402,2413,2423,2434,2444
dd 2455,2466,2476,2487,2497,2508,2518,2529,2540,2550
dd 2561,2571,2582,2592,2603,2613,2624,2634,2645,2655
dd 2666,2676,2686,2697,2707,2718,2728,2738,2749,2759
dd 2770,2780,2790,2801,2811,2821,2832,2842,2852,2863
dd 2873,2883,2893,2904,2914,2924,2934,2945,2955,2965
dd 2975,2985,2996,3006,3016,3026,3036,3046,3056,3067
dd 3077,3087,3097,3107,3117,3127,3137,3147,3157,3167
dd 3177,3187,3197,3207,3217,3227,3237,3247,3257,3267
dd 3277,3287,3297,3306,3316,3326,3336,3346,3356,3365
dd 3375,3385,3395,3405,3414,3424,3434,3444,3453,3463
dd 3473,3483,3492,3502,3512,3521,3531,3540,3550,3560
dd 3569,3579,3588,3598,3608,3617,3627,3636,3646,3655
dd 3665,3674,3684,3693,3703,3712,3721,3731,3740,3750
dd 3759,3768,3778,3787,3796,3806,3815,3824,3834,3843
dd 3852,3861,3871,3880,3889,3898,3907,3917,3926,3935
dd 3944,3953,3962,3971,3980,3990,3999,4008,4017,4026
dd 4035,4044,4053,4062,4071,4080,4089,4098,4106,4115
dd 4124,4133,4142,4151,4160,4169,4177,4186,4195,4204
dd 4213,4221,4230,4239,4247,4256,4265,4274,4282,4291
dd 4299,4308,4317,4325,4334,4342,4351,4360,4368,4377
dd 4385,4394,4402,4411,4419,4427,4436,4444,4453,4461
dd 4469,4478,4486,4495,4503,4511,4519,4528,4536,4544
dd 4552,4561,4569,4577,4585,4593,4602,4610,4618,4626
dd 4634,4642,4650,4658,4666,4674,4682,4690,4698,4706
dd 4714,4722,4730,4738,4746,4754,4762,4769,4777,4785
dd 4793,4801,4808,4816,4824,4832,4839,4847,4855,4863
dd 4870,4878,4885,4893,4901,4908,4916,4923,4931,4938
dd 4946,4953,4961,4968,4976,4983,4991,4998,5006,5013
dd 5020,5028,5035,5042,5050,5057,5064,5071,5079,5086
dd 5093,5100,5107,5115,5122,5129,5136,5143,5150,5157
dd 5164,5171,5178,5185,5192,5199,5206,5213,5220,5227
dd 5234,5241,5248,5254,5261,5268,5275,5282,5288,5295
dd 5302,5309,5315,5322,5329,5335,5342,5349,5355,5362
dd 5368,5375,5381,5388,5394,5401,5407,5414,5420,5427
dd 5433,5439,5446,5452,5459,5465,5471,5477,5484,5490
dd 5496,5502,5509,5515,5521,5527,5533,5539,5546,5552
dd 5558,5564,5570,5576,5582,5588,5594,5600,5606,5612
dd 5617,5623,5629,5635,5641,5647,5652,5658,5664,5670
dd 5675,5681,5687,5693,5698,5704,5709,5715,5721,5726
dd 5732,5737,5743,5748,5754,5759,5765,5770,5776,5781
dd 5786,5792,5797,5802,5808,5813,5818,5824,5829,5834
dd 5839,5844,5850,5855,5860,5865,5870,5875,5880,5885
dd 5890,5895,5900,5905,5910,5915,5920,5925,5930,5935
dd 5939,5944,5949,5954,5959,5963,5968,5973,5978,5982
dd 5987,5992,5996,6001,6005,6010,6015,6019,6024,6028
dd 6033,6037,6041,6046,6050,6055,6059,6063,6068,6072
dd 6076,6081,6085,6089,6093,6097,6102,6106,6110,6114
dd 6118,6122,6126,6130,6134,6138,6142,6146,6150,6154
dd 6158,6162,6166,6170,6174,6178,6181,6185,6189,6193
dd 6196,6200,6204,6208,6211,6215,6218,6222,6226,6229
dd 6233,6236,6240,6243,6247,6250,6254,6257,6260,6264
dd 6267,6270,6274,6277,6280,6284,6287,6290,6293,6296
dd 6300,6303,6306,6309,6312,6315,6318,6321,6324,6327
dd 6330,6333,6336,6339,6342,6345,6348,6350,6353,6356
dd 6359,6362,6364,6367,6370,6372,6375,6378,6380,6383
dd 6386,6388,6391,6393,6396,6398,6401,6403,6405,6408
dd 6410,6413,6415,6417,6420,6422,6424,6426,6429,6431
dd 6433,6435,6437,6440,6442,6444,6446,6448,6450,6452
dd 6454,6456,6458,6460,6462,6464,6466,6467,6469,6471
dd 6473,6475,6476,6478,6480,6482,6483,6485,6486,6488
dd 6490,6491,6493,6494,6496,6497,6499,6500,6502,6503
dd 6505,6506,6507,6509,6510,6511,6513,6514,6515,6516
dd 6518,6519,6520,6521,6522,6523,6524,6525,6527,6528
dd 6529,6530,6531,6531,6532,6533,6534,6535,6536,6537
dd 6538,6538,6539,6540,6541,6541,6542,6543,6543,6544
dd 6545,6545,6546,6546,6547,6547,6548,6548,6549,6549
dd 6550,6550,6550,6551,6551,6551,6552,6552,6552,6552
dd 6553,6553,6553,6553,6553,6553,6553,6553,6553,6553
dd 6554,6553,6553,6553,6553,6553,6553,6553,6553,6553
dd 6553,6552,6552,6552,6552,6551,6551,6551,6550,6550
dd 6550,6549,6549,6548,6548,6547,6547,6546,6546,6545
dd 6545,6544,6543,6543,6542,6541,6541,6540,6539,6538
dd 6538,6537,6536,6535,6534,6533,6532,6531,6531,6530
dd 6529,6528,6527,6525,6524,6523,6522,6521,6520,6519
dd 6518,6516,6515,6514,6513,6511,6510,6509,6507,6506
dd 6505,6503,6502,6500,6499,6497,6496,6494,6493,6491
dd 6490,6488,6486,6485,6483,6482,6480,6478,6476,6475
dd 6473,6471,6469,6467,6466,6464,6462,6460,6458,6456
dd 6454,6452,6450,6448,6446,6444,6442,6440,6437,6435
dd 6433,6431,6429,6426,6424,6422,6420,6417,6415,6413
dd 6410,6408,6405,6403,6401,6398,6396,6393,6391,6388
dd 6386,6383,6380,6378,6375,6372,6370,6367,6364,6362
dd 6359,6356,6353,6350,6348,6345,6342,6339,6336,6333
dd 6330,6327,6324,6321,6318,6315,6312,6309,6306,6303
dd 6300,6296,6293,6290,6287,6284,6280,6277,6274,6270
dd 6267,6264,6260,6257,6254,6250,6247,6243,6240,6236
dd 6233,6229,6226,6222,6218,6215,6211,6208,6204,6200
dd 6196,6193,6189,6185,6181,6178,6174,6170,6166,6162
dd 6158,6154,6150,6146,6142,6138,6134,6130,6126,6122
dd 6118,6114,6110,6106,6102,6097,6093,6089,6085,6081
dd 6076,6072,6068,6063,6059,6055,6050,6046,6041,6037
dd 6033,6028,6024,6019,6015,6010,6005,6001,5996,5992
dd 5987,5982,5978,5973,5968,5963,5959,5954,5949,5944
dd 5939,5935,5930,5925,5920,5915,5910,5905,5900,5895
dd 5890,5885,5880,5875,5870,5865,5860,5855,5850,5844
dd 5839,5834,5829,5824,5818,5813,5808,5802,5797,5792
dd 5786,5781,5776,5770,5765,5759,5754,5748,5743,5737
dd 5732,5726,5721,5715,5709,5704,5698,5693,5687,5681
dd 5675,5670,5664,5658,5652,5647,5641,5635,5629,5623
dd 5617,5612,5606,5600,5594,5588,5582,5576,5570,5564
dd 5558,5552,5546,5539,5533,5527,5521,5515,5509,5502
dd 5496,5490,5484,5477,5471,5465,5459,5452,5446,5439
dd 5433,5427,5420,5414,5407,5401,5394,5388,5381,5375
dd 5368,5362,5355,5349,5342,5335,5329,5322,5315,5309
dd 5302,5295,5288,5282,5275,5268,5261,5254,5248,5241
dd 5234,5227,5220,5213,5206,5199,5192,5185,5178,5171
dd 5164,5157,5150,5143,5136,5129,5122,5115,5107,5100
dd 5093,5086,5079,5071,5064,5057,5050,5042,5035,5028
dd 5020,5013,5006,4998,4991,4983,4976,4968,4961,4953
dd 4946,4938,4931,4923,4916,4908,4901,4893,4885,4878
dd 4870,4863,4855,4847,4839,4832,4824,4816,4808,4801
dd 4793,4785,4777,4769,4762,4754,4746,4738,4730,4722
dd 4714,4706,4698,4690,4682,4674,4666,4658,4650,4642
dd 4634,4626,4618,4610,4602,4593,4585,4577,4569,4561
dd 4552,4544,4536,4528,4519,4511,4503,4495,4486,4478
dd 4469,4461,4453,4444,4436,4427,4419,4411,4402,4394
dd 4385,4377,4368,4360,4351,4342,4334,4325,4317,4308
dd 4299,4291,4282,4274,4265,4256,4247,4239,4230,4221
dd 4213,4204,4195,4186,4177,4169,4160,4151,4142,4133
dd 4124,4115,4106,4098,4089,4080,4071,4062,4053,4044
dd 4035,4026,4017,4008,3999,3990,3980,3971,3962,3953
dd 3944,3935,3926,3917,3907,3898,3889,3880,3871,3861
dd 3852,3843,3834,3824,3815,3806,3796,3787,3778,3768
dd 3759,3750,3740,3731,3721,3712,3703,3693,3684,3674
dd 3665,3655,3646,3636,3627,3617,3608,3598,3588,3579
dd 3569,3560,3550,3540,3531,3521,3512,3502,3492,3483
dd 3473,3463,3453,3444,3434,3424,3414,3405,3395,3385
dd 3375,3365,3356,3346,3336,3326,3316,3306,3297,3287
dd 3277,3267,3257,3247,3237,3227,3217,3207,3197,3187
dd 3177,3167,3157,3147,3137,3127,3117,3107,3097,3087
dd 3077,3067,3056,3046,3036,3026,3016,3006,2996,2985
dd 2975,2965,2955,2945,2934,2924,2914,2904,2893,2883
dd 2873,2863,2852,2842,2832,2821,2811,2801,2790,2780
dd 2770,2759,2749,2738,2728,2718,2707,2697,2686,2676
dd 2666,2655,2645,2634,2624,2613,2603,2592,2582,2571
dd 2561,2550,2540,2529,2518,2508,2497,2487,2476,2466
dd 2455,2444,2434,2423,2413,2402,2391,2381,2370,2359
dd 2349,2338,2327,2316,2306,2295,2284,2274,2263,2252
dd 2241,2231,2220,2209,2198,2188,2177,2166,2155,2144
dd 2134,2123,2112,2101,2090,2079,2069,2058,2047,2036
dd 2025,2014,2003,1992,1982,1971,1960,1949,1938,1927
dd 1916,1905,1894,1883,1872,1861,1850,1839,1828,1817
dd 1806,1795,1784,1773,1762,1751,1740,1729,1718,1707
dd 1696,1685,1674,1663,1652,1641,1630,1619,1608,1597
dd 1585,1574,1563,1552,1541,1530,1519,1508,1496,1485
dd 1474,1463,1452,1441,1430,1418,1407,1396,1385,1374
dd 1363,1351,1340,1329,1318,1307,1295,1284,1273,1262
dd 1250,1239,1228,1217,1206,1194,1183,1172,1161,1149
dd 1138,1127,1115,1104,1093,1082,1070,1059,1048,1036
dd 1025,1014,1003,991,980,969,957,946,935,923
dd 912,901,889,878,867,855,844,833,821,810
dd 799,787,776,765,753,742,731,719,708,696
dd 685,674,662,651,640,628,617,605,594,583
dd 571,560,548,537,526,514,503,491,480,469
dd 457,446,434,423,411,400,389,377,366,354
dd 343,332,320,309,297,286,274,263,252,240
dd 229,217,206,194,183,172,160,149,137,126
dd 114,103,92,80,69,57,46,34,23,11
dd 0,-11,-23,-34,-46,-57,-69,-80,-92,-103
dd -114,-126,-137,-149,-160,-172,-183,-194,-206,-217
dd -229,-240,-252,-263,-274,-286,-297,-309,-320,-332
dd -343,-354,-366,-377,-389,-400,-411,-423,-434,-446
dd -457,-469,-480,-491,-503,-514,-526,-537,-548,-560
dd -571,-583,-594,-605,-617,-628,-640,-651,-662,-674
dd -685,-696,-708,-719,-731,-742,-753,-765,-776,-787
dd -799,-810,-821,-833,-844,-855,-867,-878,-889,-901
dd -912,-923,-935,-946,-957,-969,-980,-991,-1003,-1014
dd -1025,-1036,-1048,-1059,-1070,-1082,-1093,-1104,-1115,-1127
dd -1138,-1149,-1161,-1172,-1183,-1194,-1206,-1217,-1228,-1239
dd -1250,-1262,-1273,-1284,-1295,-1307,-1318,-1329,-1340,-1351
dd -1363,-1374,-1385,-1396,-1407,-1418,-1430,-1441,-1452,-1463
dd -1474,-1485,-1496,-1508,-1519,-1530,-1541,-1552,-1563,-1574
dd -1585,-1597,-1608,-1619,-1630,-1641,-1652,-1663,-1674,-1685
dd -1696,-1707,-1718,-1729,-1740,-1751,-1762,-1773,-1784,-1795
dd -1806,-1817,-1828,-1839,-1850,-1861,-1872,-1883,-1894,-1905
dd -1916,-1927,-1938,-1949,-1960,-1971,-1982,-1992,-2003,-2014
dd -2025,-2036,-2047,-2058,-2069,-2079,-2090,-2101,-2112,-2123
dd -2134,-2144,-2155,-2166,-2177,-2188,-2198,-2209,-2220,-2231
dd -2241,-2252,-2263,-2274,-2284,-2295,-2306,-2316,-2327,-2338
dd -2349,-2359,-2370,-2381,-2391,-2402,-2413,-2423,-2434,-2444
dd -2455,-2466,-2476,-2487,-2497,-2508,-2518,-2529,-2540,-2550
dd -2561,-2571,-2582,-2592,-2603,-2613,-2624,-2634,-2645,-2655
dd -2666,-2676,-2686,-2697,-2707,-2718,-2728,-2738,-2749,-2759
dd -2770,-2780,-2790,-2801,-2811,-2821,-2832,-2842,-2852,-2863
dd -2873,-2883,-2893,-2904,-2914,-2924,-2934,-2945,-2955,-2965
dd -2975,-2985,-2996,-3006,-3016,-3026,-3036,-3046,-3056,-3067
dd -3077,-3087,-3097,-3107,-3117,-3127,-3137,-3147,-3157,-3167
dd -3177,-3187,-3197,-3207,-3217,-3227,-3237,-3247,-3257,-3267
dd -3277,-3287,-3297,-3306,-3316,-3326,-3336,-3346,-3356,-3365
dd -3375,-3385,-3395,-3405,-3414,-3424,-3434,-3444,-3453,-3463
dd -3473,-3483,-3492,-3502,-3512,-3521,-3531,-3540,-3550,-3560
dd -3569,-3579,-3588,-3598,-3608,-3617,-3627,-3636,-3646,-3655
dd -3665,-3674,-3684,-3693,-3703,-3712,-3721,-3731,-3740,-3750
dd -3759,-3768,-3778,-3787,-3796,-3806,-3815,-3824,-3834,-3843
dd -3852,-3861,-3871,-3880,-3889,-3898,-3907,-3917,-3926,-3935
dd -3944,-3953,-3962,-3971,-3980,-3990,-3999,-4008,-4017,-4026
dd -4035,-4044,-4053,-4062,-4071,-4080,-4089,-4098,-4106,-4115
dd -4124,-4133,-4142,-4151,-4160,-4169,-4177,-4186,-4195,-4204
dd -4213,-4221,-4230,-4239,-4247,-4256,-4265,-4274,-4282,-4291
dd -4299,-4308,-4317,-4325,-4334,-4342,-4351,-4360,-4368,-4377
dd -4385,-4394,-4402,-4411,-4419,-4427,-4436,-4444,-4453,-4461
dd -4469,-4478,-4486,-4495,-4503,-4511,-4519,-4528,-4536,-4544
dd -4552,-4561,-4569,-4577,-4585,-4593,-4602,-4610,-4618,-4626
dd -4634,-4642,-4650,-4658,-4666,-4674,-4682,-4690,-4698,-4706
dd -4714,-4722,-4730,-4738,-4746,-4754,-4762,-4769,-4777,-4785
dd -4793,-4801,-4808,-4816,-4824,-4832,-4839,-4847,-4855,-4863
dd -4870,-4878,-4885,-4893,-4901,-4908,-4916,-4923,-4931,-4938
dd -4946,-4953,-4961,-4968,-4976,-4983,-4991,-4998,-5006,-5013
dd -5020,-5028,-5035,-5042,-5050,-5057,-5064,-5071,-5079,-5086
dd -5093,-5100,-5107,-5115,-5122,-5129,-5136,-5143,-5150,-5157
dd -5164,-5171,-5178,-5185,-5192,-5199,-5206,-5213,-5220,-5227
dd -5234,-5241,-5248,-5254,-5261,-5268,-5275,-5282,-5288,-5295
dd -5302,-5309,-5315,-5322,-5329,-5335,-5342,-5349,-5355,-5362
dd -5368,-5375,-5381,-5388,-5394,-5401,-5407,-5414,-5420,-5427
dd -5433,-5439,-5446,-5452,-5459,-5465,-5471,-5477,-5484,-5490
dd -5496,-5502,-5509,-5515,-5521,-5527,-5533,-5539,-5546,-5552
dd -5558,-5564,-5570,-5576,-5582,-5588,-5594,-5600,-5606,-5612
dd -5617,-5623,-5629,-5635,-5641,-5647,-5652,-5658,-5664,-5670
dd -5675,-5681,-5687,-5693,-5698,-5704,-5709,-5715,-5721,-5726
dd -5732,-5737,-5743,-5748,-5754,-5759,-5765,-5770,-5776,-5781
dd -5786,-5792,-5797,-5802,-5808,-5813,-5818,-5824,-5829,-5834
dd -5839,-5844,-5850,-5855,-5860,-5865,-5870,-5875,-5880,-5885
dd -5890,-5895,-5900,-5905,-5910,-5915,-5920,-5925,-5930,-5935
dd -5939,-5944,-5949,-5954,-5959,-5963,-5968,-5973,-5978,-5982
dd -5987,-5992,-5996,-6001,-6005,-6010,-6015,-6019,-6024,-6028
dd -6033,-6037,-6041,-6046,-6050,-6055,-6059,-6063,-6068,-6072
dd -6076,-6081,-6085,-6089,-6093,-6097,-6102,-6106,-6110,-6114
dd -6118,-6122,-6126,-6130,-6134,-6138,-6142,-6146,-6150,-6154
dd -6158,-6162,-6166,-6170,-6174,-6178,-6181,-6185,-6189,-6193
dd -6196,-6200,-6204,-6208,-6211,-6215,-6218,-6222,-6226,-6229
dd -6233,-6236,-6240,-6243,-6247,-6250,-6254,-6257,-6260,-6264
dd -6267,-6270,-6274,-6277,-6280,-6284,-6287,-6290,-6293,-6296
dd -6300,-6303,-6306,-6309,-6312,-6315,-6318,-6321,-6324,-6327
dd -6330,-6333,-6336,-6339,-6342,-6345,-6348,-6350,-6353,-6356
dd -6359,-6362,-6364,-6367,-6370,-6372,-6375,-6378,-6380,-6383
dd -6386,-6388,-6391,-6393,-6396,-6398,-6401,-6403,-6405,-6408
dd -6410,-6413,-6415,-6417,-6420,-6422,-6424,-6426,-6429,-6431
dd -6433,-6435,-6437,-6440,-6442,-6444,-6446,-6448,-6450,-6452
dd -6454,-6456,-6458,-6460,-6462,-6464,-6466,-6467,-6469,-6471
dd -6473,-6475,-6476,-6478,-6480,-6482,-6483,-6485,-6486,-6488
dd -6490,-6491,-6493,-6494,-6496,-6497,-6499,-6500,-6502,-6503
dd -6505,-6506,-6507,-6509,-6510,-6511,-6513,-6514,-6515,-6516
dd -6518,-6519,-6520,-6521,-6522,-6523,-6524,-6525,-6527,-6528
dd -6529,-6530,-6531,-6531,-6532,-6533,-6534,-6535,-6536,-6537
dd -6538,-6538,-6539,-6540,-6541,-6541,-6542,-6543,-6543,-6544
dd -6545,-6545,-6546,-6546,-6547,-6547,-6548,-6548,-6549,-6549
dd -6550,-6550,-6550,-6551,-6551,-6551,-6552,-6552,-6552,-6552
dd -6553,-6553,-6553,-6553,-6553,-6553,-6553,-6553,-6553,-6553
dd -6554,-6553,-6553,-6553,-6553,-6553,-6553,-6553,-6553,-6553
dd -6553,-6552,-6552,-6552,-6552,-6551,-6551,-6551,-6550,-6550
dd -6550,-6549,-6549,-6548,-6548,-6547,-6547,-6546,-6546,-6545
dd -6545,-6544,-6543,-6543,-6542,-6541,-6541,-6540,-6539,-6538
dd -6538,-6537,-6536,-6535,-6534,-6533,-6532,-6531,-6531,-6530
dd -6529,-6528,-6527,-6525,-6524,-6523,-6522,-6521,-6520,-6519
dd -6518,-6516,-6515,-6514,-6513,-6511,-6510,-6509,-6507,-6506
dd -6505,-6503,-6502,-6500,-6499,-6497,-6496,-6494,-6493,-6491
dd -6490,-6488,-6486,-6485,-6483,-6482,-6480,-6478,-6476,-6475
dd -6473,-6471,-6469,-6467,-6466,-6464,-6462,-6460,-6458,-6456
dd -6454,-6452,-6450,-6448,-6446,-6444,-6442,-6440,-6437,-6435
dd -6433,-6431,-6429,-6426,-6424,-6422,-6420,-6417,-6415,-6413
dd -6410,-6408,-6405,-6403,-6401,-6398,-6396,-6393,-6391,-6388
dd -6386,-6383,-6380,-6378,-6375,-6372,-6370,-6367,-6364,-6362
dd -6359,-6356,-6353,-6350,-6348,-6345,-6342,-6339,-6336,-6333
dd -6330,-6327,-6324,-6321,-6318,-6315,-6312,-6309,-6306,-6303
dd -6300,-6296,-6293,-6290,-6287,-6284,-6280,-6277,-6274,-6270
dd -6267,-6264,-6260,-6257,-6254,-6250,-6247,-6243,-6240,-6236
dd -6233,-6229,-6226,-6222,-6218,-6215,-6211,-6208,-6204,-6200
dd -6196,-6193,-6189,-6185,-6181,-6178,-6174,-6170,-6166,-6162
dd -6158,-6154,-6150,-6146,-6142,-6138,-6134,-6130,-6126,-6122
dd -6118,-6114,-6110,-6106,-6102,-6097,-6093,-6089,-6085,-6081
dd -6076,-6072,-6068,-6063,-6059,-6055,-6050,-6046,-6041,-6037
dd -6033,-6028,-6024,-6019,-6015,-6010,-6005,-6001,-5996,-5992
dd -5987,-5982,-5978,-5973,-5968,-5963,-5959,-5954,-5949,-5944
dd -5939,-5935,-5930,-5925,-5920,-5915,-5910,-5905,-5900,-5895
dd -5890,-5885,-5880,-5875,-5870,-5865,-5860,-5855,-5850,-5844
dd -5839,-5834,-5829,-5824,-5818,-5813,-5808,-5802,-5797,-5792
dd -5786,-5781,-5776,-5770,-5765,-5759,-5754,-5748,-5743,-5737
dd -5732,-5726,-5721,-5715,-5709,-5704,-5698,-5693,-5687,-5681
dd -5675,-5670,-5664,-5658,-5652,-5647,-5641,-5635,-5629,-5623
dd -5617,-5612,-5606,-5600,-5594,-5588,-5582,-5576,-5570,-5564
dd -5558,-5552,-5546,-5539,-5533,-5527,-5521,-5515,-5509,-5502
dd -5496,-5490,-5484,-5477,-5471,-5465,-5459,-5452,-5446,-5439
dd -5433,-5427,-5420,-5414,-5407,-5401,-5394,-5388,-5381,-5375
dd -5368,-5362,-5355,-5349,-5342,-5335,-5329,-5322,-5315,-5309
dd -5302,-5295,-5288,-5282,-5275,-5268,-5261,-5254,-5248,-5241
dd -5234,-5227,-5220,-5213,-5206,-5199,-5192,-5185,-5178,-5171
dd -5164,-5157,-5150,-5143,-5136,-5129,-5122,-5115,-5107,-5100
dd -5093,-5086,-5079,-5071,-5064,-5057,-5050,-5042,-5035,-5028
dd -5020,-5013,-5006,-4998,-4991,-4983,-4976,-4968,-4961,-4953
dd -4946,-4938,-4931,-4923,-4916,-4908,-4901,-4893,-4885,-4878
dd -4870,-4863,-4855,-4847,-4839,-4832,-4824,-4816,-4808,-4801
dd -4793,-4785,-4777,-4769,-4762,-4754,-4746,-4738,-4730,-4722
dd -4714,-4706,-4698,-4690,-4682,-4674,-4666,-4658,-4650,-4642
dd -4634,-4626,-4618,-4610,-4602,-4593,-4585,-4577,-4569,-4561
dd -4552,-4544,-4536,-4528,-4519,-4511,-4503,-4495,-4486,-4478
dd -4469,-4461,-4453,-4444,-4436,-4427,-4419,-4411,-4402,-4394
dd -4385,-4377,-4368,-4360,-4351,-4342,-4334,-4325,-4317,-4308
dd -4299,-4291,-4282,-4274,-4265,-4256,-4247,-4239,-4230,-4221
dd -4213,-4204,-4195,-4186,-4177,-4169,-4160,-4151,-4142,-4133
dd -4124,-4115,-4106,-4098,-4089,-4080,-4071,-4062,-4053,-4044
dd -4035,-4026,-4017,-4008,-3999,-3990,-3980,-3971,-3962,-3953
dd -3944,-3935,-3926,-3917,-3907,-3898,-3889,-3880,-3871,-3861
dd -3852,-3843,-3834,-3824,-3815,-3806,-3796,-3787,-3778,-3768
dd -3759,-3750,-3740,-3731,-3721,-3712,-3703,-3693,-3684,-3674
dd -3665,-3655,-3646,-3636,-3627,-3617,-3608,-3598,-3588,-3579
dd -3569,-3560,-3550,-3540,-3531,-3521,-3512,-3502,-3492,-3483
dd -3473,-3463,-3453,-3444,-3434,-3424,-3414,-3405,-3395,-3385
dd -3375,-3365,-3356,-3346,-3336,-3326,-3316,-3306,-3297,-3287
dd -3277,-3267,-3257,-3247,-3237,-3227,-3217,-3207,-3197,-3187
dd -3177,-3167,-3157,-3147,-3137,-3127,-3117,-3107,-3097,-3087
dd -3077,-3067,-3056,-3046,-3036,-3026,-3016,-3006,-2996,-2985
dd -2975,-2965,-2955,-2945,-2934,-2924,-2914,-2904,-2893,-2883
dd -2873,-2863,-2852,-2842,-2832,-2821,-2811,-2801,-2790,-2780
dd -2770,-2759,-2749,-2738,-2728,-2718,-2707,-2697,-2686,-2676
dd -2666,-2655,-2645,-2634,-2624,-2613,-2603,-2592,-2582,-2571
dd -2561,-2550,-2540,-2529,-2518,-2508,-2497,-2487,-2476,-2466
dd -2455,-2444,-2434,-2423,-2413,-2402,-2391,-2381,-2370,-2359
dd -2349,-2338,-2327,-2316,-2306,-2295,-2284,-2274,-2263,-2252
dd -2241,-2231,-2220,-2209,-2198,-2188,-2177,-2166,-2155,-2144
dd -2134,-2123,-2112,-2101,-2090,-2079,-2069,-2058,-2047,-2036
dd -2025,-2014,-2003,-1992,-1982,-1971,-1960,-1949,-1938,-1927
dd -1916,-1905,-1894,-1883,-1872,-1861,-1850,-1839,-1828,-1817
dd -1806,-1795,-1784,-1773,-1762,-1751,-1740,-1729,-1718,-1707
dd -1696,-1685,-1674,-1663,-1652,-1641,-1630,-1619,-1608,-1597
dd -1585,-1574,-1563,-1552,-1541,-1530,-1519,-1508,-1496,-1485
dd -1474,-1463,-1452,-1441,-1430,-1418,-1407,-1396,-1385,-1374
dd -1363,-1351,-1340,-1329,-1318,-1307,-1295,-1284,-1273,-1262
dd -1250,-1239,-1228,-1217,-1206,-1194,-1183,-1172,-1161,-1149
dd -1138,-1127,-1115,-1104,-1093,-1082,-1070,-1059,-1048,-1036
dd -1025,-1014,-1003,-991,-980,-969,-957,-946,-935,-923
dd -912,-901,-889,-878,-867,-855,-844,-833,-821,-810
dd -799,-787,-776,-765,-753,-742,-731,-719,-708,-696
dd -685,-674,-662,-651,-640,-628,-617,-605,-594,-583
dd -571,-560,-548,-537,-526,-514,-503,-491,-480,-469
dd -457,-446,-434,-423,-411,-400,-389,-377,-366,-354
dd -343,-332,-320,-309,-297,-286,-274,-263,-252,-240
dd -229,-217,-206,-194,-183,-172,-160,-149,-137,-126
dd -114,-103,-92,-80,-69,-57,-46,-34,-23,-11
   
eosinus:
   
col1:
 dd 0
; misc raycaster vars:
vxx:
 dd 0
vyy:
 dd 0
vl:
 dd 0
vpx:
dd 0x0001FFFF ; initial player position * 0xFFFF
 vpy:
dd 0x0001FFFF
vstepx:
 dd 0
vstepy:
 dd 0
vxxint:
 dd 0
vyyint:
 dd 0
vk:
 dd 0
va:
 dd 0
va2:
 dd 0
vdd:
 dd 0
vx1:
 dd 0
vx1b:
 dd 0
vh:
 dd 0
vdt:
 dd 0
vheading: ; initial heading: 0 to 3599
 dd 0
vacompare:
 dd 0
vpxi:
 dd 0
vpyi:
 dd 0
wtolong:
 dw 0,0
   
xtemp:
 dd 0
ytemp:
 dd 0
xfrac:
 dd 0
yfrac:
 dd 0
h_old:
 dd 0
vbottom:
 dd 0
mouseya:
 dd 0
remeax:
 dd 0
remebx:
 dd 0
remecx:
 dd 0
remedx:
 dd 0
remedi:
 dd 0
remesi:
 dd 0
red:
 dd 0
green:
 dd 0
blue:
 dd 0
pseudo:
 dd 0
step1:
 dd 0
step64:
 dd 0
lasty:
 dd 0
ceil:
include "ceil.inc"
wall:
include "wall.inc"
wall2:
include "wall2.inc"
wall3:
include "wall3.inc"
wall4:
include "wall4.inc"
wall5:
include "wall5.inc"
wall6:
include "wall6.inc"
wall7:
include "wall7.inc"
   
   
   
labelt:
      db   'FISHEYE RAYCASTING ENGINE ETC. FREE3D'
   
labellen:
   
I_END:
   
   
   
   
   