
;FigureInit, (Object.FigData)

;add_object_in_list, (врем. стр.), (Object.FigData)
;       mov [Object.FigData], (tl_node_add(врем. стр.),tl_node_get_data)

;draw_fig2d_litl, ___, pobj, (Object.FigData), fign, ___, ___, ___

struct Object
        OType dd ?
        FigCount dd ?
        FigData dd ? ;array pointers to Figures
        MinX dq ?
        MinY dq ?
        SizeX dq ?
        SizeY dq ?
        WScale dq ? ;prewiew window scale
        MScale dq ? ;mouse scale (show partition figure)
        MCentrX dd ? ;mouse centr X
        MCentrY dd ? ;mouse centr Y
        Caption rb 32
ends

struct Figure
        OType dd ?
        PoiCount dd ?
        PoiData dd ?
        MinX dq ?
        MinY dq ?
        SizeX dq ?
        SizeY dq ?
        WScale dq ? ;prewiew window scale (show all figure)
        MScale dq ? ;mouse scale (show partition figure)
        MCentrX dd ? ;mouse centr X
        MCentrY dd ? ;mouse centr Y
        Caption rb 32
ends

struct Point
        Prop dd ? ;prorerties
        CoordX dq ? ;coord X
        CoordY dq ? ;coord Y
ends

txt_err_1 db 'Ошибка при считывании объекта',0
txt_err_2 db 'Ошибка при считывании фигуры',0
txt_err_3 db 'Не найдена точка в фигуре',0
txt_err_no_figure_select db 'Не выбрана команда для перемещения',0
txt_err_figure_is_0 db 'Команда не может быть сдвинута вверх,',13,10,'она в начале списка.',0
txt_err_figure_is_last db 'Команда не может быть сдвинута вниз,',13,10,'она в конце списка.',0
txt_err_no_point_del db 'Не можна удалить все точки из фигуры.',0
txt_err_poi_is_0 db 'Не можна переместить точку вверх,',13,10,'она в начале списка.',0
txt_err_poi_is_last db 'Не можна переместить точку вниз,',13,10,'она в конце списка.',0
txt_err_no_1_point_sel db 'Выберите не больше одной точки.',0

txt_min_x db 'min x: ',0
txt_min_y db 'min y: ',0

if lang eq ru
;
; rus
;
txt_preview db 'Предпросмотр результата:',0
txt_port db 'Порт:',0
txt_but_cancel db 'Отмена',0
txt_but_run db 'Выполнить',0
txt_size_x db 'размер x: ',0
txt_size_y db 'размер y: ',0
txt_command:
  db 'Команда:', 0
  .end:
txt_filename:
  db 'Файл (полный путь):', 0
  .end:
else
;
; eng
;
txt_preview db 'Preview:',0
txt_port db 'Port:',0
txt_but_cancel db 'Cancel',0
txt_but_run db 'Run',0
txt_size_x db 'size x: ',0
txt_size_y db 'size y: ',0
txt_command:
  db 'Command:', 0
  .end:
txt_filename:
  db 'Full file name (path):', 0
  .end:
end if

align 4
txt_s_poi db 'X',0
txt_s_poi_Y db ' Y',0
txt_s_poi_Z db ' Z',0
txt_n db 13,0

align 4
zoom_plus dq 1.25992105
mouse_prop_x dd 0 ;курсор мыши относительно центра окна buf_0 (для измен. масштаба)
mouse_prop_y dd 0
mouse_down_x dd 0 ;курсор мыши относительно левого верхнего угла окна buf_0 (для сдвига фигуры)
mouse_down_y dd 0
opt_draw  dd 1
graf_margin_left   dd 3 ;margin in pixels
graf_margin_right  dd 3+6 ;margin in pixels
graf_margin_top    dd 3+9 ;margin in pixels
graf_margin_bot    dd 3+9 ;margin in pixels
data_printed dd ? ;число выполненных команд
data_all     dd ? ;число всех команд

;global variables:
ObjData Object
FigData Figure ;временная структура для заполнения tree1
PoiData Point

size_one_list equ sizeof.Figure ;max (sizeof.Figure, sizeof.Object)

if sizeof.Object > sizeof.Figure
   ... need fix size_one_list
end if

if (Object.FigCount <> Figure.PoiCount) | (Object.FigData <> Figure.PoiData) | (Object.MScale <> Figure.MScale) | (Object.MCentrX <> Figure.MCentrX) | (Object.MCentrY <> Figure.MCentrY)
   ... need fix offsets
end if
        
txt_buf rb 80 ;???
txt_sp db ' ',0

;input:
; esi - text pointer
align 4
proc txt_next_line uses eax ecx edi, mlen:dword
        mov al,13
        mov ecx,[mlen]
        mov edi,esi
        repne scasb
        cmp byte[edi],10
        jne @f
                inc edi
        @@:
        mov esi,edi
        ret
endp

;input:
; edi - destination buffer
; esi - source text string: '...\nl
; mlen - maximum len destination buffer
align 4
proc txt_load_line uses eax ebx ecx esi, mlen:dword
        mov byte[edi],0
        mov al,13
        cmp byte[esi],al
        je .end_f
                mov ecx,[mlen]
                mov ebx,edi
                mov edi,esi
                repne scasb ;found end of string
                mov ecx,edi
                sub ecx,esi
                dec ecx ;ecx = string len
                mov edi,ebx
                rep movsb ;copy caption string
                cmp byte[esi],13
                je @f
                .cycle0:
                        inc esi
                        cmp byte[esi],13
                        jne .cycle0
                @@:
                mov byte[edi],0
                inc esi
        .end_f:
        ret
endp

;input:
; itxt - input description text
; msize - max size
align 4
proc FileInit uses eax ebx ecx edi esi, itxt:dword, msize:dword
        cmp dword[ObjData.FigData],0
        je @f
        cmp dword[ObjData.FigCount],0 ;or Figure.PoiCount
        je @f
                stdcall mem.Free,[ObjData.FigData] ;or Figure.PoiData
        @@:
        mov dword[offs_last_timer],0

        mov esi,[itxt]
        mov ebx,esi
        add ebx,[msize]
        stdcall ObjectInit,ObjData,ebx
        ret
endp

;input:
; esi - input description text
; pobj - pointer to object struct
; etxt - pointer to end of input text
align 4
proc ObjectInit uses eax ebx ecx edi, pobj:dword, etxt:dword
                mov ebx,[pobj]
                mov [ebx+Object.OType],'Obj'
                mov edi,ebx
                add edi,Object.Caption
                mov dword[edi],'CNC'

                stdcall get_max_lines
                test eax,eax
                jz .err_init
                mov [ebx+Object.FigCount],eax
                mov ecx,eax

                shl eax,2
                stdcall mem.Alloc,eax
                mov [ebx+Object.FigData],eax
                push ecx
                        mov edi,eax
                        xor eax,eax
                        mov [data_all],eax ;хитрое начальное обнуление счетчика
                        mov [data_printed],eax
                        rep stosd ;clear memory
                pop ecx

                mov edi,[ebx+Object.FigData]
align 4
                .cycle0:
                        stdcall FigureInit,edi
                        mov eax,[edi]
                        mov eax,[eax+Figure.PoiCount]
                        or eax,eax
                        jnz @f
                        	inc eax ;минимум 1-на команда
                        @@:
                        add [data_all],eax
                        cmp esi,[etxt]
                        jge .cycle0end
                        add edi,4
                loop .cycle0
                .cycle0end:
                inc dword[ebx+Object.FigCount]
                or ecx,ecx
                jz @f
                        ;уменьшаем объем памяти выделенный для команд
                        sub [ebx+Object.FigCount],ecx
                        mov eax,[ebx+Object.FigCount]
                        shl eax,2
                        stdcall mem.ReAlloc,[ebx+Object.FigData],eax
                        mov [ebx+Object.FigData],eax
                @@:
                ;установка счетчика и прогресбара на 25 %
                mov eax,[data_all]
                mov [pb.max],eax
                shr eax,2
                mov [data_printed],eax
                mov [pb.value],eax

                stdcall ObjCalculateScale,ebx

                jmp .end_f
        .err_init:
                notify_window_run txt_err_1
        .end_f:
        ret
endp

align 4
proc ObjCalculateScale uses eax ebx ecx edi, pobj:dword
        mov ebx,[pobj]

        ;*** Calculate scale for object ***
        finit

        ;found min coord X
        mov ecx,[ebx+Object.FigCount]
        jecxz .cycle2end
        mov edi,[ebx+Object.FigData]
        @@:
                mov eax,[edi]
                dec ecx
                jecxz .cycle2end
                add edi,4
                or eax,eax
                jz @b
                cmp [eax+Figure.PoiCount],0
                je @b
        push edi esi ;copy first min X
                lea esi,[eax+Figure.MinX]
                lea edi,[ebx+Object.MinX]
                movsd
                movsd
        pop esi edi
align 4
        .cycle2:
                mov eax,[edi]
                or eax,eax
                jz @f
                cmp [eax+Figure.PoiCount],0
                je @f
                fld qword[eax+Figure.MinX]
                fcomp qword[ebx+Object.MinX]
                fstsw ax
                sahf
                jae @f
                        push edi esi
                        mov eax,[edi]
                        lea esi,[eax+Figure.MinX]
                        lea edi,[ebx+Object.MinX]
                        movsd
                        movsd
                        pop esi edi
                @@:
                add edi,4
        loop .cycle2
        .cycle2end:

        ;found min coord Y
        mov ecx,[ebx+Object.FigCount]
        jecxz .cycle3end
        mov edi,[ebx+Object.FigData]
        @@:
                mov eax,[edi]
                dec ecx
                jecxz .cycle3end
                add edi,4
                or eax,eax
                jz @b
                cmp [eax+Figure.PoiCount],0
                je @b
        push edi esi ;copy first min Y
                lea esi,[eax+Figure.MinY]
                lea edi,[ebx+Object.MinY]
                movsd
                movsd
        pop esi edi
align 4
        .cycle3:
                mov eax,[edi]
                or eax,eax
                jz @f
                cmp [eax+Figure.PoiCount],0
                je @f
                fld qword[eax+Figure.MinY]
                fcomp qword[ebx+Object.MinY]
                fstsw ax
                sahf
                jae @f
                        push edi esi
                        mov eax,[edi]
                        lea esi,[eax+Figure.MinY]
                        lea edi,[ebx+Object.MinY]
                        movsd
                        movsd
                        pop esi edi
                @@:
                add edi,4
        loop .cycle3
        .cycle3end:

        ;found max coord X
        mov ecx,[ebx+Object.FigCount]
        jecxz .cycle4end
        mov edi,[ebx+Object.FigData]
        ;copy first min X
        mov eax,[edi]
        fld qword[eax+Figure.MinX]
        fadd qword[eax+Figure.SizeX]
        fstp qword[ebx+Object.SizeX]
        dec ecx
        jecxz .cycle4end
align 4
        .cycle4:
                add edi,4
                mov eax,[edi]
                or eax,eax
                jz @f
                cmp [eax+Figure.PoiCount],0
                je @f
                fld qword[eax+Figure.MinX]
                fadd qword[eax+Figure.SizeX]
                fcomp qword[ebx+Object.SizeX]
                fstsw ax
                sahf
                jbe @f
                        mov eax,[edi]
                        fld qword[eax+Figure.MinX]
                        fadd qword[eax+Figure.SizeX]
                        fstp qword[ebx+Object.SizeX]
                @@:
        loop .cycle4
        .cycle4end:

        ;found max coord Y
        mov ecx,[ebx+Object.FigCount]
        jecxz .cycle5end
        mov edi,[ebx+Object.FigData]
        ;copy first min Y
        mov eax,[edi]
        fld qword[eax+Figure.MinY]
        fadd qword[eax+Figure.SizeY]
        fstp qword[ebx+Object.SizeY]
        dec ecx
        jecxz .cycle5end
align 4
        .cycle5:
                add edi,4
                mov eax,[edi]
                or eax,eax
                jz @f
                cmp [eax+Figure.PoiCount],0
                je @f
                fld qword[eax+Figure.MinY]
                fadd qword[eax+Figure.SizeY]
                fcomp qword[ebx+Object.SizeY]
                fstsw ax
                sahf
                jbe @f
                        mov eax,[edi]
                        fld qword[eax+Figure.MinY]
                        fadd qword[eax+Figure.SizeY]
                        fstp qword[ebx+Object.SizeY]
                @@:
        loop .cycle5
        .cycle5end:

        ;found size X
        fld qword[ebx+Object.SizeX]
        fsub qword[ebx+Object.MinX]
        fstp qword[ebx+Object.SizeX]
        ;found size Y
        fld qword[ebx+Object.SizeY]
        fsub qword[ebx+Object.MinY]
        fstp qword[ebx+Object.SizeY]

        ;*** Calculate scale ***
                fld1
                fstp qword[ebx+Object.WScale] ;set default scale

                fld qword[ebx+Object.SizeX]
                ftst
                fstsw ax
                sahf
                jne .els_0 ;if (SizeX == 0.0)
                        ffree st0
                        fincstp
                        jmp .els_1
                .els_0: ;else if (SizeX != 0.0)
                        fild dword[buf_0.w]
                        fisub dword[graf_margin_left]
                        fisub dword[graf_margin_right]
                        fxch st1
                        fdivp
                        fstp qword[ebx+Object.WScale]
                .els_1:

                fld qword[ebx+Object.SizeY]
                ftst
                fstsw ax
                sahf
                jne .els_2 ;if (SizeY == 0.0)
                        ffree st0
                        fincstp
                        jmp .els_3
                .els_2: ;else if (SizeY != 0.0)
                        fild dword[buf_0.h]
                        fisub dword[graf_margin_top]
                        fisub dword[graf_margin_bot]
                        fxch st1
                        fdivp
                        fcom qword[ebx+Object.WScale]
                        fstsw ax
                        sahf
                        jbe @f
                                ffree st0
                                fincstp
                                jmp .els_3
                        @@:
                        fstp qword[ebx+Object.WScale]
                .els_3:

                fld1
                fstp qword[ebx+Figure.MScale]
                mov dword[ebx+Figure.MCentrX],0
                mov dword[ebx+Figure.MCentrY],0
        ret
endp

;input:
; esi - input description text
; pfig - pointer to figure struct
;output:
; esi - output description text
align 4
proc FigureInit uses eax ebx ecx edx edi, pfig:dword
        mov ebx,FigData ; temp struct
        mov [ebx+Figure.OType],'Fig'
        mov edi,ebx
        add edi,Figure.Caption
        stdcall txt_load_line,32
                
        xor eax,eax
        mov [ebx+Figure.PoiCount],eax
        mov [ebx+Figure.PoiData],eax
        cmp byte[esi],'X'
        jne .end0
                stdcall get_max_points
                mov [ebx+Figure.PoiCount],eax
                or eax,eax
                jz .end0
                mov ecx,eax
                imul eax,sizeof.Point
                stdcall mem.Alloc,eax
                mov [ebx+Figure.PoiData],eax
                or eax,eax
                jz .err_init
                mov edi,eax
align 4
                .cycle0:
                        stdcall PointInit,edi
                        or eax,eax
                        jz .cycle0end
                        add edi,sizeof.Point
                        loop .cycle0
                .cycle0end:
                or ecx,ecx
                jz .end1
                        ;уменьшаем объем памяти выделенный для точек
                        sub [ebx+Figure.PoiCount],ecx
                        mov eax,[ebx+Figure.PoiCount]
                        imul eax,sizeof.Point
                        stdcall mem.ReAlloc,[ebx+Figure.PoiData],eax
                        mov [ebx+Figure.PoiData],eax
                jmp .end1
        .err_init:
                notify_window_run txt_err_2
        .end0:
                ;command
                stdcall txt_next_line, 80
                cmp byte[esi],13
                jne @f
                        ;if null line
                        inc esi
                        cmp byte[esi],10
                        jne .end2
                                inc esi
                        .end2:
                        mov dword[edi],(10 shl 8)+13 ;new line
                @@:
                stdcall add_object_in_list,ebx,[pfig]
                jmp .end_f
        .end1:
                ;coords
                stdcall add_object_in_list,ebx,[pfig]
                or eax,eax
                jz .end_f
                stdcall FigCalculateSizes,eax,1
        .end_f:
        ret
endp

;input:
; esi - pointer to file
;output:
; eax - max lines in file
align 4
proc get_max_lines uses esi
        xor eax,eax
        .cycle0:
                cmp byte[esi],13
                jne @f
                        inc eax
                @@:
                inc esi
                cmp byte[esi],0
                jne .cycle0
        ret
endp

;input:
; esi - pointer to file
;output:
; eax - max point coords in file
align 4
proc get_max_points uses esi
        xor eax,eax
        .cycle0:
                cmp word[esi],' Y'
                jne @f
                        inc eax
                @@:
                inc esi
                cmp byte[esi+1],0
                jne .cycle0
        ret
endp

;input:
; opt_calc - если 0 - пересчитать масштаб фигуры,
;                 1 - пересчитать размеры и масштаб фигуры
align 4
proc FigCalculateSizes uses eax ebx ecx edi esi, pfig:dword, opt_calc:dword
        mov ebx,[pfig]
        or ebx,ebx
        jz .end_f

        finit
        bt dword[opt_calc],0
        jnc .calc_scale

        ;*** Calculate sizes ***
        mov esi,[ebx+Figure.PoiData]
        or esi,esi
        jz .end_f

        lea esi,[esi+Point.CoordX]
        lea edi,[ebx+Figure.MinX]
        movsd
        movsd ;Figure.MinX = Point[0].CoordX
        sub esi,8
        lea edi,[ebx+Figure.SizeX]
        movsd
        movsd ;Figure.SizeX = Point[0].CoordX
        mov esi,[ebx+Figure.PoiData]
        lea esi,[esi+Point.CoordY]
        lea edi,[ebx+Figure.MinY]
        movsd
        movsd ;Figure.MinY = Point[0].CoordY
        sub esi,8
        lea edi,[ebx+Figure.SizeY]
        movsd
        movsd ;Figure.SizeY = Point[0].CoordY

        ;found min coord X
        mov ecx,[ebx+Figure.PoiCount]
        jecxz .cycle2end
        mov edi,[ebx+Figure.PoiData]
align 4
        .cycle2:
                fld qword[edi+Point.CoordX]
                fcomp qword[ebx+Figure.MinX]
                fstsw ax
                sahf
                ja @f
                        push edi esi
                        lea esi,[edi+Point.CoordX]
                        lea edi,[ebx+Figure.MinX]
                        movsd
                        movsd
                        pop esi edi
                @@:
                add edi,sizeof.Point
        loop .cycle2
        .cycle2end:

        ;found min coord Y
        mov ecx,[ebx+Figure.PoiCount]
        jecxz .cycle3end
        mov edi,[ebx+Figure.PoiData]
align 4
        .cycle3:
                fld qword[edi+Point.CoordY]
                fcomp qword[ebx+Figure.MinY]
                fstsw ax
                sahf
                ja @f
                        push edi esi
                        lea esi,[edi+Point.CoordY]
                        lea edi,[ebx+Figure.MinY]
                        movsd
                        movsd
                        pop esi edi
                @@:
                add edi,sizeof.Point
        loop .cycle3
        .cycle3end:

        ;found max coord X
        mov ecx,[ebx+Figure.PoiCount]
        jecxz .cycle4end
        mov edi,[ebx+Figure.PoiData]
align 4
        .cycle4:
                fld qword[edi+Point.CoordX]
                fcomp qword[ebx+Figure.SizeX]
                fstsw ax
                sahf
                jbe @f
                        push edi esi
                        lea esi,[edi+Point.CoordX]
                        lea edi,[ebx+Figure.SizeX]
                        movsd
                        movsd
                        pop esi edi
                @@:
                add edi,sizeof.Point
        loop .cycle4
        .cycle4end:

        ;found max coord Y
        mov ecx,[ebx+Figure.PoiCount]
        jecxz .cycle5end
        mov edi,[ebx+Figure.PoiData]
align 4
        .cycle5:
                fld qword[edi+Point.CoordY]
                fcomp qword[ebx+Figure.SizeY]
                fstsw ax
                sahf
                jbe @f
                        push edi esi
                        lea esi,[edi+Point.CoordY]
                        lea edi,[ebx+Figure.SizeY]
                        movsd
                        movsd
                        pop esi edi
                @@:
                add edi,sizeof.Point
        loop .cycle5
        .cycle5end:

        ;found size X
        fld qword[ebx+Figure.SizeX]
        fsub qword[ebx+Figure.MinX]
        fstp qword[ebx+Figure.SizeX]
        ;found size Y
        fld qword[ebx+Figure.SizeY]
        fsub qword[ebx+Figure.MinY]
        fstp qword[ebx+Figure.SizeY]
                
        ;*** Calculate scale ***
align 4
        .calc_scale:
        mov dword[ebx+Figure.MCentrX],0
        mov dword[ebx+Figure.MCentrY],0
        fld1
        fst qword[ebx+Figure.MScale] ;???
        fstp qword[ebx+Figure.WScale] ;set default scale

        fld qword[ebx+Figure.SizeX]
        ftst
        fstsw ax
        sahf
        jne .els_0 ;if (SizeX == 0.0)
                ffree st0
                fincstp
                jmp .els_1
        .els_0: ;else if (SizeX != 0.0)
                fild dword[buf_0.w]
                fisub dword[graf_margin_left]
                fisub dword[graf_margin_right]
                fxch st1
                fdivp
                fstp qword[ebx+Figure.WScale]
        .els_1:

        fld qword[ebx+Figure.SizeY]
        ftst
        fstsw ax
        sahf
        jne .els_2 ;if (SizeY == 0.0)
                ffree st0
                fincstp
                jmp .els_3
        .els_2: ;else if (SizeY != 0.0)
                fild dword[buf_0.h]
                fisub dword[graf_margin_top]
                fisub dword[graf_margin_bot]
                fxch st1
                fdivp
                fcom qword[ebx+Figure.WScale]
                fstsw ax
                sahf
                jbe @f
                        ffree st0
                        fincstp
                        jmp .els_3
                @@:
                fstp qword[ebx+Figure.WScale]
        .els_3:

        .end_f:
        ret
endp

;input:
; esi - input description text
; ppoi - pointer to point struct
;output:
; eax - 0 (if error init) or 1
; esi - output description text
align 4
proc PointInit uses ebx ecx edi, ppoi:dword
        mov     ecx,64 ;защита от зацикливания
        @@:
                lodsb
                cmp     al,' '
                jne     @f
                loop @b
        @@:
        dec esi
        cmp byte[esi],'X'
        jne .err_init
                inc esi
                stdcall conv_str_to_int, esi
                mov ebx,[ppoi]

                ;Data_String <- esi
                push esi
                        mov ecx,32
                        mov edi,esi
                        @@:
                                lodsb
                                or al,al
                                jz @f
                                cmp al,' '
                                je @f
                                cmp al,13
                                je @f
                                loop @b
                        @@:
                        mov esi,edi
                        sub ecx,32
                        neg ecx
                        mov edi,Data_String
                        rep movsb
                        mov byte[edi],0
                pop esi
                stdcall String_to_DoubleFloat
                ;Data_Double -> Point.CoordX
                push esi
                        mov esi,Data_Double
                        lea edi,[ebx+Point.CoordX]
                        movsd
                        movsd
                pop esi

                push edi
                mov al,'Y'
                mov ecx,80
                mov edi,esi
                repne scasb
                mov esi,edi
                pop edi
                ;Data_String <- esi
                push esi
                        mov ecx,32
                        mov edi,esi
                        @@:
                                lodsb
                                or al,al
                                jz @f
                                cmp al,' '
                                je @f
                                cmp al,13
                                je @f
                                loop @b
                        @@:
                        mov esi,edi
                        sub ecx,32
                        neg ecx
                        mov edi,Data_String
                        rep movsb
                        mov byte[edi],0
                pop esi
                stdcall String_to_DoubleFloat
                ;Data_Double -> Point.CoordY
                push esi
                        mov esi,Data_Double
                        lea edi,[ebx+Point.CoordY]
                        movsd
                        movsd
                pop esi

                stdcall txt_next_line, 80
                jmp @f
        .err_init:
                xor eax,eax
                jmp .end_f
        @@:
                xor eax,eax
                inc eax
        .end_f:
        ret
endp

;input:
; buffer - добавляемые данные
; pfig - указатель на добавленную фигуру (для контроля со стороны родительского объекта)
;output:
; eax - object data pointer
align 4
proc add_object_in_list uses ebx ecx edi esi, buffer:dword, pfig:dword
        mov ebx,[pfig]
        or ebx,ebx
        jz @f
                stdcall mem.Alloc,sizeof.Figure
                mov [ebx],eax
                mov ecx,sizeof.Figure
                mov edi,eax
                mov esi,[buffer]
                rep movsb
        @@:
        ret
endp

;description:
; функция для рисования выбранного объекта
align 4
proc draw_obj2d, pobj:dword
locals
        data_draw dd ?
        CentrX dd ?
        CentrY dd ?
endl
pushad
        mov ebx,[pobj]
        stdcall [buf2d_clear], buf_0, [buf_0.color]
        mov ecx,[ebx+Object.FigCount]
        or ecx,ecx
        jz .end_f
        ;;jecxz .end_f
                mov edi,[ebx+Object.FigData]
                finit
                fld qword[ebx+Object.SizeY]
                fmul qword[ebx+Object.WScale]
                fmul qword[ebx+Object.MScale]
                fchs
                fistp dword[CentrY]
                mov eax,[CentrY]
                sub eax,[graf_margin_top]
                sub eax,[graf_margin_bot]
                add eax,[buf_0.h]
                sar eax,1
                add eax,[graf_margin_bot] ;_bot а не _top - в связи с тем что коорд. Y перевернута
                add eax,[ebx+Object.MCentrY]
                mov [CentrY],eax
                fld qword[ebx+Object.SizeX]
                fmul qword[ebx+Object.WScale]
                fmul qword[ebx+Object.MScale]
                fchs
                fistp dword[CentrX]
                mov edx,[CentrX]
                sub edx,[graf_margin_left]
                sub edx,[graf_margin_right]
                add edx,[buf_0.w]
                sar edx,1
                add edx,[graf_margin_left]
                add edx,[ebx+Object.MCentrX]
                mov [CentrX],edx
                mov dword[data_draw],0
align 4
                .cycle0:
                        stdcall draw_fig2d_litl, buf_0,ebx,[edi],[data_draw],0,edx,eax
                        mov esi,[edi]
                        mov esi,[esi+Figure.PoiCount]
                        or esi,esi
                        jnz @f
                        inc esi
                        @@:
                        add [data_draw],esi
                        add edi,4
                        loop .cycle0
                stdcall [buf2d_flip_v], buf_0

        bt dword[opt_draw],0
        jnc .end_f
                ; draw min & sizes
                mov word[NumberSymbolsAD],8
                stdcall mem_copy, txt_buf,txt_size_x,20
                fld qword[ebx+Object.SizeX]
                fstp qword[Data_Double]
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall str_cat,txt_buf,txt_n
                stdcall str_cat,txt_buf,txt_size_y
                fld qword[ebx+Object.SizeY]
                fstp qword[Data_Double]
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall [buf2d_draw_text], buf_0, buf_1,txt_buf,3,23,0

                fld qword[ebx+Object.MinX]
                fstp qword[Data_Double]
                stdcall mem_copy, txt_buf,txt_min_x,20
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall str_cat,txt_buf,txt_n
                stdcall str_cat,txt_buf,txt_min_y
                fld qword[ebx+Object.MinY]
                fstp qword[Data_Double]
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall [buf2d_draw_text], buf_0, buf_1,txt_buf,3,3,0
        .end_f:
popad
        ret
endp

;input:
; pbuf - pointer to buffer 2d
; pobj - указатель на объект
; pfig - указатель на фигуру
; fign - номер фигуры
;description:
; функция для рисования объекта по частям
align 4
proc draw_fig2d_litl, pbuf:dword, pobj:dword, pfig:dword, fign:dword,\
	Scale:dword, CentrX:dword, CentrY:dword
locals
        line_col dd ? ;figure color
		PervX dd 0 ;pervios point coord X
        PervY dd 0 ;pervios point coord Y
        GrafX dd ? ;active point coord X
        GrafY dd ? ;active point coord Y
endl
pushad
        mov ebx,[pfig]
        test ebx,ebx
        jz .end_f
        mov ecx,[ebx+Figure.PoiCount]
        ;;jecxz .end_f
        test ecx,ecx
        jz .end_f
        mov edi,[ebx+Figure.PoiData]
        mov edx,[pobj]

        fld1
        mov eax,[Scale]
        or eax,eax
        jnz @f
                lea eax,[edx+Object.MScale]
                fmul qword[eax]
                lea eax,[edx+Object.WScale]
        @@:
        fmul qword[eax]
        mov esi,[fign]
align 4
        .cycle0:
                fld qword[edi+Point.CoordX]
                fsub qword[edx+Object.MinX]
                fmul st0,st1
                fiadd dword[CentrX]
                fistp dword[ebp-8]
                fld qword[edi+Point.CoordY]
                fsub qword[edx+Object.MinY]
                fmul st0,st1
                fiadd dword[CentrY]
                fistp dword[ebp-4]
                stdcall [buf2d_set_pixel], [pbuf], [GrafX],[GrafY], 0xff0000
                cmp ecx,[ebx+Figure.PoiCount]
                je .end0
                        ;определение цвета
                        cmp esi,[data_printed]
                        jl @f
                        mov [line_col],0x80 ;if not printed
                        jmp .end1
                        @@:
                        mov [line_col],0xff0000 ;if was printed
                        .end1:
                        stdcall [buf2d_line_sm], [pbuf], [PervX],[PervY], [GrafX],[GrafY], [line_col]
                        stdcall [buf2d_line], [pbuf], [PervX],[PervY], [GrafX],[GrafY], [line_col]
                .end0:
                mov eax,[GrafX]
                mov [PervX],eax
                mov eax,[GrafY]
                mov [PervY],eax
                add edi,sizeof.Point
                inc esi
        ;;loop .cycle0
                dec ecx
        jnz .cycle0
        ffree st0
        fincstp

        .end_f:
popad
        ret
endp

;description:
; функция для рисования выбранной фигуры
align 4
proc draw_fig2d, pfig:dword
locals
        n dd 0
        col_lin dd 0 ;цвет линий фигуры
        CentrX dd ?
        CentrY dd ?
        PervX dd 0 ;pervios point coord X
        PervY dd 0 ;pervios point coord Y
        GrafX dd ? ;active point coord X
        GrafY dd ? ;active point coord Y
        old_tl dd 0 ;old text coord X0
        old_tt dd 0 ;old text coord Y0
        old_tw dd 0 ;old text coord X1
        old_th dd 0 ;old text coord Y1
        new_tl dd 0 ;new text coord X0
        ;new_tt dd 0 ;new text coord Y0
        new_tw dd 0 ;new text coord X1
        ;new_th dd 0 ;new text coord Y1
endl
pushad
        stdcall [buf2d_clear], buf_0, [buf_0.color]
        mov ebx,[pfig]
        test ebx,ebx
        jz .end_f
        mov ecx,[ebx+Figure.PoiCount]
        ;;jecxz .end_f
        test ecx,ecx
        jz .end_f
        mov edi,[ebx+Figure.PoiData]
        finit
        fld qword[ebx+Figure.SizeY]
        fmul qword[ebx+Figure.WScale]
        fmul qword[ebx+Figure.MScale]
        fchs
        fistp dword[CentrY]
        mov eax,[CentrY]
        sub eax,[graf_margin_top]
        sub eax,[graf_margin_bot]
        add eax,[buf_0.h]
        sar eax,1
        add eax,[graf_margin_bot]
        add eax,[ebx+Figure.MCentrY]
        mov [CentrY],eax
        ;CentrY = (-(Figure.SizeY * Figure.WScale) - graf_margin_top - graf_margin_bot + buf_0.h) / 2 + graf_margin_bot
        fld qword[ebx+Figure.SizeX]
        fmul qword[ebx+Figure.WScale]
        fmul qword[ebx+Figure.MScale]
        fchs
        fistp dword[CentrX]
        mov eax,[CentrX]
        sub eax,[graf_margin_left]
        sub eax,[graf_margin_right]
        add eax,[buf_0.w]
        sar eax,1
        add eax,[graf_margin_left]
        add eax,[ebx+Figure.MCentrX]
        mov [CentrX],eax        
        ;CentrX = (-(Figure.SizeX * Figure.WScale) - graf_margin_left - graf_margin_right + buf_0.w) / 2 + graf_margin_left

align 4
        .cycle0:
                fld qword[edi+Point.CoordX]
                fsub qword[ebx+Figure.MinX]
                fmul qword[ebx+Figure.WScale]
                fmul qword[ebx+Figure.MScale]
                fiadd dword[CentrX]
                fistp dword[GrafX] 
                ;GrafX = (Point.CoordX - Figure.MinX) * Figure.WScale + CentrX
                fld qword[edi+Point.CoordY]
                fsub qword[ebx+Figure.MinY]
                fmul qword[ebx+Figure.WScale]
                fmul qword[ebx+Figure.MScale]
                fiadd dword[CentrY]
                fisub dword[buf_0.h] ;invert coord Y
                fchs
                fistp dword[GrafY]
                ;GrafY = -((Point.CoordY - Figure.MinY) * Figure.WScale + CentrY - buf_0.h)

                stdcall [buf2d_set_pixel], buf_0, [GrafX],[GrafY], 0xff0000
                mov dword[col_lin],0x80
                cmp ecx,[ebx+Figure.PoiCount]
                je @f
                        stdcall [buf2d_line_sm], buf_0, [PervX],[PervY], [GrafX],[GrafY], [col_lin]
                        stdcall [buf2d_line], buf_0, [PervX],[PervY], [GrafX],[GrafY], [col_lin]
                @@:
                mov eax,[GrafX]
                mov [PervX],eax
                mov edx,[GrafY]
                mov [PervY],edx
                add edi,sizeof.Point

                bt dword[opt_draw],0
                jnc .end0
                ; draw number points
                ;mov eax,[GrafX]
                ;mov edx,[GrafY]
                add eax,2
                add edx,2
                push eax edi
                        mov eax,[n]
                        mov edi,txt_buf
                        stdcall convert_int_to_str,20
                pop edi eax

                inc dword[n]
                ;calculate new text coords
                mov [new_tl],eax
                mov esi,eax
                stdcall str_len,txt_buf
                imul eax,[buf_1.w]
                add eax,esi
                mov [new_tw],eax
                ;rect compare coords
                cmp [old_tl],eax
                jg .no_comp
                cmp [old_tw],esi
                jl .no_comp
                        ;calculate new text coord Y1
                        mov esi,edx
                        add esi,[font_h]
                cmp [old_tt],esi
                jg .no_comp
                cmp [old_th],edx
                ;jl .no_comp
                        ;jmp .end0
                jge .end0
                .no_comp:
                        mov eax,[new_tl]
                        mov [old_tl],eax
                        mov eax,[new_tw]
                        mov [old_tw],eax
                        mov [old_tt],edx
                        add edx,[font_h]
                        mov [old_th],edx
                        stdcall [buf2d_draw_text], buf_0, buf_1,txt_buf,[old_tl],[old_tt],0x000000
                .end0:
                dec ecx
        jnz .cycle0

        bt dword[opt_draw],0
        jnc .end_f
                ; draw min & sizes
                mov word[NumberSymbolsAD],8
                stdcall mem_copy, txt_buf,txt_size_x,20
                fld qword[ebx+Figure.SizeX]
                fstp qword[Data_Double]
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall str_cat,txt_buf,txt_n
                stdcall str_cat,txt_buf,txt_size_y
                fld qword[ebx+Figure.SizeY]
                fstp qword[Data_Double]
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall [buf2d_draw_text], buf_0, buf_1,txt_buf,3,23,0

                fld qword[ebx+Figure.MinX]
                fstp qword[Data_Double]
                stdcall mem_copy, txt_buf,txt_min_x,20
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall str_cat,txt_buf,txt_n
                stdcall str_cat,txt_buf,txt_min_y
                fld qword[ebx+Figure.MinY]
                fstp qword[Data_Double]
                call DoubleFloat_to_String
                call String_crop_0
                stdcall str_cat,txt_buf,Data_String
                stdcall [buf2d_draw_text], buf_0, buf_1,txt_buf,3,3,0
        .end_f:
popad
        ret
endp

align 4
proc mouse_left_d uses eax ebx
        call buf_get_mouse_coord
        cmp eax,-1
        je .end_f
                mov [mouse_down_x],eax
                mov [mouse_down_y],ebx
        .end_f:
        ret
endp

align 4
proc mouse_left_u uses eax ebx ecx
        call buf_get_mouse_coord
        cmp eax,-1
        je .end_f
                sub eax,[mouse_down_x]
                sub ebx,[mouse_down_y]
                mov ecx,ObjData
                add [ecx+Figure.MCentrX],eax
                sub [ecx+Figure.MCentrY],ebx
                mov dword[offs_last_timer],0
        .end_f:
        ret
endp
