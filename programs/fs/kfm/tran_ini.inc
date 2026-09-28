;---------------------------------------------------------------------
proc normalize_extension_of_file
    push  edi
    mov   esi,[extension_start]
    mov   edi,extension_temp_storage
    cld
@@:
    lodsb
    call  char_todown
    stosb
    test  al,al
    jnz   @b
    pop   edi
    ret
endp
;---------------------------------------------------------------------
open_file_with_appl_1:
    mov   ebp,files_associations_1
    jmp   open_file_with_appl.1
;---------------------------------------------------------------------
open_file_with_appl:
; edx - start file name
; esi - start extension
    ;mov   ebp,files_associations     ;Leency
.1:
	
    ; mov   [extension_start],esi     ;Leency: do not get file assoc 
    ; call  search_star_and_end_tags  ;from KFM.INI, use /sys/@open
    ; cmp   ebp,-1
    ; je	  .end
    ; call  normalize_extension_of_file
; .search_association:
    ; cmp   [end_tag],edi
    ; jbe   .end
    ; mov   esi,extension_temp_storage
    ; mov   ecx,[extension_size]
    ; inc   edi
    ; cld
    ; push   edi
    ; rep  cmpsb
    ; pop    edi
    ; jne   .search_association
    ; push  esi
    ; mov   esi,edi
    ; add   esi,[extension_size] ;4
    ; cmp   [esi],byte ' '
    ; je	  @f
    ; pop   esi
    ; jmp   .search_association
; @@:
    ; add   esp,4
    ; inc   esi
    ; mov   edi,file_name  ;start_file_data.name
    ; xor   eax,eax
; @@:
    ; cld
    ; lodsb
    ; stosb
    ; cmp    al,' '
    ; cmp   al,13
    ; jnz   @b
    ; mov   [edi-1],byte 0 ;'#'
    ; cmp   [use_patch],0
    ; je	  @f
    ; mov   [use_patch],0

.run_sysopen:
@@:
    cmp   [fb_right.select_panel_counter],1
    je    @f
    mov   esi,read_folder_name	    ;read_folder.name
    jmp   .start
@@:
    mov   esi,read_folder_1_name    ;read_folder_1.name
.start:
    mov   edi,start_parameter
    cmp   [open_param],0
    je    .start_1
    mov   eax,[open_param]
    mov   eax,[eax]
    cld
    stosd
;    call draw_window
;    mov   eax,0
;    ret
.start_1:
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   [edi-1], byte '/'
    mov   esi,edx
    mov   [error_pointer],edx  ;start_file_data.name
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
.continue:
    mov   [start_file_data.param],start_parameter
    mcall SF_FILE, sysopen  ;Leency
    cmp   eax,0
    jge   @f
    mov  ebx,0
    sub  ebx,eax
    mov  eax,ebx
    call start_error_window_thread
@@:
    mov   eax,0
    ret
.end:
    mov   eax,1
    ret
;---------------------------------------------------------------------