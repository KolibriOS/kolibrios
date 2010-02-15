//BOX_LIB
//Lrz - 2008

dword edit_box_draw = #aEdit_box_draw;
dword edit_box_key  = #aEdit_box_key;
dword edit_box_mouse = #aEdit_box_mouse;


dword  am__ = 0x0;
dword  bm__ = 0x0;

char aEdit_box_draw[9]  = "edit_box\0";
char aEdit_box_key[13] = "edit_box_key\0";
char aEdit_box_mouse[15] = "edit_box_mouse\0";



struct edit_box{
dword width, left, top, color, shift_color, focus_border_color, blur_border_color,
text_color, max, text, mouse_variable, flags, size, pos, offset, cl_curs_x, cl_curs_y, shift, shift_old;
};

//ed_width        equ [edi]               ;ширина компонента
//ed_left         equ [edi+4]             ;положение по оси х
//ed_top          equ [edi+8]             ;положение по оси у
//ed_color        equ [edi+12]            ;цвет фона компонента
//shift_color     equ [edi+16]            ;=0x6a9480
//ed_focus_border_color   equ [edi+20]    ;цвет рамки компонента
//ed_blur_border_color    equ [edi+24]    ;цвет не активного компонента
//ed_text_color   equ [edi+28]            ;цвет текста
//ed_max          equ [edi+32]            ;кол-во символов которые можно максимально ввести
//ed_text         equ [edi+36]            ;указатель на буфер
//ed_flags        equ [edi+40]            ;флаги
//ed_size equ [edi+42]                    ;кол-во символов
//ed_pos  equ [edi+46]                    ;позиция курсора
//ed_offset       equ [edi+50]            ;смещение
//cl_curs_x       equ [edi+54]            ;предыдущее координата курсора по х
//cl_curs_y       equ [edi+58]            ;предыдущее координата курсора по у
//ed_shift_pos    equ [edi+62]            ;положение курсора
//ed_shift_pos_old equ [edi+66]           ;старое положение курсора



int fastcall load_editbox_lib(EAX)
{
//set mask 
        $mov    eax,40
        $mov    ebx,0x27
        $int    0x40
// load DLL
        $mov     eax, 68
        $mov     ebx, 19
        ECX="/sys/lib/box_lib.obj";
        $int     0x40
        $test    eax, eax
        $jz      exit

// initialize import
        $mov     edx,eax
        ESI=#edit_box_draw;
import_loop:
        $lodsd
        $test    eax,eax
        $jz      import_done
        $push    edx
import_find:
        $mov     ebx,DSDWORD[EDX]
        $test    ebx, ebx
        $jz      exit
        $push    eax
nex1:
        $mov     cl,DSBYTE[EAX];
        $cmp     cl,DSBYTE[EBX];
        $jnz     import_find_next
        $test    cl,cl
        $jz      import_found
        $inc     eax
        $inc     ebx
        $jmp     nex1
import_find_next:
        $pop     eax
        $add     edx, 8
        $jmp     import_find
import_found:
        $pop     eax
        $mov     eax,DSDWORD[edx+4]
        $mov     DSDWORD[esi-4],eax
        $pop     edx
        $jmp     import_loop
import_done:
        return 0;
exit:
        return -1;
}
