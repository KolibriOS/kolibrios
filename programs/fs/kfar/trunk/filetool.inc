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
        jb      .do_delete_dir_entry
        cmp     ebx, del_dir_query_area+32+304*del_dir_query_size
        jnz     .delete_dir_entry_done
        jmp     .return_from_recursion
.do_delete_dir_entry:
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
        call    delete_last_name
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
        call    delete_last_name_from_end
        jmp     .return_from_recursion
.done:
.cancel:
        mov     [dirinfo.first], 0      ; do not destroys flags
        popad
        ret

makedir:
; create directory with name from CopyDestEditBuf+12
; destroys eax
        push    ebx
        push    70
        pop     eax
        mov     ebx, mkdirinfo
        int     0x40
        pop     ebx
        test    eax, eax
        jz      .ret
        cmp     dword [esp+8], DeleteErrorBtn
        jnz     @f
        cmp     [copy_bSkipAll], 0
        jz      @f
        push    1
        pop     eax
        jmp     .ret
@@:
        push    dword CopyDestEditBuf+12
        push    dword aCannotMakeFolder
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    dword [eax+20]
        push    dword [eax+16]
        push    eax
        push    3
        push    -1
        push    -1
        push    dword aError
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jz      makedir
.ret:
        ret     8

copy_file_worker:
; in: execdata = source name, CopyDestEditBuf+12 = destination name, edx = BDFE block for source
; out: CF and ZF not set <=> cancel job ("ja cancel_label")
; destroys eax,esi,edi
        push    CopyDestEditBuf+12+513
        cmp     [bDestIsFolder], 0
        jz      .noaddtoname
        mov     esi, CopyDestEditBuf+12
@@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-1], '/'
        pop     edi
        push    esi
        mov     edi, esi
        lea     esi, [edx+40]
@@:
        cmp     edi, CopyDestEditBuf+12+513
        jae     .overflow
        lodsb
        stosb
        test    al, al
        jnz     @b
        jmp     .noaddtoname
.overflow:
.ret_zf:
        pop     esi
        and     byte [esi-1], 0         ; ZF=1
        ret
.noaddtoname:
; Нельзя скопировать файл поверх самого себя!
        mov     esi, execdata
        mov     edi, CopyDestEditBuf+12
        push    esi edi
        call    strcmpi
        pop     edi esi
        jnz     @f
        push    esi
        push    aCannotCopyToSelf
        mov     eax, esp
        push    ContinueBtn
        push    1
        push    eax
        push    2
        push    -1
        push    -1
        push    aError
        call    SayErr
        pop     eax
        pop     eax
        jmp     .ret_zf
@@:
; Собственно, копируем
; esi->source name, edi->destination name
        push    ebx
        mov     [writeinfo.code], 2
        mov     [writeinfo.name], edi
        and     dword [writeinfo.first], 0
        and     dword [writeinfo.first+4], 0
        mov     [writeinfo.data], copy_buffer
        mov     ebx, readinfo
        and     dword [ebx+readinfo.first-readinfo], 0
        and     dword [ebx+readinfo.first+4-readinfo], 0
        mov     [ebx+readinfo.size-readinfo], copy_buffer_size
        mov     [ebx+readinfo.data-readinfo], copy_buffer
        mov     [ebx+readinfo.name-readinfo], esi
.copyloop:
        mov     ebx, readinfo
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .copyreadok
        cmp     eax, 6
        jz      .copyreadok
        cmp     [copy_bSkipAll2], 0
        jnz     .copyfailed_del2
        push    esi
        push    dword aCannotReadFile
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    dword DeleteErrorBtn
        push    4
        push    eax
        push    3
        push    -1
        push    -1
        push    dword aError
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jz      .copyloop
        jmp     .copyfailed_parseuser
.copyreadok:
        add     dword [readinfo.first], ebx
        adc     dword [readinfo.first+4], 0
        mov     [writeinfo.size], ebx
        test    ebx, ebx
        jnz     .copywrite
        cmp     byte [writeinfo.code], 2
        jnz     .copydone
.copywrite:
        mov     ebx, writeinfo
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .copywriteok
        cmp     [copy_bSkipAll2], 0
        jnz     .copyfailed_del2
        push    edi
        push    dword aCannotWriteFile
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    dword DeleteErrorBtn
        push    4
        push    eax
        push    3
        push    -1
        push    -1
        push    dword aError
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jz      .copywrite
.copyfailed_parseuser:
        cmp     al, 2
        jnz     @f
        mov     [copy_bSkipAll2], 1
        dec     eax
@@:
        cmp     al, 1
        pushf
        jmp     .copyfailed
.copywriteok:
        mov     ecx, [writeinfo.size]
        add     dword [writeinfo.first], ecx
        adc     dword [writeinfo.first+4], 0
        mov     [writeinfo.code], 3
        cmp     ecx, copy_buffer_size
        jz      .copyloop
.copydone:
; now try to set attributes from source, ignore errors
        mov     edi, attrinfo.attr
        mov     esi, edx
        push    8
        pop     ecx
        rep     movsd
        mov     ebx, attrinfo
        mov     [ebx+attrinfo.name-attrinfo], CopyDestEditBuf+12
        inc     dword [ebx]
        push    70
        pop     eax
        push    ebx
        int     0x40
        pop     ebx
        dec     dword [ebx]
        xor     eax, eax        ; ZF=1
.ret:
        pop     ebx
        pop     esi
        mov     byte [esi-1], 0
        ret
.copydone2:
        popf
        jmp     .ret
.copyfailed:
        cmp     [bConfirmDeleteIncomplete], 0
        jz      .copyfailed_del
        cmp     [writeinfo.code], 2
        jz      .copydone2
        push    dword aIncompleteFile
        mov     eax, esp
        push    dword DeleteOrKeepBtn
        push    2
        push    eax
        push    1
        push    -1
        push    -1
        push    dword aCopyCaption
        call    SayErr
        add     esp, 4
        test    eax, eax
        jnz     .copydone2
.copyfailed_del:
        mov     ebx, delinfo
        push    dword [ebx+21]
        mov     dword [ebx+21], edi
        push    70
        pop     eax
        int     0x40
; ignore errors
        pop     dword [delinfo+21]
        jmp     .copydone2
.copyfailed_del2:
        xor     eax, eax
        pushf
        jmp     .copyfailed_del

copy_file:
; in: eax->BDFE block for source, CopyDestEditBuf+12 contains ASCIIZ full name for destination
; out: CF and ZF not set <=> cancel job ("ja cancel_label")
        pushad
        mov     [copy_dir_stack_ptr], copy_dir_stack
        mov     [bNeedRestoreName], 0
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
        mov     edx, [esp+28]
        test    byte [edx], 10h
        jnz     .copy_dir
        call    copy_file_worker
.popad_ret:
        popad
        ret

.biiig:
        mov     byte [edi-1], 0
        jmp     .popad_ret

.copy_dir:
; recursive copy of directory
        cmp     [bDestIsFolder], 0
        mov     [bDestIsFolder], 0
        jz      .target_created
        mov     [bNeedRestoreName], 1
        mov     esi, CopyDestEditBuf+12
@@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-1], '/'
        mov     edi, esi
        lea     esi, [edx+40]
@@:
        cmp     edi, CopyDestEditBuf+12+513
        jae     .biiig
        lodsb
        stosb
        test    al, al
        jnz     @b
.create_target:
.enter_recursion:
        push    DeleteErrorBtn
        push    4
        call    makedir
        jz      .target_created
        cmp     al, 2
        jnz     @f
        dec     eax
        mov     [copy_bSkipAll], 1
@@:
        cmp     al, 1
        jz      .copy_dir_entry_done
        jmp     .cancel
.target_created:
        xor     ebp, ebp        ; ebp will contain number of copied items
.return_from_recursion:
.read_retry:
        mov     ebx, dirinfo
        mov     [ebx+dirinfo.first-dirinfo], ebp
        mov     [ebx+dirinfo.size-dirinfo], copy_dir_query_size
        mov     [ebx+dirinfo.dirdata-dirinfo], copy_dir_query_area
        mov     [ebx+dirinfo.name-dirinfo], execdata
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .readok
        cmp     eax, 6
        jz      .readok
; read error
        cmp     [copy_bSkipAll], 0
        jz      .skip1
        push    execdata
        push    aCannotReadFolder
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
        test    al, al
        jz      .read_retry
        cmp     al, 2
        jnz     @f
        dec     eax
        mov     [copy_bSkipAll], 1
@@:
        cmp     al, 1
        jz      .skip1
        jmp     .cancel
.readok:
; loop through a directory and copy items
        mov     edx, copy_dir_query_area+32
        imul    ebx, 304
        add     ebx, edx
.copy_dir_entry_loop:
        cmp     edx, ebx
        jb      .do_copy_dir_entry
        cmp     ebx, copy_dir_query_area+32+copy_dir_query_size*304
        jz      .return_from_recursion
        jmp     .copy_dir_entry_done
.do_copy_dir_entry:
        inc     ebp
; ignore special entries "." and ".."
        cmp     word [edx+40], '.'
        jz      .copy_dir_entry_continue
        cmp     word [edx+40], '..'
        jnz     @f
        cmp     byte [edx+42], 0
        jz      .copy_dir_entry_continue
@@:
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
        mov     esi, CopyDestEditBuf+12
@@:
        lodsb
        test    al, al
        jnz     @b
        mov     byte [esi-1], '/'
        mov     edi, esi
        lea     esi, [edx+40]
@@:
        cmp     edi, CopyDestEditBuf+513
        jae     .fullname2_big
        lodsb
        stosb
        test    al, al
        jnz     @b
        test    byte [edx], 10h
        jnz     .entry_is_folder
        call    copy_file_worker
        ja      .cancel
        jmp     .restore_name
.entry_is_folder:
; allocate new item in directory stack
        mov     eax, [copy_dir_stack_ptr]
        mov     [eax], ebp
        add     eax, 4
        mov     [copy_dir_stack_ptr], eax
; do recursive copying
        jmp     .enter_recursion
.fullname_big:
; we will just ignore such files and continue - in real life this situation can not happen
        mov     esi, execdataend-1
        jmp     .do_restore_name2
.fullname2_big:
        mov     esi, CopyDestEditBuf+12+512
        jmp     .restore_name2
.skip1:
.restore_name:
        mov     esi, CopyDestEditBuf+12
@@:
        lodsb
        test    al, al
        jnz     @b
        dec     esi
        dec     esi
.restore_name2:
        call    delete_last_name
        mov     esi, execdata
@@:
        lodsb
        test    al, al
        jnz     @b
        dec     esi
        dec     esi
.do_restore_name2:
        call    delete_last_name
.copy_dir_entry_continue:
        add     edx, 304
        jmp     .copy_dir_entry_loop
.copy_dir_entry_done:
; return to previous directory
; pop item from directory stack
        mov     ecx, [copy_dir_stack_ptr]
        cmp     ecx, copy_dir_stack
        jbe     .done
        sub     ecx, 4
        mov     [copy_dir_stack_ptr], ecx
        mov     ebp, [ecx]
; restore prev directory name
        mov     esi, execdata
        call    delete_last_name_from_end
        mov     esi, CopyDestEditBuf+12
        call    delete_last_name_from_end
        jmp     .return_from_recursion
.done:
.cancel:
        mov     [dirinfo.first], 0      ; do not destroys flags
        popad
        ret

delete_last_name_from_end:
        lodsb
        test    al, al
        jnz     delete_last_name_from_end
        dec     esi
        dec     esi
delete_last_name:
        std
@@:
        lodsb
        cmp     al, '/'
        jnz     @b
        cld
        mov     byte [esi+1], 0
        ret
