;---------------------------------------------------------------------
get_icon_number:
    mov   ebp,[extension_size]	;[icons_end_tag]
    mov   edx,[icons_end_tag]
    mov   edi,[ini_file_start]
    dec   edi
.search_association:
    cmp   edx,edi
    jbe   .end
    mov   esi,[extension_start]
    mov   ecx,ebp
    test  ecx,ecx
    je	  @f
    sub   ecx,2
@@:
    inc   edi
    mov   ebx,edi
    cld
    rep   cmpsb
    mov   edi,ebx
    jne   .search_association
    mov   esi,edi
    add   esi,ebp
    test  ebp,ebp
    je	  @f
    sub   esi,2
@@:
    cmp   [esi],byte ' '
    jne   .search_association
    inc   esi
    xor   ebx,ebx
    call  .calculate
    imul  eax,1000
    call  .calculate_1
    imul  eax,100
    call  .calculate_1
    imul  eax,10
    call  .calculate_1
    add   ebx,eax
    ret
.end:
    mov  ebx,2
    ret
;---------------------------------------------------------------------
.calculate_1:
    add   ebx,eax
.calculate:
    xor   eax,eax
    cld
    lodsb
    sub   eax,0x30
    ret
;---------------------------------------------------------------------
open_file_with_appl_1:
    mov   ebp,files_associations_1
    jmp   open_file_with_appl.1
;---------------------------------------------------------------------
open_file_with_appl:
; edx - start file name
; esi - start extension
    mov   ebp,files_associations
.1:
    mov   [extension_start],esi
    call  search_star_and_end_tags
    cmp   ebp,-1
    je	  .end
.search_association:
    cmp   [end_tag],edi
    jbe   .end
    mov   esi,[extension_start] ;extension_temp_area
    mov   ecx,[extension_size]
    inc   edi
    cld
    push   edi
    rep  cmpsb
    pop    edi
    jne   .search_association
    push  esi
    mov   esi,edi
    add   esi,[extension_size] ;4
    cmp   [esi],byte ' '
    je	  @f
    pop   esi
    jmp   .search_association
@@:
    add   esp,4
    inc   esi
    mov   edi,file_name  ;start_file_data.name
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    cmp    al,' '
    cmp   al,13
    jnz   @b
    mov   [edi-1],byte 0 ;'#'
    cmp   [use_patch],0
    je	  @f
    mov   [use_patch],0
    jmp   .continue
@@:
    cmp   [select_panel_counter],0
    jne   @f
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
    call  start_appl
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
search_star_and_end_tags:
    mov   edi,[ini_file_start]
    dec   edi
.search_tag1:
    cmp   [left_folder_data],edi
    jbe  .end
    mov   esi,ebp
    mov   ecx,22
    inc   edi
    cld
    push   edi
    rep  cmpsb
    pop    edi
    jne     .search_tag1
    dec    edi
    push   edi
.search_end_tag:
    cmp   [left_folder_data],edi
    jbe  .end
    mov   esi,end_section_tag
    mov   ecx,7
    inc   edi
    cld
    push   edi
    rep  cmpsb
    pop    edi
    jne     .search_end_tag
    mov   [end_tag],edi
    pop   edi
    ret
.end:
    mov   ebp,-1
    ret
;---------------------------------------------------------------------