; mov [pointer_to_screen],eax
; mov [output_array],ebx
; mov [screen_size_x],ecx
;setpixel (pixel_x,pixel_y)
setpixel:

  mov ecx,[output_array]
  mov eax,[pixel_y]
  mov ebx,[screen_size_x]
  imul eax,ebx
  add eax,[pixel_x]
  lea eax,[eax+eax*2]
  add eax,[pointer_to_screen]
  mov [ecx],eax
  add [output_array],4
  ret
; procedure <<<<<<PutPixel>>>>>>
; IN
; eax - pointer to picture
; ebx - picture size x
; ecx - color of pixel
; esi - x coordinat of pixel
; edi - y coordinat of pixel
; OUT
; not returned value
PutPixel:

  imul ebx,edi
  add ebx,esi
  lea ebx,[ebx+ebx*2]
  add eax,ebx
  mov ebx,ecx
  shr ebx,16
  mov [eax],cx
  mov [eax+2],bl
  ret
; function <<<<<<GetColorOfPixel>>>>>>
; IN
; eax - pointer to picture
; ebx - picture size x
; esi - x coordinat of pixel
; edi - y coordinat of pixel
; OUT
; eax - color of pixel in coordinats (x,y)
GetColorOfPixel:

  imul ebx,edi
  add ebx,esi
  lea ebx,[ebx+ebx*2]
  add eax,ebx
  mov ebx,[eax]
  and ebx,0xffffff
  mov eax,ebx
  ret

;procedure <<<<calculate line>>>>
; IN
;eax - pointer to screen
;ebx - output array
;ecx - (screen size x)*bytes per pixel
;edx - x1*65536+y1
;esi   x2
;edi - y2
; OUT
;eax - number of pixels
calculate_line:

  mov [pointer_to_screen],eax
  mov [output_array],ebx
  mov [screen_size_x],ecx
  mov eax,edx
  and eax,0xffff
  shr edx,16
  mov [x_l],edx     ;x=x1
  mov [y_l],eax     ;y=y1
  mov [sx],1
  mov [sy],1

  mov edx,[output_array]

  sub esi,[x_l]     ;esi=x2-x1
  jnz no_0_x
  and esi,0
  jmp x_no_minus
  no_0_x:
  jns x_no_minus
  neg esi
  mov [sx],-1	  ;sx=-sx
  x_no_minus:

  sub edi,[y_l]     ;edi=y2-y1
  jnz no_0_y
  and edi,0
  jmp y_no_minus
  no_0_y:
  jns y_no_minus
  neg edi
  mov [sy],-1	  ;sy=-sy
  y_no_minus:

  mov [_dx],esi
  mov [_dy],edi

  cmp [_dx],0	   ;if (dx=0 & dy=0) {  }
  jnz no_null_d
  cmp [_dy],0
  jnz no_null_d
  mov eax,[x_l]
  mov ebx,[y_l]
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel    ;setpixel(x1,y1)
  mov eax,1
  ret
  no_null_d:

  cmp edi,esi	   ;if (dy>dx) {  }
  jle no_bigger
  mov eax,[_dx]
  mov [z],eax	   ;z=dx
  mov eax,[_dy]
  mov [_dx],eax    ;dx=dy
  mov eax,[z]
  mov [_dy],eax    ;dy=z
  mov [_ch],dword 1 ;ch=true
  jmp exit_if2
  no_bigger:	   ;else {}
  and [_ch],0
  exit_if2:

  mov eax,[_dy]
  shl eax,1
  sub eax,[_dx]
  mov [e],eax	 ;e=2*dy-dx

  mov esi,1	 ;counter
  mov edi,[sx]
  mov ebp,[sy]
  do:

     mov eax,[x_l]
     mov ebx,[y_l]
     mov [pixel_x],eax
     mov [pixel_y],ebx
     call setpixel

     cmp [e],0
     js while_e

       mov eax,[_ch]
       test eax,eax
       jz ch_else
       add [x_l],edi
       jmp exit_if3
       ch_else:
       add [y_l],ebp
       exit_if3:

       mov eax,[_dx]
       shl eax,1
       sub [e],eax    ;e=e-2*dx

     while_e:

     mov eax,[_ch]
     test eax,eax
     jz ch_else2
     add [y_l],ebp
     jmp exit_if4
     ch_else2:
     add [x_l],edi
     exit_if4:

     mov eax,[_dy]
     shl eax,1
     add [e],eax ;e=e+2*dy
     inc esi

     mov ecx,[_dx]
     cmp esi,ecx
     jle do	     ;while (i<=dx)

     mov eax,[x_l]
     mov ebx,[y_l]
     mov [pixel_x],eax
     mov [pixel_y],ebx
     call setpixel

     mov eax,[output_array]
     sub eax,edx
     shr eax,2

  ret
;---------------------------
;procedure calculate <<<<<rectangle>>>>>>>
; IN
;eax - pointer to screen
;ebx - output array
;ecx - (screen size x)
;edx - x1*65536+y1
;esi   x2
;edi - y2
; OUT
;eax - number of pixels

calculate_rectangle:

  mov [pointer_to_screen],eax
  mov [output_array],ebx
  mov [screen_size_x],ecx
  mov eax,edx
  and eax,0xffff
  shr edx,16
  mov [x_l],edx     ;x=x1
  mov [y_l],eax     ;y=y1
  mov [x2],esi	    ;x2
  mov [y2],edi	    ;y2
  mov [sx],1
  mov [sy],1

  mov edx,[output_array]

  sub esi,[x_l]     ;esi=x2-x1
  jnz no_0_x_r
  and esi,0
  jmp x_no_minus_r
  no_0_x_r:
  jns x_no_minus_r
  neg esi
  mov [sx],-1	  ;sx=-sx
  x_no_minus_r:

  sub edi,[y_l]     ;edi=y2-y1
  jnz no_0_y_r
  and edi,0
  jmp y_no_minus_r
  no_0_y_r:
  jns y_no_minus_r
  neg edi
  mov [sy],-1	  ;sy=-sy
  y_no_minus_r:

  mov [_dx],esi
  mov [_dy],edi

  cmp [_dx],0	   ;if (dx=0 & dy=0) {  }
  jnz no_null_r
  cmp [_dy],0
  jnz no_null_r
  mov eax,[x_l]
  mov ebx,[y_l]
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel    ;setpixel(x1,y1)
  mov eax,1
  ret
  no_null_r:

  mov edi,[_dx]
  mov esi,[x_l]       ;x1

  horizontal_lines:

  mov eax,esi	      ;x
  mov ebx,[y_l]       ;y1
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,esi	      ;x
  mov ebx,[y2]	      ;y2
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  add esi,[sx]
  dec edi
  jns horizontal_lines

  mov edi,[_dy]
  mov esi,[y_l]       ;y1
  vertical_lines:

  mov eax,[x_l]       ;x1
  mov ebx,esi	      ;y
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x2]	       ;x2
  mov ebx,esi	       ;y
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  add esi,[sy]
  dec edi
  jns vertical_lines

  mov eax,[output_array]
  sub eax,edx
  shr eax,2

  ret
;procedure <<<<<<calculate_circle>>>>>>
; IN
;eax - pointer to screen
;ebx - output array
;ecx - (screen size x)*bytes per pixel
;edx - x_c*65536+y_c
;esi   r
; OUT
;eax - number of pixels

calculate_circle:

  mov [pointer_to_screen],eax
  mov [output_array],ebx
  mov [screen_size_x],ecx
  mov eax,edx
  and eax,0xffff
  shr edx,16
  mov [x2],edx	   ;circle centr x
  mov [y2],eax	   ;circle centr y
  mov [r],esi	   ;radius of cicrle

  mov edx,[output_array]

  mov [x_l],0	   ;x=0
  mov [y_l],esi    ;y=r
  mov [d],3
  mov eax,[r]
  shl eax,1
  sub [d],eax	   ;d=3-2*r


  while_circle:

  mov esi,[y_l]
  mov edi,[x_l]

  sub esi,edi	   ;while y>=x
  js exit_while_circle


  mov eax,[x_l] ;x
  mov ebx,[y_l] ;y
  add eax,[x2]	;x+xc
  add ebx,[y2]	;y+yc
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x_l] ;x
  mov ebx,[y2]	;yc
  add eax,[x2]	;x+xc
  sub ebx,[y_l] ;yc-y
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x2]	;xc
  mov ebx,[y2]	;yc
  sub eax,[x_l] ;xc-x
  add ebx,[y_l] ;yc+y
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x2]	;xc
  mov ebx,[y2]	;yc
  sub eax,[x_l] ;xc-x
  sub ebx,[y_l] ;yc-y
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x2]	;xc
  mov ebx,[y2]	;yc
  add eax,[y_l] ;xc+y
  add ebx,[x_l] ;yc+x
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x2]	;xc
  mov ebx,[y2]	;yc
  add eax,[y_l] ;xc+y
  sub ebx,[x_l] ;yc-x
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x2]	;xc
  mov ebx,[y2]	;yc
  sub eax,[y_l] ;xc-y
  add ebx,[x_l] ;x+yc
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[x2]	;xc
  mov ebx,[y2]	;yc
  sub eax,[y_l] ;xc-y
  sub ebx,[x_l] ;yc-x
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  cmp [d],0
  jns if1_d
  mov eax,[x_l]
  shl eax,2
  add eax,6
  add [d],eax  ;d=d+4*x+6
  jmp exit_if1_d
  if1_d:
  mov eax,[x_l]
  sub eax,[y_l]
  shl eax,2
  add eax,10
  add [d],eax	 ;d=d+4*(x-y)+10
  dec [y_l]
  exit_if1_d:
  inc [x_l]

  jmp while_circle

  exit_while_circle:

  mov eax,[output_array]
  sub eax,edx
  shr eax,2

  ret
;procedure <<<<<<flood fill>>>>>
;IN
;eax - pointer to screen
;ebx - pointer to output array
;ecx - picture size x
;edx - x0*65536+y0
;esi - x_max*65536+y_max
;edi - color of flood fill arrea
flood_fill:

  mov [pointer_to_screen],eax
  mov [output_array],ebx
  mov [screen_size_x],ecx

  mov eax,edx
  and edx,0xffff
  shr eax,16
  mov [sx],eax	 ;x0
  mov [sy],edx	 ;y0

  mov eax,esi
  and esi,0xffff
  shr eax,16
  mov [m_x],eax  ;maximum x size of picture
  mov [m_y],esi  ;maximum y size of picture

  mov [c_f],edi    ;color of feel

  mov eax,[output_array] ; stek
  mov [eax],dword 1	 ; number of pointes in stek
  mov esi,[sx]
  mov edi,[sy]
  mov [eax+4],si	;x0
  mov [eax+4+2],di	  ;y0

  mov eax,[pointer_to_screen]
  mov ebx,[screen_size_x]
  call GetColorOfPixel
  mov [c],eax

  cmp eax,[c_f]
  jne can_flood_fill
  mov eax,[output_array]
  mov [eax],dword 0
  ret
  can_flood_fill:

  while_no_null:
  ;извлекаем координаты из стека
  mov eax,[output_array]
  mov ebx,[eax]
  dec ebx
  shl ebx,2		  ;ebx=ebx*8
  add ebx,4		  ;ebx=ebx*8+4
  xor esi,esi
  xor edi,edi
  mov si,[eax+ebx]	 ;x
  mov di,[eax+ebx+2]	 ;y
  mov [x2],esi
  mov [y2],edi

  mov [sy],edi

  mov eax,[output_array]
  mov ebx,[eax]
  dec ebx
  mov [eax],ebx 	  ;

  ; go to right until color=c or until x<MaxXPicture
  mov edx,[x2]
  go_to_right:

  mov eax,[pointer_to_screen]
  mov ebx,[screen_size_x]
  mov esi,edx;
  mov edi,[sy]
  call GetColorOfPixel

  cmp eax,[c]
  jne not_color_of_feeling_arrea_right

  mov eax,[pointer_to_screen]
  mov ebx,[screen_size_x]
  mov ecx,[c_f]    ;color of filinf flood
  mov esi,edx
  mov edi,[sy]
  call PutPixel

  inc edx

  mov eax,edx
  cmp eax,[m_x]
  jle go_to_right

  not_color_of_feeling_arrea_right:

  mov eax,edx
  dec eax		 ;----------------------------------
  mov [x_r],eax      ; save right absciss


  ; go to left until color=c or until x>0
  mov esi,[x2]
  mov edi,[y2]
  dec esi
  mov edx,esi
  mov [sy],edi

  go_to_left:

  mov eax,[pointer_to_screen]
  mov ebx,[screen_size_x]
  mov esi,edx
  mov edi,[sy]
  call GetColorOfPixel

  cmp eax,[c]
  jne not_color_of_feeling_arrea_left

  mov eax,[pointer_to_screen]
  mov ebx,[screen_size_x]
  mov ecx,[c_f]    ;color of filing flood
  mov esi,edx
  mov edi,[sy]
  call PutPixel

  dec edx
  jns go_to_left

  not_color_of_feeling_arrea_left:

  mov eax,edx
  inc eax	    ;----------------------------------------
  mov [x_l],eax      ; save left absciss

  mov edi,[y2]
  dec edi
  jns no_null_1_floodfill
  mov edi,1
  no_null_1_floodfill:
  mov [sy],edi

  mov eax,[output_array]
  mov ebx,[eax]
  mov [l_s],ebx 	; save total number of points in stek
  mov [l_c],-1		; last color = -1
  mov edx,[x_l]
  analizing_from_left_to_right_upper:
  ;анализируем пиксели над линией с левого края
  mov eax,[pointer_to_screen]
  mov ebx,[screen_size_x]
  mov esi,edx
  mov edi,[sy]
  call GetColorOfPixel
  push eax

  cmp eax,[c]	 ;если цвет проверямого пикселя не равен с,то не заносим его в стек
  jne no_fill_1
  mov ebx,[l_c]
  cmp ebx,[c]
  jne no_fill_1
  mov edi,[sy]
  mov eax,[output_array]
  mov ebx,[eax]
  inc ebx
  mov [eax],ebx
  dec ebx
  shl ebx,2
  add ebx,4
  mov esi,edx
  dec esi
  mov [eax+ebx],si
  mov [eax+ebx+2],di
  no_fill_1:

  pop [l_c]
  inc edx
  cmp edx,[x_r]
  jle analizing_from_left_to_right_upper

  mov ebx,[l_c]
  cmp ebx,[c]	    ;last color is c(color of filing arrea) ?
  jne have_changes_in_stek_1
  ;save last color in stek
  mov edi,[sy]
  mov eax,[output_array]
  mov ebx,[eax]
  inc ebx
  mov [eax],ebx
  dec ebx
  shl ebx,2
  add ebx,4
  mov esi,edx
  dec esi
  mov [eax+ebx],si
  mov [eax+ebx+2],di

  have_changes_in_stek_1:

  mov edi,[y2]
  inc edi
  cmp edi,[m_y]
  jle no_max_1_floodfill
  mov edi,[m_y]
  no_max_1_floodfill:
  mov [sy],edi

  mov eax,[output_array]
  mov ebx,[eax]
  mov [l_s],ebx 	; save total number of points in stek
  mov [l_c],-1		; last color = -1
  mov edx,[x_l]
  analizing_from_left_to_right_down:

  ;анализируем пиксели под линией с левого края
  mov eax,[pointer_to_screen]
  mov ebx,[screen_size_x]
  mov esi,edx
  mov edi,[sy]
  call GetColorOfPixel
  push eax

  cmp eax,[c]	 ;если цвет проверямого пикселя не равен с,то не заносим его в стек
  je no_fill_3
  mov ebx,[l_c]
  cmp ebx,[c]
  jne no_fill_3
  mov edi,[sy]
  mov eax,[output_array]
  mov ebx,[eax]
  inc ebx
  mov [eax],ebx
  dec ebx
  shl ebx,2
  add ebx,4
  mov esi,edx
  dec esi
  mov [eax+ebx],si
  mov [eax+ebx+2],di
  no_fill_3:

  pop [l_c]
  inc edx
  cmp edx,[x_r]
  jle analizing_from_left_to_right_down

  mov ebx,[l_c]
  cmp ebx,[c]	    ;last color is c(color of filing arrea) ?
  jne have_changes_in_stek_2
  ;save last color in stek
  mov edi,[sy]
  mov eax,[output_array]
  mov ebx,[eax]
  inc ebx
  mov [eax],ebx
  dec ebx
  shl ebx,2
  add ebx,4
  mov esi,edx
  dec esi
  mov [eax+ebx],si
  mov [eax+ebx+2],di

  have_changes_in_stek_2:

  mov ebx,[output_array] ;while stek have points  пока в стеке есть точки(координаты)
  mov ecx,[ebx]
  test ecx,ecx
  jnz while_no_null


  ret
; procedure <<<<<<calculate ellips>>>>>>
; IN
; eax - pointer to picture
; ebx - output array
; ecx - picture size x
; edx - x0*65536+y0
; esi - a*65536+b
; edi - color
; OUT
; procedure not return value
calculate_ellips:

  mov [pointer_to_screen],eax
  mov [output_array],ebx
  mov [screen_size_x],ecx
  mov [e],ebx

  mov eax,edx
  and eax,0xffff
  shr edx,16
  mov [x2],edx	   ; centr x
  mov [y2],eax	   ; centr y

  mov eax,esi
  and eax,0xffff   ;eax=b
  shr esi,16	   ;esi=a
  mov [a],esi
  mov [b],eax

  mov [c],edi	   ;color
  mov [row],eax ;b

  imul esi,esi ;a*a
  imul eax,eax ;b*b

  mov [a_square],esi
  mov [b_square],eax
  and [col],0

  shl esi,1
  shl eax,1
  mov [two_a_square],esi
  mov [two_b_square],eax
  shl esi,1
  shl eax,1
  mov [four_a_square],esi
  mov [four_b_square],eax

  mov eax,[row] 	     ;eax=row
  mov ebx,eax
  dec ebx		     ;ebx=(row-1)
  imul ebx,eax		     ;ebx=(row-1)*row
  imul ebx,[two_a_square]    ;ebx=two_a_square*((row-1)*row)
  mov ecx,[a_square]	     ;ecx=a_square
  mov eax,1
  sub eax,ecx		     ;eax=(1-a_square)
  imul eax,[two_b_square]    ;eax=two_b_square*(1-a_square)
  add ebx,[a_square]	     ;ebx=two_a_square*((row-1)*row)+a_square
  add eax,ebx		     ;eax=two_a_square*((row-1)*row)+a_square+two_b_square*(1-a_square)
  mov [d],eax

  mov edx,[x2]		     ;x
  mov ebp,[y2]		     ;y

  while_ellips_1:
  mov esi,[a_square]
  mov edi,[b_square]
  imul esi,[row]
  imul edi,[col]
  cmp esi,edi
  jle no_while_ellips_1

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  add eax,[col]      ;x+col
  add ebx,[row]      ;y+row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  add eax,[col]      ;x+col
  sub ebx,[row]      ;y-row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  sub eax,[col]      ;x-col
  add ebx,[row]      ;y+row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  sub eax,[col]      ;x-col
  sub ebx,[row]      ;y-row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[d]
  cmp eax,0
  js  lab33		 ;>=0
  dec [row]
  mov ebx,[four_a_square]
  mov ecx,[row]
  imul ebx,ecx		;ebx=four_a_square*(row)
  sub [d],ebx		;d=d-four_a_square*(row)
  lab33:

  mov eax,[col]
  shl eax,1
  add eax,3
  imul eax,[two_b_square]
  add [d],eax		;d=d+two_b_square*(3+(col*2))

  inc [col]
  jmp while_ellips_1

  no_while_ellips_1:

  mov eax,[col]
  inc eax
  imul eax,[col]
  imul eax,[two_b_square] ;eax=two_b_square*((col+1)*col)
  mov ebx,[row]
  sub ebx,2
  imul ebx,[row]
  inc ebx
  imul ebx,[two_a_square] ;ebx=two_a_square*((row-2)*row+1)
  mov ecx,1
  sub ecx,[two_a_square]
  imul ecx,[b_square]	  ;ecx=(1-two_a_square)*b_square
  add eax,ebx
  add eax,ecx
  mov [d],eax		  ;two_b_square*(col+1)*col+two_a_square*(row*(row-2)+1)+(1-two_a_square)*b_square

  mov edx,[x2]		     ;x
  mov ebp,[y2]		     ;y

  while_ellips_2:
  mov esi,[row]
  inc esi
  test esi,esi
  jz no_while_ellips_2

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  add eax,[col]      ;x+col
  add ebx,[row]      ;y+row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  add eax,[col]      ;x+col
  sub ebx,[row]      ;y-row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  sub eax,[col]      ;x-col
  add ebx,[row]      ;y+row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,edx	     ;x
  mov ebx,ebp	     ;y
  sub eax,[col]      ;x-col
  sub ebx,[row]      ;y-row
  mov [pixel_x],eax
  mov [pixel_y],ebx
  call setpixel

  mov eax,[d]
  cmp eax,0
  jns lab34

  inc [col]
  mov ebx,[col]
  imul ebx,[four_b_square]
  add [d],ebx		 ;d=d+four_b_square*col
  lab34:
  dec [row]
  mov ebx,[row]
  shl ebx,1
  mov eax,3
  sub eax,ebx
  imul eax,[two_a_square]
  add [d],eax		;d=d+two_b_square*(3-(row*2))

  jmp while_ellips_2
  no_while_ellips_2:

  mov eax,[output_array]
  sub eax,[e]
  shr eax,2

  ret
;---------------------------
  x_l dd ?
  y_l dd ?
  x_r dd ?
  y_r dd ?
  _dx dd ?
  _dy dd ?
  sx  dd ?
  sy  dd ?
  z   dd ?
  e   dd ?
  i   dd ?
  _ch dd ?
  x2  dd ?
  y2  dd ?
  d   dd ?
  r   dd ?
  m_x dd ?
  m_y dd ?
  c   dd ?
  c_f dd ?
  l_c dd ?
  l_s dd ?
;---------------------------
pointer_to_screen      dd 0
screen_size_x	       dd 0
output_array	       dd 0
pixel_x 	       dd 0
pixel_y 	       dd 0
;---------------------------
a		       dd 0
b		       dd 0
col		       dd 0
row		       dd 0
bnew		       dd 0
a_square	       dd 0
b_square	       dd 0
two_a_square	       dd 0
two_b_square	       dd 0
four_a_square	       dd 0
four_b_square	       dd 0
;-----------------------------