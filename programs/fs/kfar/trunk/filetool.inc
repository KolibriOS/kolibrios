delete_file_worker:
; in: ecx=flags: 1=deleting directory
; out: eax=0 - OK, eax=1 - retry, eax=2 - skip, eax=-1 - cancel,
;      PF, ZF, CF and SF set accordingly to 'cmp eax,2' (or 'cmp al,2')
        push    ebx
        push    70
        pop     eax
        mov     ebx, delinfo
        int     0x40
        pop     ebx
        test    eax, eax
        jz      .ret
        cmp     [del_bSkipAll], 0
        jz      @f
        push    2
        pop     eax
        jmp     .ret
@@:
        push    execdata
        push    aCannotDeleteFolder
        test    cl, 1
        jnz     @f
        mov     dword [esp], aCannotDeleteFile
@@:
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    DeleteErrorBtn
        push    4
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        cmp     al, -1
        jz      @f
        inc     eax
        cmp     al, 4   ; "cancel" button
        jnz     @f
        or      eax, -1
@@:
        cmp     al, 3   ; "skip all" button
        jnz     .ret
        mov     [del_bSkipAll], 1
        dec     eax
.ret:
        cmp     al, 2
        ret

delete_file:
; in: eax->BDFE block
; out: CF and ZF not set <=> cancel job ("ja cancel_label")
        pushad
        mov     [del_dir_stack_ptr], del_dir_stack
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     edi, execdata
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        mov     esi, [esp+28]
        mov     ecx, esi
        add     esi, 40
        mov     al, '/'
        stosb
.l1:
        lodsb
        cmp     edi, execdataend
        jb      @f
        call    panels_OnKey.bigfilename
        popad
        ret
@@:
        stosb
        test    al, al
        jnz     .l1
        mov     ecx, [esp+28]
        test    byte [ecx], 10h
        jnz     .delete_dir
.retrydel:
        xor     ecx, ecx
        call    delete_file_worker
        jae     @f
        jp      .retrydel
@@:
        popad
        ret

.delete_dir:
; recursive delete of directory
        xor     ebp, ebp        ; ebp will contain number of undeletable items
.return_from_recursion:
        mov     ebx, dirinfo
        mov     [ebx+dirinfo.first-dirinfo], ebp
        mov     [ebx+dirinfo.size-dirinfo], del_dir_query_size
        mov     [ebx+dirinfo.dirdata-dirinfo], del_dir_query_area
        mov     [ebx+dirinfo.name-dirinfo], execdata
        push    70
        pop     eax
        int     0x40
; if we get read error, the best available action is try to delete directory itself
        test    eax, eax
        jz      @f
        cmp     eax, 6
        jnz     .do_delete_dir
@@:
; loop through a directory and delete items
        mov     edx, del_dir_query_area+32
        imul    ebx, 304
        add     ebx, edx
.delete_dir_entry_loop:
        cmp     edx, ebx
        jae     .delete_dir_entry_done
; ignore special entries "." and ".."
        inc     ebp
        cmp     word [edx+40], '.'
        jz      .delete_dir_entry_continue
        cmp     word [edx+40], '..'
        jnz     @f
        cmp     byte [edx+42], 0
        jz      .delete_dir_entry_continue
@@:
        dec     ebp
        mov     esi, execdata
@@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-1], '/'
        mov     edi, esi
        lea     esi, [edx+40]
@@:
        cmp     edi, execdataend
        jae     .fullname_big
        lodsb
        stosb
        test    al, al
        jnz     @b
        test    byte [edx], 10h
        jnz     .entry_is_folder
.retry2:
        xor     ecx, ecx
        call    delete_file_worker
        ja      .cancel
        jz      .skip
        jp      .retry2
        jmp     .restore_name
.entry_is_folder:
; allocate new item in directory stack
        mov     eax, [del_dir_stack_ptr]
        mov     [eax], ebp
        add     eax, 4
        mov     [del_dir_stack_ptr], eax
; do recursive deleting
        jmp     .delete_dir
.fullname_big:
; we will just ignore such files and continue - in real life this situation can not happen
        inc     ebp
        mov     esi, execdataend-1
        jmp     .do_restore_name
.skip:
        inc     ebp
.restore_name:
        mov     esi, execdata
@@:
        lodsb
        test    al, al
        jnz     @b
        dec     esi
        dec     esi
.do_restore_name:
        std
@@:
        lodsb
        cmp     al, '/'
        jnz     @b
        cld
        mov     byte [esi+1], 0
.delete_dir_entry_continue:
        add     edx, 304
        jmp     .delete_dir_entry_loop
.delete_dir_entry_done:
.do_delete_dir:
        mov     cl, 1
        call    delete_file_worker
        ja      .cancel
        jz      @f
        jp      .delete_dir
@@:
; al=0 - OK, al=2 - skip this directory
; return to previous directory
; pop item from directory stack
        mov     ecx, [del_dir_stack_ptr]
        cmp     ecx, del_dir_stack
        jbe     .done
        sub     ecx, 4
        mov     [del_dir_stack_ptr], ecx
        mov     ebp, [ecx]
        cmp     al, 2
        sbb     ebp, -1
; restore prev directory name
        mov     esi, execdata
@@:
        lodsb
        test    al, al
        jnz     @b
        dec     esi
        dec     esi
        std
@@:
        lodsb
        cmp     al, '/'
        jnz     @b
        cld
        mov     byte [esi+1], 0
        jmp     .return_from_recursion
.done:
.cancel:
        mov     [dirinfo.first], 0      ; do not destroys flags
        popad
        ret
