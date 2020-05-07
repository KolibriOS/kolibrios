; ------------------------------------------------------------- ;
; KWINE is a fork of program PELoad written by 0CodErr
; author of fork - rgimad
; ------------------------------------------------------------- ;
ORG 0
BITS 32
; ------------------------------------------------------------- ;
PATH_SIZE      equ 4096
PARAMS_SIZE    equ 4096
STACK_SIZE     equ 4096
END_           equ IMAGE_BASE - (PATH_SIZE + PARAMS_SIZE + STACK_SIZE)
; ------------------------------------------------------------- ;
IMAGE_BASE     equ 400000H
; ------------------------------------------------------------- ;
MENUET01       db 'MENUET01'
version        dd 1
program.start  dd start_
program.end    dd END_
program.memory dd END_ + PATH_SIZE + PARAMS_SIZE + STACK_SIZE
program.stack  dd END_ + PATH_SIZE + PARAMS_SIZE + STACK_SIZE
program.params dd END_ + PATH_SIZE
program.path   dd END_
; ------------------------------------------------------------- ;
load.library:
        mov    eax, 68
        mov    ebx, 19
        mov    ecx, [esp + 4]
        int    64
        ret    4
; ------------------------------------------------------------- ;
getprocaddress:
        mov    edx, [esp + 8]
        xor    eax, eax
        test   edx, edx
        jz     .end
.next:
        xor    eax, eax
        cmp    [edx], dword 0
        jz     .end
        mov    esi, [edx]
        mov    edi, [esp + 4]
.next_:
        lodsb
        scasb
        jne    .fail
        or     al, al
        jnz    .next_
        jmp    .ok
.fail:
        add    edx, 8
        jmp    .next
.ok:
        mov    eax, [edx + 4]
.end:
        ret    8
; ------------------------------------------------------------- ;
realloc.app.mem:
        mov    eax, 64
        mov    ebx, 1
        mov    ecx, [esp + 4]
        int    64
        ret    4
; ------------------------------------------------------------- ;				
set.current.directory:
        mov    eax, 30
        mov    ebx, 1
        mov    ecx, [esp + 4]
        int    64
        ret    4			
; ------------------------------------------------------------- ;
%if 0 ; comment
int2str:
%define number [esp + 8 + 8 * 4]
%define buffer [esp + 4 + 8 * 4]
        pushad
		    mov    edi,  buffer
		    mov    eax, "    " ; 4 spaces
		    stosd
		    stosd
		    stosw
		    xor    al, al
		    stosb
				dec    edi
				dec    edi
				mov    ecx, 10
	      mov    eax, number
.next:
		    xor    edx, edx
		    div    ecx         ; ecx = 10
		    add    edx,  48    ; edx = (eax MOD ecx) + 48
		    mov    [edi], dl
		    dec    edi
		    test   eax, eax
		    jnz    .next
				popad
        ret    8
%undef number
%undef buffer
%endif ; endcomment
; ------------------------------------------------------------- ;
; test_file_path db "/hd3/1/mntest.exe",0
                        ; complex
                        ; address
                        ; data
                        ; hello
                        ; numbers  +-
                        ; proc
                        ; sptrim
                        ; se
                        ; clear
                        ; locals   +-
                        ; tokenise -
                        ; mntest
file_path      dd 0
lib_name       dd 0
lib            dd 0
func           dd 0
func_name      dd 0
; ------------------------------------------------------------- ;
sz_pe_load     db "KWINE",0
; ------------------------------------------------------------- ;
con_init            dd 0
con_write_asciiz    dd 0
con_exit            dd 0
console             dd 0
sz_con_init         db "con_init",0
sz_con_write_asciiz db "con_write_asciiz",0
sz_con_exit         db "con_exit",0
sz_console          db "/sys/lib/console.obj",0
; ------------------------------------------------------------- ;
MZ                   dw 0
PE                   dw 0
lfa_new              dd 0
NumberOfSections     dd 0
SizeOfOptionalHeader dd 0
EntryPoint           dd 0
SizeOfImage          dd 0
SizeOfHeaders        dd 0
DataDirectories      dd 0
SectionsTable        dd 0
Import               dd 0
; ------------------------------------------------------------- ;
ERROR_MESSAGE      dd 0
err_params         db "Parameters error",0
err_file_path      db "No input file path",0
err_read_file      db "Read file error",0
err_mz_not_found   db "No DOS signature found",0
err_pe_not_found   db "No PE signature found",0
err_load_library   db "Error load library: ",0
err_func_not_found db "Not found function: ",0
								 
; ------------------------------------------------------------- ;
%if 0 ; comment
msg_buffer resb 256
sz_new_line          db 10,0
sz_space             db " ",0
sz_space_colon_space db " : ",0
%endif ; endcomment
sz_empty             db "",0


start_:
; ------------------------------------------------------------- ;
; find params and file path
        ; mov    eax, test_file_path
        mov    eax, [program.params]
				cmp    [eax], byte 34 ; quote
				jne    .no_quote
        inc    eax
				mov    edi, eax
.find_quote_or_zero:
				cmp    [edi], byte 0
				je     .found
				cmp    [edi], byte 34 ; quote
        je     .found
        inc    edi	
        jmp    .find_quote_or_zero				
.no_quote:	
				mov    edi, eax
.find_space_or_zero:
				cmp    [edi], byte 0
				je     .found
				cmp    [edi], byte 32 ; space
        je     .found
        inc    edi	
        jmp    .find_space_or_zero				
.found:
				mov    [edi], byte 0
				mov    [file_path], eax
; check file path
        mov    eax, [file_path]
        mov    al, [eax]
				test   al, al
        jne     file_path_ok
				mov    [ERROR_MESSAGE], dword err_file_path
        jmp    ERROR
file_path_ok:
; check MZ signature (IMAGE_DOS_HEADER.e_magic)
        push   dword [file_path];filepath
        dec    esp
        mov    [esp], byte 0
        push   dword MZ;buffer
        push   dword 2;count
        push   dword 0
        push   dword 0;position
        push   dword 0
        mov    ebx, esp
        mov    eax, 70
        int    64
				test   eax, eax
        je     read_ok
				mov    [ERROR_MESSAGE], dword err_read_file
        jmp    ERROR
read_ok:
        cmp    word [MZ], "MZ"
        je     MZ_exists
				mov    [ERROR_MESSAGE], dword err_mz_not_found
        jmp    ERROR
MZ_exists:
; get lfa_new (IMAGE_DOS_HEADER.e_lfanew)
        push   dword [file_path];filepath
        dec    esp
        mov    [esp], byte 0
        push   dword lfa_new;buffer
        push   dword 4;count
        push   dword 0
        push   dword 60;position
        push   dword 0
        mov    ebx, esp
        mov    eax, 70
        int    64
; check PE signature (IMAGE_OPTIONAL_HEADER.Magic)
        push   dword [file_path];filepath
        dec    esp
        mov    [esp], byte 0
        push   dword PE;buffer
        push   dword 2;count
        push   dword 0
        push   dword [lfa_new];position
        push   dword 0
        mov    ebx, esp
        mov    eax, 70
        int    64
        cmp    word [PE], "PE"
        je     PE_exists
				mov    [ERROR_MESSAGE], dword err_pe_not_found
        jmp    ERROR
PE_exists:
; get size of headers (IMAGE_OPTIONAL_HEADER.SizeOfHeaders)
	      push   dword [file_path];filepath
        dec    esp
        mov    [esp], byte 0
        push   dword SizeOfHeaders;buffer
        push   dword 4;count
        push   dword 0
				mov    eax, [lfa_new]
				add    eax, 84
        push   eax;position
        push   dword 0
        mov    ebx, esp
        mov    eax, 70
        int    64
; resize	app memory and load headers
        mov    eax, IMAGE_BASE
        add    eax, [SizeOfHeaders]
        push   eax
				call   realloc.app.mem

	      push   dword [file_path];filepath
        dec    esp
        mov    [esp], byte 0
        push   dword IMAGE_BASE;buffer
        push   dword [SizeOfHeaders];count
        push   dword 0
        push   dword 0;position
        push   dword 0
        mov    ebx, esp
        mov    eax, 70
        int    64

        add    esp, (25 * 5) ; restore our stack top

				mov    edx, [lfa_new]
; get SizeOfImage (IMAGE_OPTIONAL_HEADER.SizeOfImage)
				mov    eax, [IMAGE_BASE + edx + 80]
				mov    [SizeOfImage], eax
; get EntryPoint (IMAGE_OPTIONAL_HEADER.AddressOfEntryPoint)
				mov    eax, [IMAGE_BASE + edx + 40]
				mov    [EntryPoint], eax
; get DataDirectories (IMAGE_OPTIONAL_HEADER.DataDirectory)
				lea    eax, [edx + 120]
				mov    [DataDirectories], eax
; get SizeOfOptionalHeader (IMAGE_FILE_HEADER.SizeOfOptionalHeader)
				movzx  eax, word [IMAGE_BASE + edx + 20]
				mov    [SizeOfOptionalHeader], ax
; get SectionsTable
        lea    eax, [edx + 24]
        add    ax, [SizeOfOptionalHeader]
				mov    [SectionsTable], eax
; get Import
				mov    eax, IMAGE_BASE
				add    eax, [DataDirectories]
				add    eax, 1 * 8
				mov    eax, [eax]
				mov    [Import], eax
; get NumberOfSections (IMAGE_FILE_HEADER.NumberOfSections)
				movzx  eax, word [IMAGE_BASE + edx + 6]
				mov    [NumberOfSections], eax
; resize	app memory and load sections to their virtual address
        mov    eax, IMAGE_BASE
        add    eax, [SizeOfImage]
        push   eax
				call   realloc.app.mem

				mov    ecx, [NumberOfSections]
next_section:
        lea    eax, [ecx - 1]
        lea    eax, [eax * 4 + eax]
				lea    eax, [eax * 8]
				add    eax, IMAGE_BASE
				add    eax, [SectionsTable]

	      push   dword [file_path] ; filepath
        dec    esp
        mov    [esp], byte 0
				mov    edx, [eax + 12]
				add    edx, IMAGE_BASE
        push   edx               ; buffer   (IMAGE_SECTION_HEADER.VirtualAddress)
        push   dword [eax + 16]  ; count    (IMAGE_SECTION_HEADER.SizeOfRawData)
        push   dword 0
        push   dword [eax + 20]  ; position (IMAGE_SECTION_HEADER.PointerToRawData)
        push   dword 0
        mov    ebx, esp
        mov    eax, 70
        int    64

        dec    ecx
				jnz     next_section

        mov    eax, [NumberOfSections]
				lea    eax, [eax * 4 + eax]
				lea    eax, [eax * 4 + eax]
				add    esp, eax ; restore our stack top




; ==========================================================
%if 0 ; comment
        push   sz_console
        call   load.library
        ; mov    [console], eax
        mov    ecx, eax
        mov    ebx, getprocaddress
				push   ecx
				push   sz_con_init
				call   ebx
				mov    [con_init], eax
				push   ecx
				push   sz_con_write_asciiz
				call   ebx
				mov    [con_write_asciiz], eax
				push   ecx
				push   sz_con_exit
				call   ebx
				mov    [con_exit], eax
        push   sz_pe_load
        push   -1
        push   -1
        push   -1
        push   -1
        call   [con_init]			

				mov    ecx, [NumberOfSections]
next_sect:
        lea    eax, [ecx - 1]
        lea    eax, [eax * 4 + eax]
				lea    eax, [eax * 8]
				add    eax, IMAGE_BASE
				add    eax, [SectionsTable]
				push   eax

        push   eax
        call   [con_write_asciiz]
        push   sz_space_colon_space
        call   [con_write_asciiz]
	

				pop    eax
				add    eax, 20
				mov    eax, [eax]
        push   eax
				push   msg_buffer
				call   int2str
        push   msg_buffer
        call   [con_write_asciiz]

        push   sz_new_line
        call   [con_write_asciiz]


        dec    ecx
				jnz     next_sect
%endif ; endcomment				
; ==============================================
; program.path = program.path_without_filename & "lib/"
				mov    edi, [program.path]
				xor    al, al
				xor    ecx, ecx
				dec    ecx
				repne scasb
				std
				mov    al, "/"
				repne scasb
				cld
				inc    edi
				inc    edi
				mov    eax, "lib/"
				stosd
;				
				mov    [lib_name], edi

				xor    ecx, ecx
next_descriptor:
        lea    eax, [ecx * 4 + ecx]
				lea    eax, [eax * 4]				
				add    eax, IMAGE_BASE
				add    eax, [Import]
        mov    edx, [eax + 12]
				add    edx, IMAGE_BASE
				
%if 0 ; comment					
pushad							
        push   edx 
        call   [con_write_asciiz]
        push   sz_new_line
        call   [con_write_asciiz]			
popad
%endif ; endcomment	

pushad
; concatenate (program.path_without_filename & "lib/") & lib_name
        mov    esi, edx
				mov    edi, [lib_name]
.copy_lib_name:				
				lodsb
				stosb
				test   al, al
				jnz    .copy_lib_name
; try to load library			
			  push   dword [program.path]
			  call   load.library
				test   eax, eax
				jnz    .lib_loaded
; concatenate "Error load library: " & 	lib_name			
				sub    edi, [lib_name]
				mov    esi, edi
				mov    edi, err_load_library				
				xor    al, al
				xor    ecx, ecx
				dec    ecx
				repne scasb
				dec    edi
				mov    ecx, esi
				mov    esi, edx
				rep movsb
;							
popad
				mov    [ERROR_MESSAGE], dword err_load_library
        jmp    ERROR				
.lib_loaded:
        mov    [lib], eax			
popad
				xor    ebx, ebx
next_function:				
				mov    edx, [eax + 16]
				lea    esi, [edx + ebx * 4 + IMAGE_BASE]
				mov    [func], esi	
				mov    edx, [esi]
				test   edx, edx
				jz     .done
				inc    edx
				inc    edx
				add    edx, IMAGE_BASE		
				
%if 0 ; comment									
pushad			
        push   edx 
        call   [con_write_asciiz]
        push   sz_new_line
        call   [con_write_asciiz]
popad
%endif ; endcomment	

pushad
        mov    [func_name], edx
; look for address of imported function								
				push   dword [lib]
				push   edx
				call   getprocaddress
				test   eax, eax
				jnz    .func_found
popad				
; concatenate "Not found function: " & 	name of function			
				mov    edi, err_func_not_found				
				xor    al, al
				xor    ecx, ecx
				dec    ecx
				repne scasb
				dec    edi
        mov    esi, edx
.copy_func_name:				
				lodsb
				stosb
				test   al, al
				jnz    .copy_func_name
;										
        dec    edi
        mov    eax, " in "
				stosd

				mov    esi, [lib_name]
.copy_lib_name:				
				lodsb
				stosb
				test   al, al
				jnz    .copy_lib_name				

				mov    [ERROR_MESSAGE], dword err_func_not_found
        jmp    ERROR				
.func_found:						
				mov    edx, [func]
        mov    [edx], eax
popad								
				inc    ebx
				jmp    next_function
.done:				
				
        inc    ecx
        lea    eax, [ecx * 4 + ecx]
				lea    eax, [eax * 4]				
				add    eax, IMAGE_BASE
				add    eax, [Import]					
        mov    eax, [eax + 12]			
				cmp    eax, dword 0
				jnz    next_descriptor				
								
				
; set.current.directory				
				mov    edi, [file_path]
				xor    al, al
				xor    ecx, ecx
				dec    ecx
				repne scasb
				std
				mov    al, "/"
				repne scasb
				cld
				mov    [edi + 1], byte 0
				push   dword [file_path]
				call   set.current.directory
				mov    [edi + 1], byte "/" ; restore full file_path
				
				; ---------------------- ; 
				; call   load_console_lib  ;
				; ---------------------- ; 
				
; go to EntryPoint
        mov    eax, [EntryPoint]	
        add    eax, IMAGE_BASE
        jmp    eax
				
				
        ; push   dword [EntryPoint]
				; push   msg_buffer
				; call   int2str
        ; push   msg_buffer
        ; call   [con_write_asciiz]
				
%if 0 ; comment	
        push   0
        call   [con_exit]
%endif ; endcomment	

%if 0 ; comment	
; dump ---------------------------------------				
	      push   dword dump_path;filepath
        dec    esp
        mov    [esp], byte 0
        push   dword 0 ;buffer
        mov    eax, IMAGE_BASE
        add    eax, [SizeOfImage]						
        push   eax;count
        push   dword 0
        push   dword 0;position
        push   dword 2
        mov    ebx, esp
        mov    eax, 70
        int    64				
				
				
				xor    eax, eax
				dec    eax
				int    64
				
				dump_path db "/hd3/1/dump.bin",0
%endif ; endcomment					
; ==========================================================


ERROR:

        push   sz_console
        call   load.library
        ; mov    [console], eax
        mov    ecx, eax
        mov    ebx, getprocaddress
				push   ecx
				push   sz_con_init
				call   ebx
				mov    [con_init], eax
				push   ecx
				push   sz_con_write_asciiz
				call   ebx
				mov    [con_write_asciiz], eax
				push   ecx
				push   sz_con_exit
				call   ebx
				mov    [con_exit], eax

        push   sz_pe_load
        push   -1
        push   -1
        push   -1
        push   -1
        call   [con_init]
        push   dword [ERROR_MESSAGE]
        call   [con_write_asciiz]
        push   0
        call   [con_exit]

				xor    eax, eax
				dec    eax
				int    64



; load_console_lib:
        ; push   sz_console
        ; call   load.library

        ; push   eax
        ; push   sz_con_init
        ; call   getprocaddress
        ; mov    [con_init], eax
				
        ; push   sz_empty
        ; push   -1
        ; push   -1
        ; push   -1
        ; push   -1
        ; call   [con_init]

        ; ret
