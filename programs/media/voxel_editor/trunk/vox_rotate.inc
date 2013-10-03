;
; Функции для изменения воксельной таблицы.
; В воксельной таблице содержатся 8 цифр,
; которые задают направление для расположения
; дочерних вокселей. Изменяя цифры в этой таблице
; можно получить повороты и зеркальные отображения
; воксельного изображения.
;
; Схема нумерации углов в таблице:
;     4
; 5       6
; |   7   |
; |   0   |
; 1       2
;     3
; где:
; 0-1 ось x
; 0-2 ось y
; 0-4 ось z

; поворот вокруг оси z
align 4
proc vox_obj_rot_z uses eax ebx ecx, v_obj:dword
	mov ebx,[v_obj]
	add ebx,vox_offs_tree_table

    ;(1)
    call vox_tbl_copy

    ;(2)
    ;0 -> 1   4 -> 5
    ;^    |   ^    |
    ;2 <- 3   6 <- 7

    mov edx,dword[eax]
    mov byte[eax+1],dl ;0->1
    mov byte[eax+3],dh ;1->3
    shr edx,16
    mov byte[eax],dl   ;2->0
    mov byte[eax+2],dh ;3->2

    mov edx,dword[eax+4]
    mov byte[eax+5],dl ;4->5
    mov byte[eax+7],dh ;5->7
    shr edx,16
    mov byte[eax+4],dl ;6->4
    mov byte[eax+6],dh ;7->6
 
    ;(3)
    call vox_tbl_paste

	ret
endp

; поворот вокруг оси y
align 4
proc vox_obj_rot_y uses eax ebx ecx, v_obj:dword
	mov ebx,[v_obj]
	add ebx,vox_offs_tree_table

    ;(1)
    call vox_tbl_copy

    ;(2)
    ;0 -> 4   1 -> 5
    ;^    |   ^    |
    ;2 <- 6   3 <- 7

	mov edx,dword[eax]
	mov ecx,dword[eax+4]

	mov byte[eax+4],dl ;0->4
	mov byte[eax+5],dh ;1->5
	shr edx,16
	mov byte[eax+0],dl ;2->0
	mov byte[eax+1],dh ;3->1

	mov byte[eax+6],cl ;4->6
	mov byte[eax+7],ch ;5->7
	shr ecx,16
	mov byte[eax+2],cl ;6->2
	mov byte[eax+3],ch ;7->3
 
    ;(3)
    call vox_tbl_paste
	ret
endp

; поворот вокруг оси x
align 4
proc vox_obj_rot_x uses eax ebx ecx, v_obj:dword
	mov ebx,[v_obj]
	add ebx,vox_offs_tree_table

    ;(1)
    call vox_tbl_copy

    ;(2)
    ;0 -> 4   2 -> 6
    ;^    |   ^    |
    ;1 <- 5   3 <- 7

	mov edx,dword[eax]
	mov ecx,dword[eax+4]

	mov byte[eax+4],dl ;0->4
	mov byte[eax+0],dh ;1->0
	shr edx,16
	mov byte[eax+6],dl ;2->6
	mov byte[eax+2],dh ;3->2

	mov byte[eax+5],cl ;4->5
	mov byte[eax+1],ch ;5->1
	shr ecx,16
	mov byte[eax+7],cl ;6->7
	mov byte[eax+3],ch ;7->3
 
    ;(3)
    call vox_tbl_paste
	ret
endp


;input:
; ebx = vox_offs_tree_table
align 4
vox_tbl_copy:
    ;(1)
    mov eax,txt_buf

    movzx ecx,byte[ebx]
    add ecx,eax
    mov byte[ecx],0

    movzx ecx,byte[ebx+1]
    add ecx,eax
    mov byte[ecx],1

    movzx ecx,byte[ebx+2]
    add ecx,eax
    mov byte[ecx],2

    movzx ecx,byte[ebx+3]
    add ecx,eax
    mov byte[ecx],3

    movzx ecx,byte[ebx+4]
    add ecx,eax
    mov byte[ecx],4

    movzx ecx,byte[ebx+5]
    add ecx,eax
    mov byte[ecx],5

    movzx ecx,byte[ebx+6]
    add ecx,eax
    mov byte[ecx],6

    movzx ecx,byte[ebx+7]
    add ecx,eax
    mov byte[ecx],7
    ret

;input:
; ebx = vox_offs_tree_table
align 4
vox_tbl_paste:
    movzx ecx,byte[eax]
    add ecx,ebx
    mov byte[ecx],0

    movzx ecx,byte[eax+1]
    add ecx,ebx
    mov byte[ecx],1

    movzx ecx,byte[eax+2]
    add ecx,ebx
    mov byte[ecx],2

    movzx ecx,byte[eax+3]
    add ecx,ebx
    mov byte[ecx],3

    movzx ecx,byte[eax+4]
    add ecx,ebx
    mov byte[ecx],4

    movzx ecx,byte[eax+5]
    add ecx,ebx
    mov byte[ecx],5

    movzx ecx,byte[eax+6]
    add ecx,ebx
    mov byte[ecx],6

    movzx ecx,byte[eax+7]
    add ecx,ebx
    mov byte[ecx],7
    ret
