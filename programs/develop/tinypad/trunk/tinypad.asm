;-----------------------------------------------------------------------------
; project name:      TINYPAD
; compiler:          flat assembler 1.64
; memory to compile: 3.0/11.5 MBytes (without/with size optimizations)
; version:           4.0
; last update:       2006-04-13 (Apr 13, 2006)
;-----------------------------------------------------------------------------
; originally by:     Ville Michael Turjanmaa >> villemt@aton.co.jyu.fi
; maintained by:     Ivan Poddubny           >> ivan-yar@bk.ru
;                    Mike Semenyako          >> mike.dld@gmail.com
;-----------------------------------------------------------------------------
; TODO:
;   optimize drawing (reduce flickering);
;   optimize memory usage (allocate only needed amount, not static 3 Mbytes);
;   add block selection ability, undo action;
;   working with multiple files (add tabs);
;   other bugfixes and speed/size optimizations
;
; HISTORY:
; 4.0.2 (mike.dld)
;   bug-fixes:
;     - program terminates if started with parameters (fine for DOCPAK)
; 4.0.1 (mike.dld)
;   bug-fixes:
;     - unable to run program after exiting from main menu (Alt+X)
;   new features:
;     - integration with DOCPAK
; 4.0 (mike.dld)
;   bug-fixes:
;     - main menu popups' "on show" was called only for first shown popup
;     - clear selection on left/right arrow keys
;   new features:
;     - "replace" dialog (w/o "skip", "replace all")
;     - line numbers display
;     - options (except "appearance" and "smart tabulation")
;     - options saving (colors, window position, "Options" popup triggers)
; 4.0 beta 2 (mike.dld)
;   bug-fixes:
;     - unable to start if /rd/1/example.asm is missing (from Halyavin)
;     - clicking on menu items draws main window in popup (from Rohan)
;     - passed parameters aren't taken into account (from Mario79)
;     - background isn't erased if text lines < screen lines after
;       selection deletion (from Rohan)
; 4.0 beta 1 (mike.dld)
;   menu bar and popup menu;
;     removed buttons from the top and input fields from the bottom since
;     now they're accesible through main/popup menu;
;   improved keyboard handling (using 66th function);
;     support for almost all FASMW keyboard shourtcuts;
;   added text selection ability, standard selection operations
;     (copy,cut,paste);
;   new integrated dialogs (open, save, find)
;   fix to collapse SPACEs into TABs only for *.asm and *.inc files
; 3.78a (mike.dld)
;   fixed termination while typing in x positions higher than (line_length+10);
;   improved drawing on small heights
;     don't draw window while its height = 0 (Kolibri "minimize" support)
; 3.78 (mike.dld)
;   now lines may be of ANY length;
;     optimized memory usage (less memory for internal file representation)
;       after loading file, it's internal size equals to its real size
;       plus 14 bytes for each line (4 bytes for line length
;         and 10 spaced to the end - to reduce data relocations count);
;     completely rewritten keyboard handling;
;     added horizontal scrollbar;
;   all line feed formats are supported: WIN(CRLF),*NIX(LF),MAC(CR);
;   etc.
; 3.77 (mike.dld)
;   changed save_string to collapse SPACEs into TABs;
;   rewrote drawfile from scratch (speed++)
;     through some drawing improvements still needed
;     (some checkups to reduce flickering);
;   writepos (size--);
;   fixed drawing window while height < 100px, and for non-asm files;
;   several small fixes; speed/size optimizations
; 3.76 (mike.dld)
;   changed loadfile/loadhdfile to expand TABs into SPACEs;
;   changed TAB,ENTER,DELETE,BSPACE keys behaviour (rewritten from scratch);
;   vertical scrollbar;
;   extra window resizing capabilities (added a couple of constants);
;   completely new text cursor management & moving text cursor with mouse;
;   improved search function, moving cursor to beginning of text found;
;   adjustable max line width (change LINE_WIDTH & recompile) // (obsolet)
; 3.75a
;   fixed converting char to upper case in read_string
; 3.75
;   rewrote save_file from scratch; bugfix in loadfile;
; 3.74
;   optimisation
; 3.73
;   completly new load_file function
; 3.72
;   speed++
; 3.71
;   error beep
; 3.6,3.7:
;   many bugs fixed
;   simple toolbar
;   compile, run applications from TINYPAD, all fasm output is in debug board
;   TAB button
;   auto-indent
;   Ctrl+L - insert comment string
;-----------------------------------------------------------------------------
; Memory 0x300000:
;   stack for popup  0x00dff0 -
;   stack for help   0x00eff0 -
;   stack            0x00fff0 -
;   load position    0x010000 +
;   edit area        0x080000 +
;   copy/paste area  0x2f0000 +
;-----------------------------------------------------------------------------

include 'macros.inc' ; useful stuff
include 'tinypad.inc'
purge mov,add,sub            ;  SPEED

header '01',1,@CODE,TINYPAD_END,AREA_ENDMEM,MAIN_STACK,@PARAMS,self_path

;include 'debug.inc'

ASEPC     = '-'           ; separator character (char)
ATOPH     = POP_IHEIGHT+2 ; menu bar height (pixels)
;OLEFT    = 50+1          ; left offset (pixels)        !!! don't change !!!
SCRLW     = 16            ; scrollbar widht/height (pixels)
ATABW     = 8             ; tab width (chars)
LINEH     = 10            ; line height (pixels)
PATHL     = 255           ; maximum path length (chars) !!! don't change !!!
AMINS     = 8             ; minimal scroll thumb size (pixels)

STATH     = 14            ; status bar height

MEV_LDOWN = 1
MEV_LUP   = 2
MEV_RDOWN = 3
MEV_RUP   = 4
MEV_MOVE  = 5

;-----------------------------------------------------------------------------
section @OPTIONS ;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

label color_tbl dword
  RGB(  0,  0,  0) ; RGB(  0,  0,  0) ; RGB(  0,  0,  0) ; text
  RGB(  0,144,  0) ; RGB(  0,144,  0) ; RGB(  0,160,  0) ; numbers
  RGB(176,  0,  0) ; RGB(160,  0,  0) ; RGB(  0,128,255) ; strings
  RGB(128,128,128) ; RGB(144,144,144) ; RGB(160,160,160) ; comments
  RGB( 48, 48,240) ; RGB( 48, 48,240) ; RGB(255,  0,  0) ; symbols
  RGB(255,255,255) ; RGB(224,224,224) ; RGB(255,255,255) ; background
  RGB(255,255,255) ; RGB(255,255,255) ; RGB(255,255,255) ; selection text
  RGB( 10, 36,106) ; RGB(  0,  0,128) ; RGB(  0, 64,128) ; selection background

ins_mode db 1

options  db OPTS_AUTOINDENT+OPTS_OPTIMSAVE+OPTS_SMARTTAB

mainwnd_pos:
  .x dd 100
  .y dd 75
  .w dd 6*80+6+SCRLW+5
  .h dd 402

;-----------------------------------------------------------------------------
section @CODE ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

;       fninit

        cld
        mov     edi,@UDATA
        mov     ecx,@PARAMS-@UDATA
        mov     al,0
        rep     stosb

        mov     [left_ofs],40+1
        mov     [f_info+4],0
        mov     [f_info+12],AREA_TEMP
        mov     [f_info+16],AREA_EDIT-AREA_TEMP

;        mov     esi,s_example
;        mov     edi,s_fname
;        mov     ecx,s_example.size
;        mov     [s_fname.size],ecx
;        rep     movsb
        mov     esi,s_example
        mov     edi,tb_opensave.text
        mov     ecx,s_example.size
        mov     [tb_opensave.length],cl
        rep     movsb

        mov     esi,s_still
        mov     edi,s_search
        mov     ecx,s_still.size
        mov     [s_search.size],ecx
        rep     movsb

        cmp     byte[@PARAMS],0
        jz      no_params

;// Willow's code to support DOCPAK [

        cmp     byte[@PARAMS],'*'
        jne     .noipc
; convert size from decimal representation to dword
        mov     esi, @PARAMS+1
        xor     edx, edx
        xor     eax, eax
@@:
        lodsb
        test    al, al
        jz      @f
        lea     edx, [edx*4+edx]
        lea     edx, [edx*2+eax]
        jmp     @b
@@:
        add     edx,20
        mcall   60,1,AREA_TEMP-16 ; 0x10000-16
        mov     dword[AREA_TEMP-16+4],8 ; [0x10000-16+4],8
        mcall   40,1000000b
        mcall   23,200
        cmp     eax,7
        jne     key.alt_x.close  ; ¤Єю эр mcall -1 ьхЄър
        mov     esi,AREA_TEMP-16 ; 0x10000-16
        mov     byte[esi],1
        mov     eax,[esi+12]
        inc     eax
        call    load_file.file_found
        jmp     @f
;       call    file_found  ; чруЁєчър Їрщыр
;       jmp     do_load_file.restorecursor ; юЄюсЁрцхэшх
  .noipc:

;// Willow's code to support DOCPAK ]

    ; parameters are at @PARAMS
        mov     esi,@PARAMS
        mov     edi,tb_opensave.text
        mov     ecx,PATHL
        rep     movsb
        mov     edi,tb_opensave.text
        mov     ecx,PATHL
        xor     al,al
        repne   scasb
        jne     key.alt_x.close
        lea     eax,[edi-tb_opensave.text-1]
        mov     [tb_opensave.length],al

  no_params:
        call    btn.load_file;do_load_file
        jnc     @f
        call    new_file

    @@:
        call    drawwindow
        mcall   66,1,1
        mcall   40,00100111b

;-----------------------------------------------------------------------------

still:
       call    writepos ; write current position & number of strings

  .skip_write:
        mcall   10;23,50; wait here until event
        cmp     [main_closed],0
        jne     key.alt_x
        dec     eax     ; redraw ?
        jz      red
        dec     eax     ; key ?
        jz      key
        dec     eax     ; button ?
        jz      button
        sub     eax,3   ; mouse ?
        jz      mouse

        jmp     still.skip_write

;-----------------------------------------------------------------------------
func red ;///// window redraw ////////////////////////////////////////////////
;-----------------------------------------------------------------------------
        call    drawwindow
        call    check_inv_all.skip_check
        jmp     still
endf

;-----------------------------------------------------------------------------
func start_fasm ;/////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
; BL = run after compile
;-----------------------------------------------------------------------------
        cmp     [asm_mode],0
        jne     @f
        ret
    @@: mov     esi,f_info.path ; s_fname
        mov     edi,fasm_parameters

        cmp     byte[esi],'/'
        je      .yes_systree

        mov     ecx,[f_info.length] ; [s_fname.size]
        rep     movsb

        mov     al,','
        stosb

        mov     ecx,[f_info.length] ; [s_fname.size]
        add     ecx,-4
        mov     esi,f_info.path ; s_fname
        rep     movsb

        mov     al,','
        stosb

        mov     dword[edi],'/RD/'
        mov     word[edi+4],'1/'
        add     edi,6

        mov     al,0
        stosb

        jmp     .run

 .yes_systree:
        mov     eax,[f_info.length]
        add     esi,eax ; [s_fname.size]
        dec     esi

        xor     ecx,ecx
        mov     al,'/'
    @@: cmp     [esi],al
        je      @f
        dec     esi
        inc     ecx
        jmp     @b
    @@: inc     esi

        push    esi esi ecx

        rep     movsb

        mov     al,','
        stosb

        pop     ecx esi

        add     ecx,-4
        rep     movsb

        mov     al,','
        stosb

        pop     ecx
        sub     ecx,f_info.path ; s_fname
        mov     esi,f_info.path ; s_fname

        rep     movsb

        mov     al,0
        stosb

 .run:
        cmp     bl,0 ; run outfile ?
        je      @f
        mov     dword[edi-1],',run'
        mov     byte[edi+3],0
    @@: mcall   19,fasm_filename,fasm_parameters
        ret
endf

;-----------------------------------------------------------------------------
func open_debug_board ;///////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
        mcall   19,debug_filename,0
        ret
endf

;-----------------------------------------------------------------------------
func open_sysfuncs_txt ;//////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
        mcall   19,docpak_filename,sysfuncs_param
        cmp     eax,0xfffffff0
        jb      @f
        mcall   19,tinypad_filename,sysfuncs_filename
    @@: ret
endf

;-----------------------------------------------------------------------------
func layout  ;///// change keyboard layout ///////////////////////////////////
;-----------------------------------------------------------------------------
        mcall   19,setup,param_setup
        mcall   5,eax
;       call    activate_me
;       ret
;endf

;func activate_me
        mcall   9,p_info,-1
        inc     eax
        inc     eax
        mov     ecx,eax
        mov     edi,[p_info.PID]
        mov     ebx,p_info
    @@: dec     ecx
        jz      @f    ; counter=0 => not found? => return
        mcall   9
        cmp     edi,[p_info.PID]
        jne     @b
        mcall   18,3
        mcall   5,eax
    @@: ret
endf

func set_opt
        test    [options],al
        je      @f
        not     al
        and     [options],al
        ret
    @@: or      [options],al
        ret
endf

func set_line_numbers
        mov     al,OPTS_LINENUMS
        call    set_opt
        ret
endf

func set_optimal_fill
        mov     al,OPTS_OPTIMSAVE
        call    set_opt
        ret
endf

func set_auto_indents
        mov     al,OPTS_AUTOINDENT
        call    set_opt
        ret
endf

func set_auto_braces
        mov     al,OPTS_AUTOBRACES
        call    set_opt
        ret
endf

func set_secure_sel
        mov     al,OPTS_SECURESEL
        call    set_opt
        ret
endf

;-----------------------------------------------------------------------------

include 'tp-draw.asm'
include 'tp-key.asm'
;include 'tp-key2.asm'
include 'tp-butto.asm'
include 'tp-mouse.asm'
include 'tp-files.asm'
include 'tp-commo.asm'
include 'tp-dialo.asm'
;include 'tp-find.asm'
include 'tp-popup.asm'
include 'tp-tbox.asm'

;-----------------------------------------------------------------------------
section @DATA ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

;addr       dd s_fname  ; address of input string
;temp       dd 0xABCD   ; used in read_string
vscrl_capt dd -1
hscrl_capt dd -1
body_capt  dd -1

key0 db \
  0x00,0x00,0x02,0x03,0x04,0x05,0x06,0x07,\
  0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x00,0x00,\
  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,\
  0x18,0x19,0x1A,0x1B,0x00,0x00,0x1E,0x1F,\
  0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,\
  0x28,0x29,0x00,0x2B,0x2C,0x2D,0x2E,0x2F,\
  0x30,0x31,0x32,0x33,0x34,0x35,0x00,0x00,\
  0x00,0x39,0x00,0x00,0x00,0x00,0x00,0x00
  times 12*16 db 0x00

accel_table dd                      \
  0x0000000E,key.bkspace           ,\ ; BackSpace
  0x0000000F,key.tab               ,\ ; Tab
  0x0000001C,key.return            ,\ ; Return
  0x0000003D,key.f3                ,\ ; F3
  0x00000043,key.f9                ,\ ; F9
  0x00000147,key.home              ,\ ; Home
  0x00000148,key.up                ,\ ; Up
  0x00000149,key.pgup              ,\ ; PageUp
  0x0000014B,key.left              ,\ ; Left
  0x0000014D,key.right             ,\ ; Right
  0x0000014F,key.end               ,\ ; End
  0x00000150,key.down              ,\ ; Down
  0x00000151,key.pgdn              ,\ ; PageDown
  0x00000152,key.ins               ,\ ; Insert
  0x00000153,key.del               ,\ ; Delete
  0x00010147,key.shift_home        ,\ ; Shift+Home
  0x00010148,key.shift_up          ,\ ; Shift+Up
  0x00010149,key.shift_pgup        ,\ ; Shift+PageUp
  0x0001014B,key.shift_left        ,\ ; Shift+Left
  0x0001014D,key.shift_right       ,\ ; Shift+Right
  0x0001014F,key.shift_end         ,\ ; Shift+End
  0x00010150,key.shift_down        ,\ ; Shift+Down
  0x00010151,key.shift_pgdn        ,\ ; Shift+PageDown
  0x00010153,key.del               ,\ ; Shift+Delete
  0x00020015,key.ctrl_y            ,\ ; Ctrl+Y
  0x00020018,key.ctrl_o            ,\ ; Ctrl+O
  0x0002001E,key.ctrl_a            ,\ ; Ctrl+A
  0x0002001F,key.ctrl_s            ,\ ; Ctrl+S
  0x00020020,key.ctrl_d            ,\ ; Ctrl+D
  0x00020021,key.ctrl_f            ,\ ; Ctrl+F
  0x00020022,key.ctrl_g            ,\ ; Ctrl+G
  0x00020023,key.ctrl_h            ,\ ; Ctrl+H
\;0x00020026,key.ctrl_l            ,\ ; Ctrl+L
  0x0002002D,key.ctrl_x            ,\ ; Ctrl+X
  0x0002002E,key.ctrl_c            ,\ ; Ctrl+C
  0x0002002F,key.ctrl_v            ,\ ; Ctrl+V
  0x00020031,key.ctrl_n            ,\ ; Ctrl+N
  0x00020043,key.ctrl_f9           ,\ ; Ctrl+F9
  0x00020147,key.ctrl_home         ,\ ; Ctrl+Home
\;0x00020148,key.ctrl_up           ,\ ; Ctrl+Up
  0x00020149,key.ctrl_pgup         ,\ ; Ctrl+PageUp
  0x0002014B,key.ctrl_left         ,\ ; Ctrl+Left
  0x0002014D,key.ctrl_right        ,\ ; Ctrl+Right
  0x0002014F,key.ctrl_end          ,\ ; Ctrl+End
\;0x00020150,key.ctrl_down         ,\ ; Ctrl+Down
  0x00020151,key.ctrl_pgdn         ,\ ; Ctrl+PageDown
  0x00020153,key.del               ,\ ; Ctrl+Del
  0x0003001F,key.shift_ctrl_s      ,\ ; Shift+Ctrl+S
  0x00030147,key.shift_ctrl_home   ,\ ; Shift+Ctrl+Home
\;0x00030148,key.shift_ctrl_up     ,\ ; Shift+Ctrl+Up
  0x00030149,key.shift_ctrl_pgup   ,\ ; Shift+Ctrl+PageUp
  0x0003014B,key.shift_ctrl_left   ,\ ; Shift+Ctrl+Left
  0x0003014D,key.shift_ctrl_right  ,\ ; Shift+Ctrl+Right
  0x0003014F,key.shift_ctrl_end    ,\ ; Shift+Ctrl+End
\;0x00030150,key.shift_ctrl_down   ,\ ; Shift+Ctrl+Down
  0x00030151,key.shift_ctrl_pgdn   ,\ ; Shift+Ctrl+PageDown
  0x0004002D,key.alt_x             ,\ ; Alt+X
  0

accel_table_textbox dd              \
\;0x00000001,key.tb.escape         ,\ ; Escape
  0x0000000E,key.tb.bkspace        ,\ ; BackSpace
\;0x0000000F,key.tb.tab            ,\ ; Tab
\;0x0000001C,key.tb.return         ,\ ; Return
  0x00000147,key.tb.home           ,\ ; Home
  0x0000014B,key.tb.left           ,\ ; Left
  0x0000014D,key.tb.right          ,\ ; Right
  0x0000014F,key.tb.end            ,\ ; End
  0x00000153,key.tb.del            ,\ ; Delete
  0x00010147,key.tb.shift_home     ,\ ; Shift+Home
  0x0001014B,key.tb.shift_left     ,\ ; Shift+Left
  0x0001014D,key.tb.shift_right    ,\ ; Shift+Right
  0x0001014F,key.tb.shift_end      ,\ ; Shift+End
  0x00010153,key.tb.del            ,\ ; Shift+Del
  0

accel_table2 dd            \
  1,btn.close_main_window ,\
\;10000,btn.compile       ,\
\;10001,btn.compile_run   ,\
\;10002,btn.debug_board   ,\
\;10003,btn.sysfuncs_txt  ,\
  'UP',btn.scroll_up      ,\
  'DN',btn.scroll_down    ,\
  'LT',btn.scroll_left    ,\
  'RT',btn.scroll_right   ,\
\;5,key.ctrl_o            ,\
  0

accel_table2_botdlg dd     \
  1,btn.close_main_window ,\
  20001,btn.bot.cancel    ,\
  20002,btn.bot.opensave  ,\
  20003,btn.bot.find      ,\
  0

add_table:
; times $61 db -$00
  times $1A db -$20
  times $25 db -$00
  times $10 db -$20
  times $30 db -$00
  times $10 db -$50
  times $04 db -$00,-$01
  times $08 db -$00

;error_beep      db 0xA0,0x30,0

sz s_example,'EXAMPLE.ASM'
sz s_still  ,'still'

;sz param_setup,'LANG',0 ; parameter for SETUP

sz fasm_filename   ,'FASM       '
sz debug_filename  ,'BOARD      '
sz tinypad_filename,'TINYPAD    '
sz docpak_filename ,'DOCPAK     '
;sz setup           ,'SETUP      ' ; to change keyboard layout

sz sysfuncs_param,'g',0

lsz sysfuncs_filename,\
  ru,<'SYSFUNCR.TXT',0>,\
  en,<'SYSFUNCS.TXT',0>

sz htext,'TINYPAD'

lszc help_text,b,\
  ru,'КОМАНДЫ:',\
  ru,' ',\
  ru,'  CTRL+F1 : Это окно',\
  ru,'  CTRL+S  : Первая строка для копирования',\
  ru,'  CTRL+E  : Последняя строка для копирования',\
  ru,'  CTRL+P  : Вставить выбранное на текущую позицию',\
  ru,'  CTRL+D  : Удалить строку',\
  ru,'  CTRL+L  : Вставить строку-разделитель',\
  ru,'  CTRL+[  : Перейти в начало файла',\
  ru,'  CTRL+]  : Перейти в конец файла',\
  ru,'  CTRL+F2 : Загрузить файл',\
  ru,'  CTRL+F3 : Поиск',\
  ru,'  CTRL+F4 : Сохранить файл',\
  ru,'  CTRL+F5 : Ввести имя файла',\
  ru,'  CTRL+F6 : Ввести строку для поиска',\
  ru,'  CTRL+F8 : Сменить раскладку клавиатуры',\
\
  en,'COMMANDS:',\
  en,' ',\
  en,'  CTRL+F1 : SHOW THIS WINDOW',\
  en,'  CTRL+S  : SELECT FIRST STRING TO COPY',\
  en,'  CTRL+E  : SELECT LAST STRING TO COPY',\
  en,'  CTRL+P  : PASTE SELECTED TO CURRENT POSITION',\
  en,'  CTRL+D  : DELETE CURRENT LINE',\
  en,'  CTRL+L  : INSERT SEPARATOR LINE',\
  en,'  CTRL+[  : GO TO THE BEGINNING OF FILE',\
  en,'  CTRL+]  : GO TO THE END OF FILE',\
  en,'  CTRL+F2 : LOAD FILE',\
  en,'  CTRL+F3 : SEARCH',\
  en,'  CTRL+F4 : SAVE FILE',\
  en,'  CTRL+F5 : ENTER FILENAME',\
  en,'  CTRL+F6 : ENTER SEARCH STRING',\
  en,'  CTRL+F8 : CHANGE KEYBOARD LAYOUT'
db 0

menubar_res main_menu,\
  ru,'Файл'  ,popup_file   ,onshow.file   ,\
  ru,'Правка',popup_edit   ,onshow.edit   ,\
  ru,'Поиск' ,popup_search ,onshow.search ,\
  ru,'Запуск',popup_run    ,onshow.run    ,\
  ru,'Опции' ,popup_options,onshow.options,\
\
  en,'File'   ,popup_file   ,onshow.file  ,\
  en,'Edit'   ,popup_edit   ,onshow.edit  ,\
  en,'Search' ,popup_search ,onshow.search,\
  en,'Run'    ,popup_run    ,onshow.run   ,\
  en,'Options',popup_options,onshow.options

popup_res popup_file,\
  ru,'Новый'           ,'Ctrl+N',key.ctrl_n      ,\
  ru,'Открыть...'      ,'Ctrl+O',key.ctrl_o      ,\
  ru,'Сохранить'       ,'Ctrl+S',key.ctrl_s      ,\
  ru,'Сохранить как...',''      ,key.shift_ctrl_s,\
  ru,'-'               ,''      ,0               ,\
  ru,'Выход'           ,'Alt+X' ,key.alt_x       ,\
\
  en,'New'       ,'Ctrl+N',key.ctrl_n      ,\
  en,'Open...'   ,'Ctrl+O',key.ctrl_o      ,\
  en,'Save'      ,'Ctrl+S',key.ctrl_s      ,\
  en,'Save as...',''      ,key.shift_ctrl_s,\
  en,'-'         ,''      ,0               ,\
  en,'Exit'      ,'Alt+X' ,key.alt_x

popup_res popup_edit,\
  ru,'Вырезать'    ,'Ctrl+X',key.ctrl_x,\
  ru,'Копировать'  ,'Ctrl+C',key.ctrl_c,\
  ru,'Вставить'    ,'Ctrl+V',key.ctrl_v,\
  ru,'Удалить'     ,''      ,key.del   ,\
  ru,'-'           ,''      ,0         ,\
  ru,'Выделить всё','Ctrl+A',key.ctrl_a,\
\
  en,'Cut'       ,'Ctrl+X',key.ctrl_x,\
  en,'Copy'      ,'Ctrl+C',key.ctrl_c,\
  en,'Paste'     ,'Ctrl+V',key.ctrl_v,\
  en,'Delete'    ,''      ,key.del   ,\
  en,'-'         ,''      ,0         ,\
  en,'Select all','Ctrl+A',key.ctrl_a

popup_res popup_search,\
  ru,'Перейти...' ,'Ctrl+G',key.ctrl_g,\
  ru,'-'          ,''      ,0         ,\
  ru,'Найти...'   ,'Ctrl+F',key.ctrl_f,\
  ru,'Найти далее','F3'    ,key.f3    ,\
  ru,'Заменить...','Ctrl+H',key.ctrl_h,\
\
  en,'Position...','Ctrl+G',key.ctrl_g,\
  en,'-'          ,''      ,0         ,\
  en,'Find...'    ,'Ctrl+F',key.ctrl_f,\
  en,'Find next'  ,'F3'    ,key.f3    ,\
  en,'Replace...' ,'Ctrl+H',key.ctrl_h

popup_res popup_run,\
  ru,'Запустить'        ,'F9'     ,key.f9           ,\
  ru,'Компилировать'    ,'Ctrl+F9',key.ctrl_f9      ,\
  ru,'-'                ,''       ,0                ,\
  ru,'Доска отладки'    ,''       ,open_debug_board ,\
  ru,'Системные функции',''       ,open_sysfuncs_txt,\
\
  en,'Run'              ,'F9'     ,key.f9           ,\
  en,'Compile'          ,'Ctrl+F9',key.ctrl_f9      ,\
  en,'-'                ,''       ,0                ,\
  en,'Debug board'      ,''       ,open_debug_board ,\
  en,'System functions' ,''       ,open_sysfuncs_txt

popup_res popup_options,\
  ru,'Внешний вид...'        ,'',0,\
  ru,'-'                     ,'',0,\
  ru,'Безопасное выделение'  ,'',set_secure_sel,\
  ru,'Автоматические скобки' ,'',set_auto_braces,\
  ru,'Автоматический отступ' ,'',set_auto_indents,\
  ru,'Умная табуляция'       ,'',0,\
  ru,'Оптимальное сохранение','',set_optimal_fill,\
  ru,'-'                     ,'',0,\
  ru,'Номера строк'          ,'',set_line_numbers,\
\
  en,'Appearance...'         ,'',0,\
  en,'-'                     ,'',0,\
  en,'Secure selection'      ,'',set_secure_sel,\
  en,'Automatic brackets'    ,'',set_auto_braces,\
  en,'Automatic indents'     ,'',set_auto_indents,\
  en,'Smart tabulation'      ,'',0,\
  en,'Optimal fill on saving','',set_optimal_fill,\
  en,'-'                     ,'',0,\
  en,'Line numbers'          ,'',set_line_numbers

lsz s_modified,\
  ru,'Изменено',\
  en,'Modified'

lsz s_2filename,\
  ru,'Имя файла:',\
  en,'Filename:'
lsz s_2open,\
  ru,'Открыть',\
  en,'Open'
lsz s_2save,\
  ru,'Сохранить',\
  en,'Save'
lsz s_2find,\
  ru,'Найти',\
  en,'Find'
db ':'
lsz s_2replace,\
  ru,'Заменить',\
  en,'Replace'
db ':'
lsz s_2cancel,\
  ru,'Отмена',\
  en,'Cancel'

sz symbols_ex,';?.%"',"'"
sz symbols   ,'#&*\:/<>|{}()[]=+-, '

TINYPAD_END:     ; end of file

self_path rb PATHL

;-----------------------------------------------------------------------------
section @UDATA ;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

f_info.length dd ?
f_info dd ?,?,?,?,?;?,0,?,AREA_TEMP,AREA_EDIT-AREA_TEMP

f_info.path:
    times PATHL+1 db ?

pos.x         dd ?    ; global X position (cursor)
pos.y         dd ?    ; global Y position (cursor)
sel.x         dd ?    ; global X position (selection start)
sel.y         dd ?    ; global Y position (selection start)
lines         dd ?    ; number of lines in file
lines.scr     dd ?    ; number of lines on the screen
columns       dd ?    ; number of columns in file
columns.scr   dd ?    ; number of columns on the screen
top_ofs       dd ?    ; height occupied by top buttons
bot_ofs       dd ?    ; height occupied by bottom buttons
              dw ?
left_ofs      dd ?    ;
top_line      dd ?    ; topmost visible line on screen
left_col      dd ?    ; leftmost visible char on line
vscrl_top     dd ?
vscrl_size    dd ?
hscrl_top     dd ?
hscrl_size    dd ?
skinh         dd ?    ; skin height
__rc          dd ?,?,?,?
;filelen       dd ?    ; file size (on save) ???
filesize      dd ?    ; file size (on load) ???
ya            dd ?    ; for read_string
;copy_start    dd ?    ; first line for copying (Ctrl+S)
copy_count    dd ?    ; number of lines for copying (Ctrl+E)
copy_size     dd ?    ; size of data to copy
s_title.size  dd ?    ; caption length

draw_blines   dd ?    ; last line to draw

cur_line_len  dd ?
h_popup       dd ?
bot_dlg_handler dd ?

sel.begin.x   dd ?
sel.begin.y   dd ?
sel.end.x     dd ?
sel.end.y     dd ?
sel.selected  db ?

in_sel        db ?

asm_mode      db ?    ; ASM highlight?
do_not_draw   db ?    ; draw top and bottom buttons?
main_closed   db ?    ; main window closed?
tb_casesen    db ?    ; focused textbox is case-sensitive?

align 4
s_fname.size  dd ?
s_fname       rb PATHL+1
align 4
s_search.size dd ?
s_search      rb PATHL+1

s_title       rb PATHL+11  ; window caption

chr db ?
ext db ?
shi dd ?

align 4
cl_3d_normal dd ?
cl_3d_pushed dd ?
cl_3d_outset dd ?
cl_3d_inset  dd ?
cl_3d_grayed dd ?

tb_opensave  TBOX
tb_find      TBOX
tb_replace   TBOX
tb_gotorow   TBOX
tb_gotocol   TBOX

focused_tb   dd ?

key1 rb 256

mst  db ?
mst2 db ?
mev  db ?
mouse_captured  db ?
just_from_popup db ?

bot_mode db ?

modified db ?

align 4

bot_dlg_height dd ?
bot_dlg_mode2  db ?

;-----------------------------------------------------------------------------
section @PARAMS ;:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
;-----------------------------------------------------------------------------

fasm_parameters:

p_info  process_information
p_info2 process_information
sc      system_colors

diff16 'Main memory size',0,$

MAIN_STACK  = 0x0000FFF0
POPUP_STACK = 0x0000EFF0

AREA_TEMP   = 0x00010000 ; 0x00010000
AREA_EDIT   = 0x000C0000 ; 0x00080000
AREA_TEMP2  = 0x00190000 ; 0x002E0000
AREA_CBUF   = 0x001A0000 ; 0x002F0000
AREA_ENDMEM = 0x001B0000 ; 0x00300000

diff10 'Header+options size',0,@CODE
diff10 'Load area size',AREA_TEMP,AREA_EDIT
diff10 'Edit area size',AREA_EDIT,AREA_TEMP2
diff10 'Total memory usage',0,AREA_ENDMEM

;store dword '/hd/' at tb_opensave.text+4*0
;store dword '1/tp' at tb_opensave.text+4*1
;store dword 'ad4/' at tb_opensave.text+4*2
;store dword 'tiny' at tb_opensave.text+4*3
;store dword 'pad.' at tb_opensave.text+4*4
;store dword 'asm'  at tb_opensave.text+4*5
;store byte  23     at tb_opensave.length
