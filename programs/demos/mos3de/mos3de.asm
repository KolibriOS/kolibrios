;MOS3DE V 0.1 - MENUETOS 3D  Engine V 0.1
   
; Compile with FASM for MENUET
   
; this is a very early Version: I'm so happy that it runs at all.
; Of course it's a bad ASM style and it can be optimized a lot.
   
; anyway - I have thought I uploap this so other PPL might optimize it
; as well or transform it to something completely diffrent.
   
; There are some Mouse- and Keyboardhandling Subs and Constructs. They are here
; because I will probably use them for the Navigation in a First Person
; Perspective Game - which is the goal of this project.
; http://www.melog.ch/mos_pub/   dietermarfurt@angelfire.com
   
; of course you can remove or replace them. It has only linear Texturemapping.
; Perspective Correction is a future option.
; a flag for multiple Textures selection should be implemented in the
; Meshfile Format. As I said - it's a very erarly version.
   
use32
   
               org    0x0
   
               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x200000                ; memory for app
               dd     0x7fff0                 ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

IMAGE_W		= 320
IMAGE_H		= 240
			   
START:                          ; start of execution
   
    call init_gfx
    call draw_window            ; at first, draw the window
   
gamestart:
;   ******* MOUSE CHECK *******
    mov eax,37
    mov ebx,1     ; check mouseposition
    int 0x40
   
    mov ebx,eax
    shr eax,16
    and eax,0x0000FFFF  ; mousex
    and ebx,0x0000FFFF  ; mousey
    mov [mousex],eax
    mov [mousey],ebx
    cmp eax,5  ; mouse out of window ?
    jb check_refresh  ; it will prevent an app-crash
    cmp ebx,22
    jb check_refresh
    cmp eax, IMAGE_W
    jg check_refresh
    cmp ebx,221
    jg check_refresh
   
    cmp eax,160 ; navigating?
    jb m_left
    cmp eax,170 ;
    jg m_right
continue:
    cmp ebx,100 ;
    jb s_up
    cmp ebx,144 ;
    jg s_down
;   ******* END OF MOUSE CHECK *******
check_refresh:
   
    mov eax,23  ; wait for system event with 10 ms timeout
    mov ebx,1   ; wait 10 ms, then continue
    int  0x40
;    mov eax,11 ; or use this for full speed instead
;    int 0x40
   
    cmp  eax,1                  ; window redraw request ?
    je   red2
    cmp  eax,2                  ; key in buffer ?
    je   key2
    cmp  eax,3                  ; button in buffer ?
    je   button2
   
    call clear_screen
    call updateworld
    call put_screen
    ;call log ; used for debugging
   
    mov edi,[mouseya] ; check flag if a refresh has to be done
    cmp edi,1
    jne gamestart
    mov [mouseya],dword 0
   
    jmp gamestart
   
; END OF MAINLOOP
   
red2:                          ; redraw
    call draw_window
;    call draw_stuff
    jmp gamestart
   
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
    cmp edi,1800
    jb ok_heading0
    sub edi,1800
    ok_heading0:
    mov [vheading],edi
    mov [mouseya],dword 1
    jmp check_refresh
   
s_right:                                  ; turn right
    mov edi,[vheading]
    sub edi,50
    cmp edi,-1
    jg ok_heading1
    add edi,1800
    ok_heading1:
    mov [vheading],edi
    mov [mouseya],dword 1
    jmp check_refresh
   
m_left:                                   ; turn left (mouse)
    mov edi,[vheading]  ; heading
    mov ecx,160
    sub ecx,eax
    sar ecx,2
    add edi,ecx
    cmp edi,1800
    jb ok_heading2
    sub edi,1800
    ok_heading2:
    mov [vheading],edi
    mov [mouseya],dword 1
    jmp continue    ; allow both: walk and rotate
   
m_right:                                  ; turn right
    mov edi,[vheading]
    sub eax,170
    sar eax,2
    sub edi,eax
    cmp edi,-1
    jg ok_heading3
    add edi,1800
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
   
    mov  eax,12                   ; function 12:tell os about windowdraw
    mov  ebx,1                    ; 1, start of draw; 2 - end
    int  0x40
 
	mov eax, 48                   ; get skin height
	mov ebx, 4
	int  0x40
	
	lea  ecx,[eax + (119 shl 16) + IMAGE_H + 4]
    mov  ebx,192*65536+IMAGE_W+9  ; [x start] *65536 + [x size]
    mov  edx,0x74000000           ; skinned window, not resizable
    mov  edi,labelt               ; window title
    mov  eax,0                    ; function 0 : define and draw window
    int  0x40

    mov  eax,12
    mov  ebx,2
    int  0x40
 
    ret
   
   
; ---------------------------------------------------------------------
init_gfx:
   
;  Pwidth = 63 Shl 16   ;original texture width in pixels -1 shl 16
;  Pheight = 63 Shl 16 ;original texture height in pixels -1 shl 16
   
   mov [pwidth],dword 63
   shl dword[pwidth],16
   mov [pheight],dword 63
   shl dword[pheight],16
   
;; Read in a mesh
   mov eax,0  ; tex1+4
   mov edi,anz
   add edi,4    ; beginning of mesh data
   
;using a copy will allow inverse kinematic transformations (future option)
   
   readmesh:
   mov edx,eax
   add edx,a_xwww ; desti
   mov ecx,[edi] ; value in ecx
   mov [edx],ecx
   add edi,4
   ;---
   mov edx,eax
   add edx,a_ywww ; desti
   mov ecx,[edi] ; value in ecx
   mov [edx],ecx
   add edi,4
   ;---
   mov edx,eax
   add edx,a_zwww ; desti
   mov ecx,[edi] ; value in ecx
   mov [edx],ecx
   add edi,4
   ;---
   
   add eax,4
   cmp edi,eo_mesh
   jl readmesh
   
;  zoom=-500
   mov [zoom],dword -500
   
ret
;------------------------------------------------------------------
   
   
   
log:
;write some Variable info to screen (used for debugging)
 pusha
  mov edi,wonder ;a_xwww
  ;add edi,200
  mov [remecx], dword 24
  print:
  mov eax,47
  mov ebx,0x000f0000
  ;mov ebx,0x000f0100  ; hex
  mov ecx,[edi]
  mov edx,[remecx]
  or edx,0x000C0000
  mov esi,0x00ff0000
  int 0x40
  add [remecx],dword 8
  add edi,dword 4
  cmp [remecx], dword 250
  jl print
 popa
ret
   
   
; ---------------------------------------------------------------------
;                          UPDATE WORLD
; ---------------------------------------------------------------------
   
 updateworld:
   
  mov edx,[a_xw]
   
; Miny% = 32767
; Maxy% = 0
  mov [miny],dword 32737
  mov [maxy],dword 0
   
; Color 0,0,0 ; clear screen - l8er
; Rect 0,0,320,240,1
   
; a=a+1.0 ; automatic rotation...
  add [a],dword 10
  cmp [a],dword 3598
  jl ok360
  mov [a],dword 0
  ok360:
   
; If a>359.9 Then a=0
; alpha=a
; beta=a
; gamma=a+a Mod 360
   
  mov eax,[a]
  mov [alpha],eax
  mov [beta], eax
  mov [gamma],eax
  add [gamma],eax
  cmp [gamma],dword 3599
  jl ok360_2
  sub [gamma],dword 3600
  ok360_2:
   
; mausy#=0.1+(MouseY()/50.0) ; >>>>>>l8er
   
  mov eax,[alpha]
  mov [alphacopy],eax
  mov eax,[beta]
  mov [betacopy],eax
  mov eax,[gamma]
  mov [gammacopy],eax
   
; For i=0 To anz ; do rotation and projection  etc
  mov esi,0
  for_i:
   
; *****************************
;    ROTATE pitch jaw roll...
; *****************************
   
;;  xl1#=zwww(i)*Sin(gamma)+xwww(i)*Cos(gamma)
    mov eax,esi
    imul eax,4
    add eax,a_zwww
    mov ebx,[eax]
    mov eax,[gammacopy]
    call get_sinus
    imul ebx,[eax]
    mov [tempdiv],ebx ; is zwww(i)*sin(gamma)
   
    mov eax,esi
    imul eax,4
    add eax,a_xwww
    mov ebx,[eax]
   
    mov eax,[gammacopy]
    call get_cosinus
    imul ebx,[eax]  ; is xwww*cos(gamma
   
    add ebx,[tempdiv]
    mov eax,ebx
    ;or eax,1
    cdq
     mov ecx,6553
     idiv ecx
    mov [xl1],eax
   
   
;;  yl1#=ywww(i)
    mov eax,esi
    imul eax,4
    add eax,a_ywww
    mov ebx,[eax]
    mov [yl1],ebx
   
   
;;  zl1#=zwww(i)*Cos(gamma)-xwww(i)*Sin(gamma)
    mov eax,esi
    imul eax,4
    add eax,a_zwww
    mov ebx,[eax]
    mov eax,[gammacopy]
    call get_cosinus
    imul ebx,[eax]
    mov [tempdiv],ebx ; is zwww(i)*cos(gamma)
   
    mov eax,esi
    imul eax,4
    add eax,a_xwww
    mov ebx,[eax]
   
    mov eax,[gammacopy]
    call get_sinus
    imul ebx,[eax]  ; is xwww*sin(gamma
   
    sub [tempdiv],ebx
    mov eax,[tempdiv]
    ;or eax,1
    cdq
    mov ecx,6553 ; once 6500
    idiv ecx
    mov [zl1],eax
   
   
   
;
;-----------------------------------------------------------------------
;;  xl2#=xl1
    mov eax,[xl1]
    mov [xl2],eax
   
;;  yl2#=yl1*Cos(beta)-zl1*Sin(beta)
    mov ebx,[yl1]
    mov eax,[betacopy]
    call get_cosinus
    imul ebx,[eax]
    mov [tempdiv],ebx ; is yl1*cos(beta)
   
    mov ebx,[zl1]  ; zl1
   
    mov eax,[betacopy]
    call get_sinus
    imul ebx,[eax]  ; is zl1*sin(beta
   
    sub [tempdiv],ebx
    mov eax,[tempdiv]
    ;or eax,1
    cdq
    mov ecx,6553
    idiv ecx
    mov [yl2],eax
   
   
   
;;  zl2#=yl1*Sin(beta)+zl1*Cos(beta)
    mov ebx,[yl1]
    mov eax,[betacopy]
    call get_sinus
    imul ebx,[eax]
    mov [tempdiv],ebx ; is yl1*sin(beta)
   
    mov ebx,[zl1]  ; zl1
   
    mov eax,[betacopy]
    call get_cosinus
    imul ebx,[eax]  ; is zl1*cos(beta
   
     add ebx,[tempdiv]
     mov eax,ebx
    ;or eax,1
    cdq
    mov ecx,6553
    idiv ecx
    mov [zl2],eax
   
   
;-----------------------------------------------------------------------
;;  xl3#=(yl2*Sin(alpha)+xl2*Cos(alpha))
    mov ebx,[yl2]
    mov eax,[alphacopy]
    call get_sinus
    imul ebx,[eax]
    mov [tempdiv],ebx ; is yl2*sin(alpha)
   
    mov ebx,[xl2]  ; xl2
    mov eax,[alphacopy]
    call get_cosinus
    imul ebx,[eax]  ; is xl2*cos(alpha
   
    add ebx,[tempdiv]
    mov eax,ebx
    ;or eax,1
    cdq
    mov ecx,6553
    idiv ecx
    mov [xl3],eax
   
   
;;  yl3#=(yl2*Cos(alpha)-xl2*Sin(alpha))
    mov ebx,[yl2]
    mov eax,[alphacopy]
    call get_cosinus
    imul ebx,[eax]
    mov [tempdiv],ebx ; is yl2*cos(alpha)
   
    mov ebx,[xl2]  ; xl2
    mov eax,[alphacopy]
    call get_sinus
    imul ebx,[eax]  ; is xl2*sin(alpha
   
    sub [tempdiv],ebx
    mov eax,[tempdiv]
    ;or eax,1
    cdq
    mov ecx,6553
    idiv ecx
    mov [yl3],eax
   
   
;;  zl3#=(zl2)
    mov eax,[zl2]
    mov [zl3],eax
   
;-----------------------------------------------------------------------
   
; *********************
;  PROJECTING 3D to 2D
   
;  If yloc# - Zoom <> 0 Then yloc = Int(yl3 ) * 200 / (zl3 - Zoom)
   ;-------
   mov ecx,[zl3]
   sub ecx,[zoom]
   mov eax,[yl3]
   imul eax,200
   or eax,1
   cdq
   idiv ecx
   ;mov eax,[yl3] ; no perpective
   mov [yloc],eax
   ;------
   
 ; If xloc# - Zoom <> 0 Then xloc = Int(xl3 ) * 200 / (zl3 - Zoom)
   
   ;------
   mov ecx,[zl3]
   sub ecx,[zoom]
   mov eax,[xl3]
   imul eax,200
   or eax,1
   cdq
   idiv ecx
   ;mov eax,[xl3] ; no perspective
   mov [xloc],eax
   ;------
   
;  xw(i)=((mausy#)*xloc) +160
;  yw(i)=((mausy#)*yloc) +120
;  zw(i)=(zl3+256)
   
   mov eax,[mousey] ; define scaling
   mov [factor],eax
   cmp eax,32000
   jl positivemousey
   mov [factor],dword 10
   jmp less220
   positivemousey:
   cmp eax,10
   jg more20
   mov [factor],dword 10
   jmp less220
   more20:
   cmp eax,220
   jl less220
   mov [factor],dword 220
   less220:
   
   mov eax,esi ; i...
   imul eax,4
   
   mov ecx,[xloc]
   mov eax,esi
   imul eax,4
   mov ebx,a_xw
   add ebx,eax
   imul ecx,[factor]
   sar ecx,5
   mov [ebx],ecx
   add [ebx],dword 160
   
   mov ecx,[yloc]
;   mov eax,esi
;   imul eax,4
   mov ebx,a_yw
   add ebx,eax
   imul ecx,[factor]
   sar ecx,5
   mov [ebx],ecx
   add [ebx],dword 120
   
   mov ecx,[zl3] ; used for z sorting
   mov ebx,a_zw
   add ebx,eax
   mov [ebx],ecx
   add [ebx],dword 256
   
; Next
  inc esi
  cmp esi,[anz]
  jle for_i
;---------------
   
   
  mov [sorted_quads],dword 0
   
   
; ; z-sorting...
; For i=0 To 10000 ; clear old info
;  zbuffer(i)=-1
; Next
  mov eax,zbuffer
  mov ebx,eax
  add ebx,40000
  clear_zbuffer:
  mov [eax],dword 1000000
  add eax,4
  cmp eax,ebx
  jle clear_zbuffer
   
   
   
; For i=0 To anz-3 Step 4
  mov eax,0
  mov ebx,[anz]
  for_i_0_to_anz:
   
;  If zw(i)>=0 ; clip Quads behind Camera
   mov ecx,eax
   imul ecx,4
   add ecx,a_zw
   mov edx,[ecx]
   cmp edx,0
   jle behindcamera
   
;   zwmax=zw(i)
    mov esi,edx
   
;   ;find quads internal most far point
;   If zwmax<zw(i+1) Then zwmax=zw(i+1)
;   If zwmax<zw(i+2) Then zwmax=zw(i+2)
;   If zwmax<zw(i+3) Then zwmax=zw(i+3)
    ; skip this for debugging
    ; jmp no_internal_sorting
    cmp esi,[ecx+4]
    jge checkmore1
    mov esi,[ecx+4]
    checkmore1:
   
    cmp esi,[ecx+8]
    jge checkmore2
    mov esi,[ecx+8]
    checkmore2:
   
    cmp esi,[ecx+12]
    jge checkmore3
    mov esi,[ecx+12]
    checkmore3:
   
    no_internal_sorting:
   
    mov [i],eax ; anz-ID in [i] (not times 4)
    push eax
    push ebx
   
;   While zbuffer(zwmax)<>-1 And zwmax<10000
;    zwmax=zwmax+1
;   Wend
    mov eax,esi  ; is actual z/poititon of point (used for z-order)
    imul eax,4
   
    findslot:
    mov ebx,eax
    add ebx,zbuffer
    cmp [ebx],dword 1000000
    je found_empty_slot
    add eax,4
    cmp eax,40000
    jl findslot
    found_empty_slot:
   
;   zbuffer(zwmax)=i
    inc dword[sorted_quads]  ; check slot writing
   
    mov ecx,[i]
    mov [ebx],ecx
   
    pop ebx
    pop eax
   
;  EndIf
   behindcamera:
; Next
  add eax,4
  cmp eax,[anz]  ;ebx
  jl for_i_0_to_anz
  ;----------------
   
  ;mov [rendered_quads],dword 0 ; debugging...
   
; For i2=10000 To 0 Step -1 ; reading quads in z-order from far to near
  mov eax,40000
  mov ebx,0
  for_i_0_to_10000:
   
   
;  i=zbuffer(i2)
   mov edx,eax
   add edx,zbuffer
   mov ecx,[edx] ; i...
   
;  If i>-1 And i< anz-2 ; if it isn't -1 then it's a Quad Point 1 ID
   cmp ecx,1000000
   je is_empty_slot
   
    ;inc dword[rendered_quads] ; used for debugging
   
;   ;***Mapping***
   
;   ;GetPolygonPoints(i)
;   ;FindSmallLargeY()
   
    mov [ilocal],ecx
    call get_polygon_points
    call find_small_large_y
   
;   X1% = Polypoints%(0, 0)
    mov edi,[polypoints]
    mov [x1],edi
;   Y1% = Polypoints%(0, 1)
    mov edi,[polypoints+4]
    mov [y1],edi
;   X2% = Polypoints%(1, 0)
    mov edi,[polypoints+8]
    mov [x2],edi
;   Y2% = Polypoints%(1, 1)
    mov edi,[polypoints+12]
    mov [y2],edi
   
;   ScanConvert(X1%, Y1%, X2%, Y2%, 1)     ;scan top of picture
    mov [pside],dword 1
    call scan_convert
   
;   X1% = Polypoints%(1, 0)
    mov edi,[polypoints+8]
    mov [x1],edi
;   Y1% = Polypoints%(1, 1)
    mov edi,[polypoints+12]
    mov [y1],edi
;   X2% = Polypoints%(2, 0)
    mov edi,[polypoints+16]
    mov [x2],edi
;   Y2% = Polypoints%(2, 1)
    mov edi,[polypoints+20]
    mov [y2],edi
   
;   ScanConvert(X1%, Y1%, X2%, Y2%, 2)   ;scan Right of picture
    mov [pside],dword 2
    call scan_convert
   
;   X1% = Polypoints%(2, 0)
    mov edi,[polypoints+16]
    mov [x1],edi
;   Y1% = Polypoints%(2, 1)
    mov edi,[polypoints+20]
    mov [y1],edi
;   X2% = Polypoints%(3, 0)
    mov edi,[polypoints+24]
    mov [x2],edi
;   Y2% = Polypoints%(3, 1)
    mov edi,[polypoints+28]
    mov [y2],edi
   
;   ScanConvert(X1%, Y1%, X2%, Y2%, 3)  ;scan bottom of picture
    mov [pside],dword 3
    call scan_convert
   
;   X1% = Polypoints%(3, 0)
    mov edi,[polypoints+24]
    mov [x1],edi
;   Y1% = Polypoints%(3, 1)
    mov edi,[polypoints+28]
    mov [y1],edi
;   X2% = Polypoints%(0, 0)
    mov edi,[polypoints]
    mov [x2],edi
;   Y2% = Polypoints%(0, 1)
    mov edi,[polypoints+4]
    mov [y2],edi
   
;   ScanConvert(X1%, Y1%, X2%, Y2%, 4)    ;scan Left of picture
    mov [pside],dword 4
    call scan_convert
   
;   TextureMap()
     call texture_map
   
;  EndIf
   is_empty_slot:
   
; Next
  sub eax,4
  cmp eax,0  ;ebx
  jge for_i_0_to_10000
   
;Wend
ret
   
;End
   
   
   
   
; ---------------------------------------------------------------------
   
get_sinus:
 imul eax,4 ; expects degree*10
 add eax,sinus
ret
   
get_cosinus:
 imul eax,4 ; expects degree*10
 add eax,sinus
 add eax,10804
 cmp eax,eosinus
 jl ok3600sub
 sub eax,14400
 ok3600sub:
ret
   
   
   
   
   
   
;; ------------------ texture mapping functions-----------------------------
   
;Function GetPolygonPoints(ilocal%) ; initially read in a rectangle
get_polygon_points:
pusha
; For Count% = 0 To 3
;  Polypoints%(Count%, 0) = xw(ilocal%+Count%)
;  Polypoints%(Count%, 1) = yw(ilocal%+Count%)
; Next
  mov eax,0
; ---
  count_0_3:
   
  mov ebx,eax
  imul ebx,8         ; count
  add ebx,polypoints  ; desti adr
  mov ecx,eax
  add ecx,[ilocal]
  imul ecx,4
  add ecx,a_xw      ; src adr
  mov edx,[ecx]
  mov [ebx],edx
  ; ----
  add ebx,4          ; desti 2
  mov ecx,eax
  add ecx,[ilocal]
  imul ecx,4
  add ecx,a_yw       ; src 2
  mov edx,[ecx]
  mov [ebx],edx
  ; ----
  inc eax
  cmp eax,4
  jl count_0_3
   
;End Function
 popa
ret
   
; -------------------------------------------------------------------------
   
;Function FindSmallLargeY()
find_small_large_y:
 pusha
; For Count% = 0 To 3
  mov eax,0
  for03b:
   
;  Ycoord% = Polypoints%(Count%, 1)
   mov ebx,eax
   imul ebx,8
   add ebx,4
   add ebx,polypoints
   mov ecx,[ebx]
   
;  If Ycoord% < Miny% Then       ; is this the New lowest y co-ord?
;   Miny% = Ycoord%             ; Yes...
;  End If
   cmp ecx,[miny]
   jge isge0
   mov [miny],ecx
   isge0:
   
;  If Ycoord% > Maxy% Then       ; is this the New highest y co-ord?
;   Maxy% = Ycoord%             ; Yes...
;  End If
   cmp ecx,[maxy]
   jle isge1
   mov [maxy],ecx
   isge1:
   
; Next
  inc eax
  cmp eax,4
  jl for03b
   
;End Function
 popa
ret
   
; -------------------------------------------------------------------------
   
   
;Function ScanConvert (X1%, Y1%, X2%, Y2%, Pside)
scan_convert:
 pusha
; If Y2% < Y1% Then
  mov eax,[y1]
  mov ebx,[y2]
  cmp eax,ebx
  jl l_else0
   
;  temp%=X1% : X1%=X2% : X2%=temp%
;  temp%=Y1% : Y1%=Y2% : Y2%=temp%
;  Lineheight% = (Y2% - Y1%)
;  ScanLeftSide(X1%, X2%, Y1%, Lineheight%, Pside)
   mov [y1],ebx ; swap y,x
   mov [y2],eax
    mov eax,[x1]
    mov ebx,[x2]
   mov [x1],ebx
   mov [x2],eax
   mov eax,[y1]
    mov ebx,[y2]
    ; eo swap
   sub ebx,eax
    mov [lineheight],ebx
    call scan_left_side
   jmp l_endif0
   
; Else
  l_else0:
   
;  Lineheight% = (Y2% - Y1%)
;  ScanRightSide(X1%, X2%, Y1%, Lineheight%, Pside)
   sub ebx,eax
   mov [lineheight],ebx
   call scan_right_side
   
; End If
  l_endif0:
;End Function
 popa
ret
; -------------------------------------------------------------------------
   
   
;Function ScanLeftSide (X1%, X2%, Ytop%, Lineheight%, Pside)
 scan_left_side:
  pusha
   
  mov eax,[y1]
  mov [ytop],eax
   
; Lineheight% = Lineheight% + 1       ; prevent divide by zero
  inc dword[lineheight]
; Xadd = (X2% - X1%) Shl 16
  mov edi,[x2]
  sub edi,[x1]
  sal edi,16
   
; Xadd = Xadd / Lineheight%
  mov eax,edi ; whole
  cdq
  mov ebx,[lineheight] ; divisor
; or ebx,1
  idiv ebx ; result now in eax
  mov [v_xadd],eax
   
; ------------
; If Pside = 1 Then
  cmp [pside],dword 1
  jne psidenot10
   
;  Px = Pwidth% - 1
;  Py = 0
   mov edi,[pwidth]
   mov esi,edi
   sub esi,1
   mov [px],esi
   mov [py],dword 0
;  Pxadd = -Pwidth%  / Lineheight%
;  Pyadd = 0
   mov eax,0
   sub eax,[pwidth]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,1
   idiv ebx ; result now in eax
   mov [pxadd],eax
   mov [pyadd],dword 0
; End If
  psidenot10:
   
; ------------
; If Pside = 2 Then
  cmp [pside],dword 2
  jne psidenot20
   
;  Px = Pwidth%
;  Py = Pheight%
   mov edi,[pwidth]
   mov esi,[pheight]
   mov [px],edi
   mov [py],esi
;  Pxadd = 0
;  Pyadd = -Pheight%  / Lineheight%
   mov [pxadd],dword 0
   mov eax,0
   sub eax,[pheight]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,1
   idiv ebx ; result now in eax
   mov [pyadd],eax
; End If
  psidenot20:
   
; ------------
; If Pside = 3 Then
  cmp [pside],dword 3
  jne psidenot30
;  Px = 0
;  Py = Pheight%
   mov [px],dword 0
   mov edi,[pheight]
   mov [py],edi
;  Pxadd = Pwidth%  / Lineheight%
;  Pyadd = 0
   mov eax,[pwidth]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,1
   idiv ebx ; result now in eax
   mov [pxadd],eax
   mov [pyadd],dword 0
; End If
  psidenot30:
   
; ------------
; If Pside = 4 Then
  cmp [pside],dword 4
  jne psidenot40 ; jne
;  Px = 0
;  Py = 0
   mov [px],dword 0
   mov [py],dword 0
;  Pxadd = 0
;  Pyadd = Pheight%  / Lineheight%
   mov [pxadd],dword 0
   mov eax,[pheight]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,1
   idiv ebx
   mov [pyadd],eax
; End If
  psidenot40:
; ------------
; x = X1% Shl 16
  mov edx,[x1] ; used for x
  mov edi,[px] ; used for px
  mov esi,[py] ; used for py
  sal edx,16
  ;mov [x],edx
;------
; For y% = 0 To Lineheight%
  mov eax,0
  for0lineheight0:
   
;  Ytopy%=Ytop%+y%
   mov ebx,[ytop]
   mov [ytopy],ebx
   add [ytopy],eax
   
;  If Ytopy%<0 Then Ytopy%=0 ; prevent read pre array
   cmp [ytopy], dword 0
   jge isnot00
   mov [ytopy],dword 0
   isnot00:
   
;  Lefttable(Ytopy%, 0) = x Sar 16    ;polygon x
   mov ecx,[ytopy]
   imul ecx,16
   add ecx,lefttable
   mov [ecx],edx
   sar dword[ecx],16
   
;  Lefttable(Ytopy%, 1) = Px          ;picture x
   add ecx,4
   mov [ecx],edi
   
;  Lefttable(Ytopy%, 2) = Py          ;picture y
   add ecx,4
   mov [ecx],esi
   
;  x = x + Xadd                       ;Next polygon x
;  Px = Px + Pxadd                    ;Next picture x
;  Py = Py + Pyadd                    ;Next picture y
   add edx,[v_xadd]
   add edi,[pxadd]
   add esi,[pyadd]
   
;------
; Next
  inc eax
  cmp eax,[lineheight]
  jl for0lineheight0
   
   
;End Function
 popa
ret
   
; -------------------------------------------------------------------------
   
; -------------------------------------------------------------------------
   
;Function ScanRightSide (X1%, X2%, Ytop%, Lineheight%, Pside)
 scan_right_side:
  pusha
  mov eax,[y1]
  mov [ytop],eax
   
; Lineheight% = Lineheight% + 1       ; prevent divide by zero
  inc dword[lineheight]
   
; Xadd = (X2% - X1%) Shl 16
  mov edi,[x2]
  sub edi,[x1]
  sal edi,16
; Xadd = Xadd / Lineheight%
  mov eax,edi ; whole
  cdq
  mov ebx,[lineheight] ; divisor
  ;or ebx,1
  idiv ebx ; result now in eax
  mov [v_xadd],eax
   
   
   
 ; ------------
; If Pside = 1 Then
  cmp [pside],dword 1
  jne psidenot11
;  Px = 0
;  Py = 0
   mov [px],dword 0
   mov [py],dword 0
;  Pxadd = Pwidth% / Lineheight%
;  Pyadd = 0
   mov eax,[pwidth]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,1
   idiv ebx ; result now in eax
   mov [pxadd],eax
   mov [pyadd],dword 0
; End If
  psidenot11:
; ------------
   
; If Pside = 2 Then
  cmp [pside],dword 2
  jne psidenot21
;  Px = Pwidth%
;  Py = 0
   mov edi,[pwidth]
   mov [px],edi
   mov [py],dword 0
;  Pxadd = 0
;  Pyadd = Pheight% / Lineheight%
   mov [pxadd],dword 0
   mov eax,[pheight]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,1
   idiv ebx ; result now in eax
   mov [pyadd],eax
; End If
  psidenot21:
; ------------
   
; If Pside = 3 Then
  cmp [pside],dword 3
  jne psidenot31
;  Px = Pwidth%
;  Py = Pheight%
   mov edi,[pwidth]
   mov [px],edi
   mov esi,[pheight]
   mov [py],esi
;  Pxadd = -Pwidth% / Lineheight%
;  Pyadd = 0
   mov eax,0
   sub eax,[pwidth]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,3  ; prevent div 0
   idiv ebx ; result now in eax
   mov [pxadd],eax
   mov [pyadd],dword 0
; End If
  psidenot31:
   
; ------------
; If Pside = 4 Then
  cmp [pside],dword 4
  jne psidenot41 ; jne
;  Px = 0
;  Py = Pheight%
   mov [px],dword 0
   mov esi,[pheight]
   mov [py],esi
;  Pxadd = 0
;  Pyadd = -Pheight% / Lineheight%
   mov [pxadd],dword 0
   mov eax,0
   sub eax,[pheight]
   cdq
   mov ebx,[lineheight] ; divisor
   ;or ebx,1
   idiv ebx
   mov [pyadd],eax
; End If
  psidenot41:
  push edx
  push edi
  push esi
; ------------
; x = X1% Shl 16
  mov edx,[x1] ; used for x
  mov edi,[px] ; used for px
  mov esi,[py] ; used for py
  sal edx,16
   
cmp [lineheight],dword 100  ; lineheight is up to 33 mio ???
jle okok
mov eax,[lineheight]
mov [wonder+4],eax
okok:
   
;------
; For y% = 0 To Lineheight%
  mov eax,0
  for0lineheight1b:
; Ytopy%=Ytop%+y%
  mov ebx,[ytop]
  mov [ytopy],ebx
  add [ytopy],eax
   
;  If Ytopy%<0 Then Ytopy%=0  ; prevent read pre array
   cmp [ytopy], dword 0
   jg isnot01
   mov [ytopy],dword 0
   isnot01:
;  righttable(Ytopy%, 0) = x Sar 16    ;polygon x
   mov ecx,[ytopy]
   imul ecx,16
   add ecx,righttable
   mov [ecx],edx
   sar dword[ecx],16
   
;  righttable(Ytopy%, 1) = Px          ;picture x
   add ecx,4
   mov [ecx],edi
;  righttable(Ytopy%, 2) = Py          ;picture y
   add ecx,4
   mov [ecx],esi
;  x = x + Xadd                       ;Next polygon x
;  Px = Px + Pxadd                    ;Next picture x
;  Py = Py + Pyadd                    ;Next picture y
   add edx,[v_xadd]
   add edi,[pxadd]
   add esi,[pyadd]
;------
; Next
  inc eax
  cmp eax,[lineheight]
  jl for0lineheight1b
dbg2:
  pop esi
  pop edi
  pop edx
   
;End Function
 popa
ret
   
   
; -------------------------------------------------------------------------
   
; well I'm shure the following Sub CAN and SHOULD be optimized a LOT.
   
   
;Function TextureMap()
texture_map:
 pusha
   
; For y% = Miny% To Maxy%
  mov eax,[miny]
  for_miny_maxy:
   
;  If y>0 And y<=239
   cmp eax,0
   jle clipy
   cmp eax,239
   jg clipy
   
   mov [y],eax
   
;   Polyx1% = Lefttable((y%), 0)
    mov ebx,eax
    sal ebx,4
    add ebx,lefttable
    mov ecx,[ebx]
    mov [polyx1],ecx
;   Px1 = Lefttable(y%, 1)
    add ebx,4
    mov ecx,[ebx]
    mov [px1],ecx
;   Py1 = Lefttable(y%, 2)
    add ebx,4
    mov ecx,[ebx]
    mov [py1],ecx
   
   
;   Polyx2% = Righttable((y%), 0)
    mov ebx,eax
    sal ebx,4
    add ebx,righttable
    mov ecx,[ebx]
    mov [polyx2],ecx
;   Px2 = Righttable(y%, 1)
    add ebx,4
    mov ecx,[ebx]
    mov [px2],ecx
;   Py2 = Righttable(y%, 2)
    add ebx,4
    mov ecx,[ebx]
    mov [py2],ecx
   
   
;   Linewidth% = Polyx2% - Polyx1%
    mov ecx,[polyx2]
    sub ecx,[polyx1]
   
;   Linewidth%=Linewidth% Or 1
    or ecx,1
    mov [linewidth],ecx
   
;   Pxadd = ((Px2 - Px1)) / Linewidth%
    mov eax,[px2]
    sub eax,[px1]
    cdq
    mov ebx,[linewidth] ; divisor
    or ebx,1
    idiv ebx  ; should be idiv ... probs
    mov [pxadd],eax
   
;   Pyadd = ((Py2 - Py1)) / Linewidth%
    mov eax,[py2]
    sub eax,[py1]
    cdq
    mov ebx,[linewidth] ; divisor
    or ebx,1
    idiv ebx  ; should be idiv ...probs
    mov [pyadd],eax
   
    mov edi,[px1]
    mov esi,[py1]
   
   
;   For x% = Polyx1% To Polyx2%
    mov eax,[polyx1]
    cmp eax,[polyx2]
    jge clipxfully
    for_polyx1_polyx2:
   
;     If x>0 And x<=319
      cmp eax,0
      jl clipx
      cmp eax,319
      jg clipx
   
;      Col%=ReadPixelFast((Px1 Shr 16),(Py1 Shr 16),imgtxt)
   
       mov ebx,edi
       sar ebx,16   ; is eq (px1 shr 16)*64
       and ebx,63
       sal ebx,2  ; is x
   
       mov ecx,esi
       sar ecx,16
       and ecx,63
       sal ecx,8  ; y
       add ecx,ebx
       add ecx,tex1 ; adr of texturepixel now in ecx
   
       mov edx,[ecx]; rgb now in edx
   
;      WritePixelFast x%,y%,Col%
       mov ebx,[y]
       mov [tempdiv],ebx
       shl dword [tempdiv],6
       shl ebx,10
       sub ebx,[tempdiv]
       ;this was imul ebx,960 ; *1024 - *64
       add ebx,eax
       add ebx,eax
       add ebx,eax
       add ebx,0x80000
       or [ebx],edx
;     EndIf
      clipx:
   
;     Px1 = Px1 + Pxadd
;     Py1 = Py1 + Pyadd
      add edi,dword [pxadd]
      add esi,dword [pyadd]
   
;   Next x
    inc eax
    cmp eax,[polyx2]
    jl for_polyx1_polyx2
    clipxfully:
    mov eax,[y]
   
;  EndIf
   clipy:
; Next y
  inc eax
  cmp eax,[maxy]
  jl for_miny_maxy
   
;End Function
   
   
popa
ret
; -------------------------------------------------------------------------
   
   
   
put_screen:
pusha
mov eax,7
mov ebx,0x80000
mov ecx,IMAGE_W*65536+IMAGE_H
mov edx,0
int 0x40
popa
ret
   
   
clear_screen:
push ebx
mov ebx,0x80000
cls:
mov [ebx],dword 0
add ebx,4
cmp ebx,0x80000+(IMAGE_W*IMAGE_H*3)
jl cls
pop ebx
ret
   
; -------------------------------------------------------------------------
   
; DATA AREA
   
   
; cube.inc includes the mesh 3D Data, in this case a simple cube. Any Model
; is theoreticly possible. Check the File, the Format is trivial.
; The Quads should not intersect for a bearable z-sorting. The Quads must be
; clockwise.
   
include "cube.inc"
   
;;------------------------------
   
grid:  ; 32*32 Blocks, Map: 0 = Air, 1 to 5 = Wall
; this is a relict from an other program. I leave it here to keep
; the Navigation Subs compatible (planning to recycle them)
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
dd 0,11,22,34,45,57,68,80,91,102
dd 114,125,137,148,160,171,182,194,205,217
dd 228,240,251,263,274,285,297,308,320,331
dd 342,354,365,377,388,400,411,422,434,445
dd 457,468,479,491,502,514,525,536,548,559
dd 571,582,593,605,616,628,639,650,662,673
dd 685,696,707,719,730,741,753,764,775,787
dd 798,810,821,832,844,855,866,878,889,900
dd 912,923,934,946,957,968,979,991,1002,1013
dd 1025,1036,1047,1059,1070,1081,1092,1104,1115,1126
dd 1138,1149,1160,1171,1183,1194,1205,1216,1228,1239
dd 1250,1261,1272,1284,1295,1306,1317,1328,1340,1351
dd 1362,1373,1384,1396,1407,1418,1429,1440,1451,1463
dd 1474,1485,1496,1507,1518,1529,1541,1552,1563,1574
dd 1585,1596,1607,1618,1629,1640,1651,1663,1674,1685
dd 1696,1707,1718,1729,1740,1751,1762,1773,1784,1795
dd 1806,1817,1828,1839,1850,1861,1872,1883,1894,1905
dd 1916,1926,1937,1948,1959,1970,1981,1992,2003,2014
dd 2025,2036,2046,2057,2068,2079,2090,2101,2111,2122
dd 2133,2144,2155,2166,2176,2187,2198,2209,2219,2230
dd 2241,2252,2262,2273,2284,2295,2305,2316,2327,2337
dd 2348,2359,2369,2380,2391,2401,2412,2423,2433,2444
dd 2454,2465,2476,2486,2497,2507,2518,2529,2539,2550
dd 2560,2571,2581,2592,2602,2613,2623,2634,2644,2655
dd 2665,2675,2686,2696,2707,2717,2728,2738,2748,2759
dd 2769,2779,2790,2800,2811,2821,2831,2841,2852,2862
dd 2872,2883,2893,2903,2913,2924,2934,2944,2954,2965
dd 2975,2985,2995,3005,3015,3026,3036,3046,3056,3066
dd 3076,3086,3096,3106,3117,3127,3137,3147,3157,3167
dd 3177,3187,3197,3207,3217,3227,3237,3246,3256,3266
dd 3276,3286,3296,3306,3316,3326,3336,3345,3355,3365
dd 3375,3385,3394,3404,3414,3424,3433,3443,3453,3463
dd 3472,3482,3492,3501,3511,3521,3530,3540,3550,3559
dd 3569,3578,3588,3598,3607,3617,3626,3636,3645,3655
dd 3664,3674,3683,3693,3702,3711,3721,3730,3740,3749
dd 3758,3768,3777,3786,3796,3805,3814,3824,3833,3842
dd 3852,3861,3870,3879,3888,3898,3907,3916,3925,3934
dd 3943,3953,3962,3971,3980,3989,3998,4007,4016,4025
dd 4034,4043,4052,4061,4070,4079,4088,4097,4106,4115
dd 4124,4133,4142,4150,4159,4168,4177,4186,4194,4203
dd 4212,4221,4230,4238,4247,4256,4264,4273,4282,4290
dd 4299,4308,4316,4325,4333,4342,4351,4359,4368,4376
dd 4385,4393,4402,4410,4419,4427,4435,4444,4452,4461
dd 4469,4477,4486,4494,4502,4511,4519,4527,4535,4544
dd 4552,4560,4568,4577,4585,4593,4601,4609,4617,4625
dd 4634,4642,4650,4658,4666,4674,4682,4690,4698,4706
dd 4714,4722,4730,4737,4745,4753,4761,4769,4777,4785
dd 4792,4800,4808,4816,4824,4831,4839,4847,4854,4862
dd 4870,4877,4885,4893,4900,4908,4915,4923,4930,4938
dd 4945,4953,4960,4968,4975,4983,4990,4998,5005,5012
dd 5020,5027,5034,5042,5049,5056,5064,5071,5078,5085
dd 5093,5100,5107,5114,5121,5128,5135,5143,5150,5157
dd 5164,5171,5178,5185,5192,5199,5206,5213,5220,5226
dd 5233,5240,5247,5254,5261,5268,5274,5281,5288,5295
dd 5301,5308,5315,5321,5328,5335,5341,5348,5355,5361
dd 5368,5374,5381,5387,5394,5400,5407,5413,5420,5426
dd 5433,5439,5445,5452,5458,5464,5471,5477,5483,5489
dd 5496,5502,5508,5514,5521,5527,5533,5539,5545,5551
dd 5557,5563,5569,5575,5581,5587,5593,5599,5605,5611
dd 5617,5623,5629,5635,5640,5646,5652,5658,5664,5669
dd 5675,5681,5686,5692,5698,5703,5709,5715,5720,5726
dd 5731,5737,5742,5748,5753,5759,5764,5770,5775,5781
dd 5786,5791,5797,5802,5807,5813,5818,5823,5828,5834
dd 5839,5844,5849,5854,5859,5864,5870,5875,5880,5885
dd 5890,5895,5900,5905,5910,5915,5920,5924,5929,5934
dd 5939,5944,5949,5953,5958,5963,5968,5972,5977,5982
dd 5986,5991,5996,6000,6005,6009,6014,6019,6023,6028
dd 6032,6036,6041,6045,6050,6054,6059,6063,6067,6072
dd 6076,6080,6084,6089,6093,6097,6101,6105,6109,6114
dd 6118,6122,6126,6130,6134,6138,6142,6146,6150,6154
dd 6158,6162,6166,6169,6173,6177,6181,6185,6188,6192
dd 6196,6200,6203,6207,6211,6214,6218,6222,6225,6229
dd 6232,6236,6239,6243,6246,6250,6253,6257,6260,6263
dd 6267,6270,6273,6277,6280,6283,6286,6290,6293,6296
dd 6299,6302,6305,6309,6312,6315,6318,6321,6324,6327
dd 6330,6333,6336,6338,6341,6344,6347,6350,6353,6356
dd 6358,6361,6364,6367,6369,6372,6375,6377,6380,6382
dd 6385,6388,6390,6393,6395,6398,6400,6403,6405,6407
dd 6410,6412,6415,6417,6419,6421,6424,6426,6428,6430
dd 6433,6435,6437,6439,6441,6443,6445,6447,6449,6451
dd 6453,6455,6457,6459,6461,6463,6465,6467,6469,6471
dd 6472,6474,6476,6478,6479,6481,6483,6484,6486,6488
dd 6489,6491,6492,6494,6495,6497,6498,6500,6501,6503
dd 6504,6506,6507,6508,6510,6511,6512,6513,6515,6516
dd 6517,6518,6519,6521,6522,6523,6524,6525,6526,6527
dd 6528,6529,6530,6531,6532,6533,6534,6535,6535,6536
dd 6537,6538,6539,6539,6540,6541,6541,6542,6543,6543
dd 6544,6545,6545,6546,6546,6547,6547,6548,6548,6549
dd 6549,6549,6550,6550,6550,6551,6551,6551,6552,6552
dd 6552,6552,6552,6553,6553,6553,6553,6553,6553,6553
dd 6553,6553,6553,6553,6553,6553,6553,6553,6552,6552
dd 6552,6552,6552,6551,6551,6551,6550,6550,6550,6549
dd 6549,6549,6548,6548,6547,6547,6546,6546,6545,6545
dd 6544,6543,6543,6542,6541,6541,6540,6539,6539,6538
dd 6537,6536,6535,6535,6534,6533,6532,6531,6530,6529
dd 6528,6527,6526,6525,6524,6523,6522,6521,6519,6518
dd 6517,6516,6515,6513,6512,6511,6510,6508,6507,6506
dd 6504,6503,6501,6500,6498,6497,6495,6494,6492,6491
dd 6489,6488,6486,6484,6483,6481,6479,6478,6476,6474
dd 6472,6471,6469,6467,6465,6463,6461,6459,6457,6455
dd 6453,6451,6449,6447,6445,6443,6441,6439,6437,6435
dd 6433,6430,6428,6426,6424,6421,6419,6417,6415,6412
dd 6410,6407,6405,6403,6400,6398,6395,6393,6390,6388
dd 6385,6382,6380,6377,6375,6372,6369,6367,6364,6361
dd 6358,6356,6353,6350,6347,6344,6341,6338,6336,6333
dd 6330,6327,6324,6321,6318,6315,6312,6309,6305,6302
dd 6299,6296,6293,6290,6286,6283,6280,6277,6273,6270
dd 6267,6263,6260,6257,6253,6250,6246,6243,6239,6236
dd 6232,6229,6225,6222,6218,6214,6211,6207,6203,6200
dd 6196,6192,6188,6185,6181,6177,6173,6169,6166,6162
dd 6158,6154,6150,6146,6142,6138,6134,6130,6126,6122
dd 6118,6114,6109,6105,6101,6097,6093,6089,6084,6080
dd 6076,6072,6067,6063,6059,6054,6050,6045,6041,6036
dd 6032,6028,6023,6019,6014,6009,6005,6000,5996,5991
dd 5986,5982,5977,5972,5968,5963,5958,5953,5949,5944
dd 5939,5934,5929,5924,5920,5915,5910,5905,5900,5895
dd 5890,5885,5880,5875,5870,5864,5859,5854,5849,5844
dd 5839,5834,5828,5823,5818,5813,5807,5802,5797,5791
dd 5786,5781,5775,5770,5764,5759,5753,5748,5742,5737
dd 5731,5726,5720,5715,5709,5703,5698,5692,5686,5681
dd 5675,5669,5664,5658,5652,5646,5640,5635,5629,5623
dd 5617,5611,5605,5599,5593,5587,5581,5575,5569,5563
dd 5557,5551,5545,5539,5533,5527,5521,5514,5508,5502
dd 5496,5489,5483,5477,5471,5464,5458,5452,5445,5439
dd 5433,5426,5420,5413,5407,5400,5394,5387,5381,5374
dd 5368,5361,5355,5348,5341,5335,5328,5321,5315,5308
dd 5301,5295,5288,5281,5274,5268,5261,5254,5247,5240
dd 5233,5226,5220,5213,5206,5199,5192,5185,5178,5171
dd 5164,5157,5150,5143,5135,5128,5121,5114,5107,5100
dd 5093,5085,5078,5071,5064,5056,5049,5042,5034,5027
dd 5020,5012,5005,4998,4990,4983,4975,4968,4960,4953
dd 4945,4938,4930,4923,4915,4908,4900,4893,4885,4877
dd 4870,4862,4854,4847,4839,4831,4824,4816,4808,4800
dd 4792,4785,4777,4769,4761,4753,4745,4737,4730,4722
dd 4714,4706,4698,4690,4682,4674,4666,4658,4650,4642
dd 4634,4625,4617,4609,4601,4593,4585,4577,4568,4560
dd 4552,4544,4535,4527,4519,4511,4502,4494,4486,4477
dd 4469,4461,4452,4444,4435,4427,4419,4410,4402,4393
dd 4385,4376,4368,4359,4351,4342,4333,4325,4316,4308
dd 4299,4290,4282,4273,4264,4256,4247,4238,4230,4221
dd 4212,4203,4194,4186,4177,4168,4159,4150,4142,4133
dd 4124,4115,4106,4097,4088,4079,4070,4061,4052,4043
dd 4034,4025,4016,4007,3998,3989,3980,3971,3962,3953
dd 3943,3934,3925,3916,3907,3898,3888,3879,3870,3861
dd 3852,3842,3833,3824,3814,3805,3796,3786,3777,3768
dd 3758,3749,3740,3730,3721,3711,3702,3693,3683,3674
dd 3664,3655,3645,3636,3626,3617,3607,3598,3588,3578
dd 3569,3559,3550,3540,3530,3521,3511,3501,3492,3482
dd 3472,3463,3453,3443,3433,3424,3414,3404,3394,3385
dd 3375,3365,3355,3345,3336,3326,3316,3306,3296,3286
dd 3276,3266,3256,3246,3237,3227,3217,3207,3197,3187
dd 3177,3167,3157,3147,3137,3127,3117,3106,3096,3086
dd 3076,3066,3056,3046,3036,3026,3015,3005,2995,2985
dd 2975,2965,2954,2944,2934,2924,2913,2903,2893,2883
dd 2872,2862,2852,2841,2831,2821,2811,2800,2790,2779
dd 2769,2759,2748,2738,2728,2717,2707,2696,2686,2675
dd 2665,2655,2644,2634,2623,2613,2602,2592,2581,2571
dd 2560,2550,2539,2529,2518,2507,2497,2486,2476,2465
dd 2454,2444,2433,2423,2412,2401,2391,2380,2369,2359
dd 2348,2337,2327,2316,2305,2295,2284,2273,2262,2252
dd 2241,2230,2219,2209,2198,2187,2176,2166,2155,2144
dd 2133,2122,2111,2101,2090,2079,2068,2057,2046,2036
dd 2025,2014,2003,1992,1981,1970,1959,1948,1937,1926
dd 1916,1905,1894,1883,1872,1861,1850,1839,1828,1817
dd 1806,1795,1784,1773,1762,1751,1740,1729,1718,1707
dd 1696,1685,1674,1663,1651,1640,1629,1618,1607,1596
dd 1585,1574,1563,1552,1541,1529,1518,1507,1496,1485
dd 1474,1463,1451,1440,1429,1418,1407,1396,1384,1373
dd 1362,1351,1340,1328,1317,1306,1295,1284,1272,1261
dd 1250,1239,1228,1216,1205,1194,1183,1171,1160,1149
dd 1138,1126,1115,1104,1092,1081,1070,1059,1047,1036
dd 1025,1013,1002,991,979,968,957,946,934,923
dd 912,900,889,878,866,855,844,832,821,810
dd 798,787,775,764,753,741,730,719,707,696
dd 685,673,662,650,639,628,616,605,593,582
dd 571,559,548,536,525,514,502,491,479,468
dd 457,445,434,422,411,400,388,377,365,354
dd 342,331,320,308,297,285,274,263,251,240
dd 228,217,205,194,182,171,160,148,137,125
dd 114,102,91,80,68,57,45,34,22,11
dd 0,-12,-23,-35,-46,-58,-69,-81,-92,-103
dd -115,-126,-138,-149,-161,-172,-183,-195,-206,-218
dd -229,-241,-252,-264,-275,-286,-298,-309,-321,-332
dd -343,-355,-366,-378,-389,-401,-412,-423,-435,-446
dd -458,-469,-480,-492,-503,-515,-526,-537,-549,-560
dd -572,-583,-594,-606,-617,-629,-640,-651,-663,-674
dd -686,-697,-708,-720,-731,-742,-754,-765,-776,-788
dd -799,-811,-822,-833,-845,-856,-867,-879,-890,-901
dd -913,-924,-935,-947,-958,-969,-980,-992,-1003,-1014
dd -1026,-1037,-1048,-1060,-1071,-1082,-1093,-1105,-1116,-1127
dd -1139,-1150,-1161,-1172,-1184,-1195,-1206,-1217,-1229,-1240
dd -1251,-1262,-1273,-1285,-1296,-1307,-1318,-1329,-1341,-1352
dd -1363,-1374,-1385,-1397,-1408,-1419,-1430,-1441,-1452,-1464
dd -1475,-1486,-1497,-1508,-1519,-1530,-1542,-1553,-1564,-1575
dd -1586,-1597,-1608,-1619,-1630,-1641,-1652,-1664,-1675,-1686
dd -1697,-1708,-1719,-1730,-1741,-1752,-1763,-1774,-1785,-1796
dd -1807,-1818,-1829,-1840,-1851,-1862,-1873,-1884,-1895,-1906
dd -1917,-1927,-1938,-1949,-1960,-1971,-1982,-1993,-2004,-2015
dd -2026,-2037,-2047,-2058,-2069,-2080,-2091,-2102,-2112,-2123
dd -2134,-2145,-2156,-2167,-2177,-2188,-2199,-2210,-2220,-2231
dd -2242,-2253,-2263,-2274,-2285,-2296,-2306,-2317,-2328,-2338
dd -2349,-2360,-2370,-2381,-2392,-2402,-2413,-2424,-2434,-2445
dd -2455,-2466,-2477,-2487,-2498,-2508,-2519,-2530,-2540,-2551
dd -2561,-2572,-2582,-2593,-2603,-2614,-2624,-2635,-2645,-2656
dd -2666,-2676,-2687,-2697,-2708,-2718,-2729,-2739,-2749,-2760
dd -2770,-2780,-2791,-2801,-2812,-2822,-2832,-2842,-2853,-2863
dd -2873,-2884,-2894,-2904,-2914,-2925,-2935,-2945,-2955,-2966
dd -2976,-2986,-2996,-3006,-3016,-3027,-3037,-3047,-3057,-3067
dd -3077,-3087,-3097,-3107,-3118,-3128,-3138,-3148,-3158,-3168
dd -3178,-3188,-3198,-3208,-3218,-3228,-3238,-3247,-3257,-3267
dd -3277,-3287,-3297,-3307,-3317,-3327,-3337,-3346,-3356,-3366
dd -3376,-3386,-3395,-3405,-3415,-3425,-3434,-3444,-3454,-3464
dd -3473,-3483,-3493,-3502,-3512,-3522,-3531,-3541,-3551,-3560
dd -3570,-3579,-3589,-3599,-3608,-3618,-3627,-3637,-3646,-3656
dd -3665,-3675,-3684,-3694,-3703,-3712,-3722,-3731,-3741,-3750
dd -3759,-3769,-3778,-3787,-3797,-3806,-3815,-3825,-3834,-3843
dd -3853,-3862,-3871,-3880,-3889,-3899,-3908,-3917,-3926,-3935
dd -3944,-3954,-3963,-3972,-3981,-3990,-3999,-4008,-4017,-4026
dd -4035,-4044,-4053,-4062,-4071,-4080,-4089,-4098,-4107,-4116
dd -4125,-4134,-4143,-4151,-4160,-4169,-4178,-4187,-4195,-4204
dd -4213,-4222,-4231,-4239,-4248,-4257,-4265,-4274,-4283,-4291
dd -4300,-4309,-4317,-4326,-4334,-4343,-4352,-4360,-4369,-4377
dd -4386,-4394,-4403,-4411,-4420,-4428,-4436,-4445,-4453,-4462
dd -4470,-4478,-4487,-4495,-4503,-4512,-4520,-4528,-4536,-4545
dd -4553,-4561,-4569,-4578,-4586,-4594,-4602,-4610,-4618,-4626
dd -4635,-4643,-4651,-4659,-4667,-4675,-4683,-4691,-4699,-4707
dd -4715,-4723,-4731,-4738,-4746,-4754,-4762,-4770,-4778,-4786
dd -4793,-4801,-4809,-4817,-4825,-4832,-4840,-4848,-4855,-4863
dd -4871,-4878,-4886,-4894,-4901,-4909,-4916,-4924,-4931,-4939
dd -4946,-4954,-4961,-4969,-4976,-4984,-4991,-4999,-5006,-5013
dd -5021,-5028,-5035,-5043,-5050,-5057,-5065,-5072,-5079,-5086
dd -5094,-5101,-5108,-5115,-5122,-5129,-5136,-5144,-5151,-5158
dd -5165,-5172,-5179,-5186,-5193,-5200,-5207,-5214,-5221,-5227
dd -5234,-5241,-5248,-5255,-5262,-5269,-5275,-5282,-5289,-5296
dd -5302,-5309,-5316,-5322,-5329,-5336,-5342,-5349,-5356,-5362
dd -5369,-5375,-5382,-5388,-5395,-5401,-5408,-5414,-5421,-5427
dd -5434,-5440,-5446,-5453,-5459,-5465,-5472,-5478,-5484,-5490
dd -5497,-5503,-5509,-5515,-5522,-5528,-5534,-5540,-5546,-5552
dd -5558,-5564,-5570,-5576,-5582,-5588,-5594,-5600,-5606,-5612
dd -5618,-5624,-5630,-5636,-5641,-5647,-5653,-5659,-5665,-5670
dd -5676,-5682,-5687,-5693,-5699,-5704,-5710,-5716,-5721,-5727
dd -5732,-5738,-5743,-5749,-5754,-5760,-5765,-5771,-5776,-5782
dd -5787,-5792,-5798,-5803,-5808,-5814,-5819,-5824,-5829,-5835
dd -5840,-5845,-5850,-5855,-5860,-5865,-5871,-5876,-5881,-5886
dd -5891,-5896,-5901,-5906,-5911,-5916,-5921,-5925,-5930,-5935
dd -5940,-5945,-5950,-5954,-5959,-5964,-5969,-5973,-5978,-5983
dd -5987,-5992,-5997,-6001,-6006,-6010,-6015,-6020,-6024,-6029
dd -6033,-6037,-6042,-6046,-6051,-6055,-6060,-6064,-6068,-6073
dd -6077,-6081,-6085,-6090,-6094,-6098,-6102,-6106,-6110,-6115
dd -6119,-6123,-6127,-6131,-6135,-6139,-6143,-6147,-6151,-6155
dd -6159,-6163,-6167,-6170,-6174,-6178,-6182,-6186,-6189,-6193
dd -6197,-6201,-6204,-6208,-6212,-6215,-6219,-6223,-6226,-6230
dd -6233,-6237,-6240,-6244,-6247,-6251,-6254,-6258,-6261,-6264
dd -6268,-6271,-6274,-6278,-6281,-6284,-6287,-6291,-6294,-6297
dd -6300,-6303,-6306,-6310,-6313,-6316,-6319,-6322,-6325,-6328
dd -6331,-6334,-6337,-6339,-6342,-6345,-6348,-6351,-6354,-6357
dd -6359,-6362,-6365,-6368,-6370,-6373,-6376,-6378,-6381,-6383
dd -6386,-6389,-6391,-6394,-6396,-6399,-6401,-6404,-6406,-6408
dd -6411,-6413,-6416,-6418,-6420,-6422,-6425,-6427,-6429,-6431
dd -6434,-6436,-6438,-6440,-6442,-6444,-6446,-6448,-6450,-6452
dd -6454,-6456,-6458,-6460,-6462,-6464,-6466,-6468,-6470,-6472
dd -6473,-6475,-6477,-6479,-6480,-6482,-6484,-6485,-6487,-6489
dd -6490,-6492,-6493,-6495,-6496,-6498,-6499,-6501,-6502,-6504
dd -6505,-6507,-6508,-6509,-6511,-6512,-6513,-6514,-6516,-6517
dd -6518,-6519,-6520,-6522,-6523,-6524,-6525,-6526,-6527,-6528
dd -6529,-6530,-6531,-6532,-6533,-6534,-6535,-6536,-6536,-6537
dd -6538,-6539,-6540,-6540,-6541,-6542,-6542,-6543,-6544,-6544
dd -6545,-6546,-6546,-6547,-6547,-6548,-6548,-6549,-6549,-6550
dd -6550,-6550,-6551,-6551,-6551,-6552,-6552,-6552,-6553,-6553
dd -6553,-6553,-6553,-6554,-6554,-6554,-6554,-6554,-6554,-6554
dd -6554,-6554,-6554,-6554,-6554,-6554,-6554,-6554,-6553,-6553
dd -6553,-6553,-6553,-6552,-6552,-6552,-6551,-6551,-6551,-6550
dd -6550,-6550,-6549,-6549,-6548,-6548,-6547,-6547,-6546,-6546
dd -6545,-6544,-6544,-6543,-6542,-6542,-6541,-6540,-6540,-6539
dd -6538,-6537,-6536,-6536,-6535,-6534,-6533,-6532,-6531,-6530
dd -6529,-6528,-6527,-6526,-6525,-6524,-6523,-6522,-6520,-6519
dd -6518,-6517,-6516,-6514,-6513,-6512,-6511,-6509,-6508,-6507
dd -6505,-6504,-6502,-6501,-6499,-6498,-6496,-6495,-6493,-6492
dd -6490,-6489,-6487,-6485,-6484,-6482,-6480,-6479,-6477,-6475
dd -6473,-6472,-6470,-6468,-6466,-6464,-6462,-6460,-6458,-6456
dd -6454,-6452,-6450,-6448,-6446,-6444,-6442,-6440,-6438,-6436
dd -6434,-6431,-6429,-6427,-6425,-6422,-6420,-6418,-6416,-6413
dd -6411,-6408,-6406,-6404,-6401,-6399,-6396,-6394,-6391,-6389
dd -6386,-6383,-6381,-6378,-6376,-6373,-6370,-6368,-6365,-6362
dd -6359,-6357,-6354,-6351,-6348,-6345,-6342,-6339,-6337,-6334
dd -6331,-6328,-6325,-6322,-6319,-6316,-6313,-6309,-6306,-6303
dd -6300,-6297,-6294,-6291,-6287,-6284,-6281,-6278,-6274,-6271
dd -6268,-6264,-6261,-6258,-6254,-6251,-6247,-6244,-6240,-6237
dd -6233,-6230,-6226,-6223,-6219,-6215,-6212,-6208,-6204,-6201
dd -6197,-6193,-6189,-6186,-6182,-6178,-6174,-6170,-6167,-6163
dd -6159,-6155,-6151,-6147,-6143,-6139,-6135,-6131,-6127,-6123
dd -6119,-6115,-6110,-6106,-6102,-6098,-6094,-6090,-6085,-6081
dd -6077,-6073,-6068,-6064,-6060,-6055,-6051,-6046,-6042,-6037
dd -6033,-6029,-6024,-6020,-6015,-6010,-6006,-6001,-5997,-5992
dd -5987,-5983,-5978,-5973,-5969,-5964,-5959,-5954,-5950,-5945
dd -5940,-5935,-5930,-5925,-5921,-5916,-5911,-5906,-5901,-5896
dd -5891,-5886,-5881,-5876,-5871,-5865,-5860,-5855,-5850,-5845
dd -5840,-5835,-5829,-5824,-5819,-5814,-5808,-5803,-5798,-5792
dd -5787,-5782,-5776,-5771,-5765,-5760,-5754,-5749,-5743,-5738
dd -5732,-5727,-5721,-5716,-5710,-5704,-5699,-5693,-5687,-5682
dd -5676,-5670,-5665,-5659,-5653,-5647,-5641,-5636,-5630,-5624
dd -5618,-5612,-5606,-5600,-5594,-5588,-5582,-5576,-5570,-5564
dd -5558,-5552,-5546,-5540,-5534,-5528,-5522,-5515,-5509,-5503
dd -5497,-5490,-5484,-5478,-5472,-5465,-5459,-5453,-5446,-5440
dd -5434,-5427,-5421,-5414,-5408,-5401,-5395,-5388,-5382,-5375
dd -5369,-5362,-5356,-5349,-5342,-5336,-5329,-5322,-5316,-5309
dd -5302,-5296,-5289,-5282,-5275,-5269,-5262,-5255,-5248,-5241
dd -5234,-5227,-5221,-5214,-5207,-5200,-5193,-5186,-5179,-5172
dd -5165,-5158,-5151,-5144,-5136,-5129,-5122,-5115,-5108,-5101
dd -5094,-5086,-5079,-5072,-5065,-5057,-5050,-5043,-5035,-5028
dd -5021,-5013,-5006,-4999,-4991,-4984,-4976,-4969,-4961,-4954
dd -4946,-4939,-4931,-4924,-4916,-4909,-4901,-4894,-4886,-4878
dd -4871,-4863,-4855,-4848,-4840,-4832,-4825,-4817,-4809,-4801
dd -4793,-4786,-4778,-4770,-4762,-4754,-4746,-4738,-4731,-4723
dd -4715,-4707,-4699,-4691,-4683,-4675,-4667,-4659,-4651,-4643
dd -4635,-4626,-4618,-4610,-4602,-4594,-4586,-4578,-4569,-4561
dd -4553,-4545,-4536,-4528,-4520,-4512,-4503,-4495,-4487,-4478
dd -4470,-4462,-4453,-4445,-4436,-4428,-4420,-4411,-4403,-4394
dd -4386,-4377,-4369,-4360,-4352,-4343,-4334,-4326,-4317,-4309
dd -4300,-4291,-4283,-4274,-4265,-4257,-4248,-4239,-4231,-4222
dd -4213,-4204,-4195,-4187,-4178,-4169,-4160,-4151,-4143,-4134
dd -4125,-4116,-4107,-4098,-4089,-4080,-4071,-4062,-4053,-4044
dd -4035,-4026,-4017,-4008,-3999,-3990,-3981,-3972,-3963,-3954
dd -3944,-3935,-3926,-3917,-3908,-3899,-3889,-3880,-3871,-3862
dd -3853,-3843,-3834,-3825,-3815,-3806,-3797,-3787,-3778,-3769
dd -3759,-3750,-3741,-3731,-3722,-3712,-3703,-3694,-3684,-3675
dd -3665,-3656,-3646,-3637,-3627,-3618,-3608,-3599,-3589,-3579
dd -3570,-3560,-3551,-3541,-3531,-3522,-3512,-3502,-3493,-3483
dd -3473,-3464,-3454,-3444,-3434,-3425,-3415,-3405,-3395,-3386
dd -3376,-3366,-3356,-3346,-3337,-3327,-3317,-3307,-3297,-3287
dd -3277,-3267,-3257,-3247,-3238,-3228,-3218,-3208,-3198,-3188
dd -3178,-3168,-3158,-3148,-3138,-3128,-3118,-3107,-3097,-3087
dd -3077,-3067,-3057,-3047,-3037,-3027,-3016,-3006,-2996,-2986
dd -2976,-2966,-2955,-2945,-2935,-2925,-2914,-2904,-2894,-2884
dd -2873,-2863,-2853,-2842,-2832,-2822,-2812,-2801,-2791,-2780
dd -2770,-2760,-2749,-2739,-2729,-2718,-2708,-2697,-2687,-2676
dd -2666,-2656,-2645,-2635,-2624,-2614,-2603,-2593,-2582,-2572
dd -2561,-2551,-2540,-2530,-2519,-2508,-2498,-2487,-2477,-2466
dd -2455,-2445,-2434,-2424,-2413,-2402,-2392,-2381,-2370,-2360
dd -2349,-2338,-2328,-2317,-2306,-2296,-2285,-2274,-2263,-2253
dd -2242,-2231,-2220,-2210,-2199,-2188,-2177,-2167,-2156,-2145
dd -2134,-2123,-2112,-2102,-2091,-2080,-2069,-2058,-2047,-2037
dd -2026,-2015,-2004,-1993,-1982,-1971,-1960,-1949,-1938,-1927
dd -1917,-1906,-1895,-1884,-1873,-1862,-1851,-1840,-1829,-1818
dd -1807,-1796,-1785,-1774,-1763,-1752,-1741,-1730,-1719,-1708
dd -1697,-1686,-1675,-1664,-1652,-1641,-1630,-1619,-1608,-1597
dd -1586,-1575,-1564,-1553,-1542,-1530,-1519,-1508,-1497,-1486
dd -1475,-1464,-1452,-1441,-1430,-1419,-1408,-1397,-1385,-1374
dd -1363,-1352,-1341,-1329,-1318,-1307,-1296,-1285,-1273,-1262
dd -1251,-1240,-1229,-1217,-1206,-1195,-1184,-1172,-1161,-1150
dd -1139,-1127,-1116,-1105,-1093,-1082,-1071,-1060,-1048,-1037
dd -1026,-1014,-1003,-992,-980,-969,-958,-947,-935,-924
dd -913,-901,-890,-879,-867,-856,-845,-833,-822,-811
dd -799,-788,-776,-765,-754,-742,-731,-720,-708,-697
dd -686,-674,-663,-651,-640,-629,-617,-606,-594,-583
dd -572,-560,-549,-537,-526,-515,-503,-492,-480,-469
dd -458,-446,-435,-423,-412,-401,-389,-378,-366,-355
dd -343,-332,-321,-309,-298,-286,-275,-264,-252,-241
dd -229,-218,-206,-195,-183,-172,-161,-149,-138,-126
dd -115,-103,-92,-81,-69,-58,-46,-35,-23,-12
   
eosinus:
   
; misc vars for mouse / keys:
wonder:
 dd 7777
dududu:
 dd 0
rendered_quads:
 dd 0
sorted_quads:
 dd 0
vxx:
 dd 0
mouseya:
 dd 0
vpx:
 dd 0
vpy:
 dd 0
vheading:
 dd 0
   
wtolong:
 dw 0,0
   
; misc Engine Vars --------------------------
imgtxt:
 dd 0
mousex:
 dd 0
mousey:
 dd 0
factor:
 dd 0
i:
 dd 0
miny:
 dd 0
maxy:
 dd 0
pwidth:
 dd 0
pheight:
 dd 0
;anz:
; dd 0
a:
 dd 0
alpha:
 dd 0
beta:
 dd 0
gamma:
 dd 0
zoom:
 dd 0
zwmax:
 dd 0
i2:
 dd 0
x1:
 dd 0
y1:
 dd 0
x2:
 dd 0
y2:
 dd 0
   
count:
 dd 0
ilocal:
 dd 0
ycoord:
 dd 0
pside:
 dd 0
temp:
 dd 0
lineheight:
 dd 0
linewidth:
 dd 0
v_xadd:
 dd 0
pxadd:
 dd 0
pyadd:
 dd 0
ytop:
 dd 0
ytopy:
 dd 0
px:
 dd 0
py:
 dd 0
x:
 dd 0
y:
 dd 0
polyx1:
 dd 0
polyx2:
 dd 0
px1:
 dd 0
py1:
 dd 0
px2:
 dd 0
py2:
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
singamma:
 dd 0
sinbeta:
 dd 0
sinalpha:
 dd 0
alphacopy:
 dd 0
betacopy:
 dd 0
gammacopy:
 dd 0
   
   
;;fixed size arrays --------------------
lefttable:
 times 1920*2 dd 0 ; 1440
righttable:
 times 1920*2 dd 0
polypoints:
 times 12 dd 0 ; 8
zbuffer:
 times 10010 dd 0 ; 10000
   
; floating vars ------------------------
   
twohundred:
 dd 200
mausy:
 dd 0
   
xl1:
 dd 0
yl1:
 dd 0
zl1:
 dd 0
   
xl2:
 dd 0
yl2:
 dd 0
zl2:
 dd 0
   
xl3:
 dd 0
yl3:
 dd 0
zl3:
 dd 0
   
xloc:
 dd 0
yloc:
 dd 0
   
tempdiv:
 dd 0
   
   
tex1:
; the texture ( dd 0xRRGGBB,0xRRGGBB... 64*64*32 Bit)
include "wall3.inc"
   
   
labelt:
      db   'MOS3DE',0
   
labellen:
   
I_END:
   
   
   
   