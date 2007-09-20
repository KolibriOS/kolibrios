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
        cmp     byte [edi-1], '/'
        jz      .l1
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
        lea     edi, [edx+40]
        and     [source_hFile], 0
copy_file_worker2:
        push    CopyDestEditBuf+12+513
        cmp     [bDestIsFolder], 0
        jz      .noaddtoname
        mov     esi, CopyDestEditBuf+12
@@:
        lodsb
        test    al, al
        jnz     @b
        pop     eax
        dec     esi
        push    esi
        cmp     byte [esi-1], '/'
        jz      @f
        mov     byte [esi], '/'
        inc     esi
@@:
        xchg    esi, edi
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
        and     byte [esi], 0           ; ZF=1
        ret
.noaddtoname:
; Нельзя скопировать файл поверх самого себя!
        mov     esi, execdata
        mov     edi, CopyDestEditBuf+12
        cmp     [source_hModule], 0
        jnz     @f
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
        mov     eax, [source_hFile]
        push    eax
        test    eax, eax
        jnz     .copyloop
.source_reopen:
        mov     eax, [source_hModule]
        test    eax, eax
        jz      .copyloop
        pushad
        push    O_READ+O_SEQUENTIAL_ONLY
        push    esi
        push    [source_hPlugin]
        call    [eax+PluginInfo.open]
        mov     [source_hFile], eax
        popad
.copyloop:
        mov     ebx, readinfo
        mov     eax, [source_hModule]
        test    eax, eax
        jz      .native
        mov     ecx, [source_hFile]
        jecxz   .readerr
        pushad
        push    [ebx+readinfo.size-readinfo]
        push    [ebx+readinfo.data-readinfo]
        push    ecx
        call    [eax+PluginInfo.read]
        mov     [esp+28], eax
        popad
        cmp     eax, -1
        jz      .readerr
        mov     ebx, eax
        jmp     .copyreadok
.native:
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .copyreadok
        cmp     eax, 6
        jz      .copyreadok
.readerr:
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
        call    SayErr
        add     esp, 3*4
        test    eax, eax
        jnz     .copyfailed_parseuser
        cmp     [source_hModule], 0
        jz      .copyloop
        cmp     [source_hFile], 0
        jz      .source_reopen
        jmp     .copyloop
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
        pop     ecx
        test    ecx, ecx
        jnz     @f
        mov     eax, [source_hModule]
        test    eax, eax
        jz      @f
        mov     ecx, [source_hFile]
        jecxz   @f
        push    edx
        push    ecx
        call    [eax+PluginInfo.close]
        pop     edx
@@:
; now try to set attributes from source, ignore errors
        mov     edi, attrinfo.attr
        mov     esi, edx
        push    8
        pop     ecx
        rep     movsd
; replace zero dates with default values
        mov     eax, [default_attr]
        cmp     dword [edi-32+8], 0
        jnz     @f
        mov     ecx, [eax+8]
        mov     [edi-32+8], ecx
        mov     ecx, [eax+12]
        mov     [edi-32+12], ecx
@@:
        cmp     dword [edi-32+16], 0
        jnz     @f
        mov     ecx, [eax+16]
        mov     [edi-32+16], ecx
        mov     ecx, [eax+20]
        mov     [edi-32+20], ecx
@@:
        cmp     dword [edi-32+24], 0
        jnz     @f
        mov     ecx, [eax+24]
        mov     [edi-32+24], ecx
        mov     ecx, [eax+28]
        mov     [edi-32+28], ecx
@@:
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
        mov     byte [esi], 0
        ret
.copydone2:
        popf
        jmp     .ret
.copyfailed:
        pop     ecx
        test    ecx, ecx
        jnz     @f
        mov     eax, [source_hModule]
        test    eax, eax
        jz      @f
        mov     ecx, [source_hFile]
        jecxz   @f
        push    ecx
        call    [eax+PluginInfo.close]
@@:
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
        push    dword aCopyCaption
        call    Message
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
        cmp     byte [edi-1], '/'
        jz      .l1
        mov     al, '/'
        stosb
.l1:
        lodsb
        cmp     edi, execdataend
        jb      @f
        call    panels_OnKey.bigfilename
        stc
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
        dec     esi
        cmp     byte [esi-1], '/'
        jz      @f
        mov     byte [esi], '/'
        inc     esi
@@:
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
        jmp     .cancel2
.target_created:
        mov     edx, [source_hModule]
        test    edx, edx
        jz      .nosetdir
        mov     esi, execdata
        push    esi     ; absolute_path
@@:
        lodsb
        test    al, al
        jnz     @b
@@:
        dec     esi
        cmp     byte [esi-1], '/'
        jnz     @b
        push    esi     ; relative_path
        push    [source_hPlugin]        ; hPlugin
        call    [edx+PluginInfo.SetFolder]
        test    al, al
        jnz     .nosetdir
        cmp     [copy_bSkipAll3], 0
        jz      .skip2
        push    execdata
        push    aCannotSetFolder
        mov     eax, esp
        push    DeleteErrorBtn
        push    4
        push    eax
        push    2
        call    SayErr
        pop     ecx ecx
        test    al, al
        jz      .target_created
        cmp     al, 2
        setz    [copy_bSkipAll3]
        ja      .cancel2
        jmp     .skip2
.nosetdir:
        xor     ebp, ebp        ; ebp will contain number of copied items
.return_from_recursion:
.read_retry:
        mov     ebx, dirinfo
        mov     [ebx+dirinfo.first-dirinfo], ebp
        mov     [ebx+dirinfo.size-dirinfo], copy_dir_query_size
        mov     [ebx+dirinfo.dirdata-dirinfo], copy_dir_query_area
        mov     [ebx+dirinfo.name-dirinfo], execdata
        mov     eax, [source_hModule]
        test    eax, eax
        jz      .readfolder_native
        push    ebp
        push    [ebx+dirinfo.dirdata-dirinfo]
        push    [ebx+dirinfo.size-dirinfo]
        push    [ebx+dirinfo.first-dirinfo]
        push    [source_hPlugin]
        call    [eax+PluginInfo.ReadFolder]
        pop     ebp
        mov     ebx, dword [copy_dir_query_area+4]
        jmp     @f
.readfolder_native:
        push    70
        pop     eax
        int     0x40
@@:
        test    eax, eax
        jz      .readok
        cmp     eax, 6
        jz      .readok
; read error
        cmp     [copy_bSkipAll], 0
        jnz     .skip1
        push    execdata
        push    aCannotReadFolder
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    DeleteErrorBtn
        push    4
        push    eax
        push    3
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
        dec     esi
        cmp     byte [esi-1], '/'
        jz      @f
        mov     byte [esi], '/'
        inc     esi
@@:
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
        dec     esi
        cmp     byte [esi-1], '/'
        jz      @f
        mov     byte [esi], '/'
        inc     esi
@@:
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
        ja      .cancel3
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
.skip1:
.copy_dir_entry_done:
; return to previous directory
        mov     esi, execdata
        call    delete_last_name_from_end
        mov     eax, [source_hModule]
        test    eax, eax
        jz      @f
        push    execdata
        push    aDotDot
        push    [source_hPlugin]
        call    [eax+PluginInfo.SetFolder]
        jmp     @f
.skip2:
        mov     esi, execdata
        call    delete_last_name_from_end
@@:
; pop item from directory stack
        mov     ecx, [copy_dir_stack_ptr]
        cmp     ecx, copy_dir_stack
        jbe     .done
        sub     ecx, 4
        mov     [copy_dir_stack_ptr], ecx
        mov     ebp, [ecx]
; restore prev directory name
        mov     esi, CopyDestEditBuf+12
        call    delete_last_name_from_end
        jmp     .return_from_recursion
.done:
        mov     [dirinfo.first], 0      ; do not destroys flags
        popad
        ret
.cancel2:
        sub     [copy_dir_stack_ptr], 4
.cancel3:
        mov     esi, execdata
        call    delete_last_name_from_end
.cancel:
        mov     eax, [source_hModule]
        test    eax, eax
        jz      .cancel.ret
        cmp     [copy_dir_stack_ptr], copy_dir_stack
        jb      .cancel.ret
        push    execdata
        push    aDotDot
        push    [source_hPlugin]
        call    [eax+PluginInfo.SetFolder]
        jmp     .cancel2
.cancel.ret:
        xor     eax, eax
        inc     eax
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
        inc     esi
        cmp     esi, execdata
        jz      @f
        cmp     esi, CopyDestEditBuf+12
        jz      @f
        dec     esi
@@:
        mov     byte [esi+1], 0
        ret

copy_AddDir:
        push    1
        pop     eax                     ; for "return true"
        pushad
        mov     esi, CopyDestEditBuf+12
@@:
        lodsb
        test    al, al
        jnz     @b
        cmp     byte [esi-2], '/'
        jnz     @f
        dec     esi
@@:
        push    dword [esi-1]
        push    esi
        mov     byte [esi-1], '/'
        mov     edi, esi
        mov     esi, [esp+28h+4]
.0:
        lodsb
        stosb
        cmp     edi, CopyDestEditBuf+12+512
        jae     .done
        test    al, al
        jnz     .0
        push    RetryOrCancelBtn
        push    2
        call    makedir
        jz      .done
        and     dword [esp+8+1Ch], 0
.done:
        pop     esi
        pop     dword [esi-1]
        popad
        ret     8

copy_AddFile:
        pushad
        mov     eax, [esp+20h+12]
        mov     [source_hFile], eax
        mov     edx, [esp+20h+8]
        mov     edi, [esp+20h+4]
        call    copy_file_worker2
        popad
        setna   al
        ret     12

virtual at 0
_FILE:
.pos            dq      ?
.bufpos         dq      ?
.bufsize        dd      ?
.mode           dd      ?
.hPlugin        dd      ?
.hFile          dd      ?
.fileinfo:
.fimode         dd      ?
.fioffset       dq      ?
.fisize         dd      ?
.fibuf          dd      ?
.finame         rb      1024
.attr           rb      40
align 512
.buf            rb      2048
.size = $
end virtual

O_READ = 1      ; allows read from file
O_WRITE = 2     ; allows write to file
O_CREATE = 4    ; if file does not exist and this flag is set, create file;
                ; if file does not exist and this flag is not set, fail
O_TRUNCATE = 8  ; truncate file if it exists
O_SEQUENTIAL_ONLY = 10h ; there will be no 'seek'/'setpos' calls

; HANDLE __stdcall open(const char* name, int mode);
; Opens physical file
open:
        pushad
        mov     ecx, _FILE.size
        call    xpgalloc
        test    eax, eax
        jz      .ret0z
        mov     [esp+28], eax
        mov     ecx, eax
        mov     esi, [esp+36]
        lea     edi, [eax+_FILE.finame]
        lea     edx, [eax+_FILE.finame+1024]
@@:
        lodsb
        stosb
        test    al, al
        jz      @f
        cmp     edi, edx
        jb      @b
.ret0:
        call    pgfree
.ret0z:
        popad
        xor     eax, eax
        ret     8
@@:
        mov     eax, [esp+40]
        mov     [ecx+_FILE.mode], eax
.getattr:
        lea     edi, [ecx+_FILE.fileinfo]
        mov     ebx, edi
        push    5
        pop     eax
        stosd
        xor     eax, eax
        stosd
        stosd
        stosd
        lea     eax, [ecx+_FILE.attr]
        stosd
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .found
        cmp     eax, 5
        jnz     .ret0
; file does not exist
        test    [ecx+_FILE.mode], O_CREATE
        jz      .ret0
.truncate:
        lea     ebx, [ecx+_FILE.fileinfo]
        mov     byte [ebx], 2
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .getattr
        jmp     .ret0
.found:
        test    [ecx+_FILE.mode], O_TRUNCATE
        jz      @f
        cmp     dword [ecx+_FILE.attr+36], eax
        jnz     .truncate
        cmp     dword [ecx+_FILE.attr+32], eax
        jnz     .truncate
@@:
        mov     dword [ecx+_FILE.pos], eax
        mov     dword [ecx+_FILE.pos+4], eax
        mov     dword [ecx+_FILE.bufpos], eax
        mov     dword [ecx+_FILE.bufpos+4], eax
        mov     [ecx+_FILE.bufsize], eax
        mov     [ecx+_FILE.hPlugin], eax
        mov     [ecx+_FILE.hFile], eax
        mov     dword [ecx+_FILE.fioffset], eax
        mov     dword [ecx+_FILE.fioffset+4], eax
        mov     [esp+28], ecx
        popad
        ret     8

; HANDLE __stdcall open2(int plugin_id, HANDLE plugin_instance, const char* name, int mode);
; Opens file on plugin panel
open2:
        cmp     dword [esp+4], 0
        jnz     .plugin
        pop     eax
        add     esp, 8
        push    eax
        jmp     open
.plugin:
        pushad
        mov     ecx, _FILE.size
        call    xpgalloc
        test    eax, eax
        jz      .ret0z
        mov     [esp+28], eax
        mov     ecx, eax
        mov     esi, [esp+44]
        lea     edi, [eax+_FILE.finame]
        lea     edx, [eax+_FILE.finame+1024]
@@:
        lodsb
        stosb
        test    al, al
        jz      @f
        cmp     edi, edx
        jb      @b
.ret0:
        call    pgfree
.ret0z:
        popad
        xor     eax, eax
        ret     8
@@:
        mov     edx, [esp+36]
        mov     [ecx+_FILE.hPlugin], edx
        mov     ebx, [esp+40]
        mov     eax, [esp+48]
        mov     [ecx+_FILE.mode], eax
        push    ebx ecx
        push    eax
        lea     eax, [ecx+_FILE.finame]
        push    eax
        push    ebx
        call    [edx+PluginInfo.open]
        pop     ecx ebx
        test    eax, eax
        jz      .ret0
        mov     [ecx+_FILE.hFile], eax
        mov     edx, [esp+36]
        push    ecx
        lea     edi, [ecx+_FILE.fileinfo]
        push    edi
        xor     eax, eax
        push    ecx
        push    10
        pop     ecx
        rep     stosd
        pop     ecx
        lea     eax, [ecx+_FILE.finame]
        push    eax
        push    ebx
        call    [edx+PluginInfo.getattr]
        pop     ecx
        xor     eax, eax
        mov     dword [ecx+_FILE.pos], eax
        mov     dword [ecx+_FILE.pos+4], eax
        mov     dword [ecx+_FILE.bufpos], eax
        mov     dword [ecx+_FILE.bufpos+4], eax
        mov     [ecx+_FILE.bufsize], eax
        mov     dword [ecx+_FILE.fioffset], eax
        mov     dword [ecx+_FILE.fioffset+4], eax
        mov     [esp+28], ecx
        popad
        ret     16

; unsigned __stdcall read(HANDLE hFile, void* buf, unsigned size);
read:
        xor     eax, eax
        pushad
        mov     ecx, [esp+36]
        test    [ecx+_FILE.mode], O_READ
        jnz     @f
.ret:
        popad
        ret     12
@@:
        cmp     dword [esp+44], eax
        jz      .ret
        mov     [ecx+_FILE.fimode], eax
        mov     ebx, [ecx+_FILE.bufsize]
        mov     eax, dword [ecx+_FILE.pos]
        and     eax, 2047
        sub     ebx, eax
        jbe     .nobuf0
        cmp     ebx, [esp+44]
        jbe     @f
        mov     ebx, [esp+44]
@@:
        push    ecx
        lea     esi, [ecx+eax+_FILE.buf]
        mov     ecx, ebx
        mov     edi, [esp+44]
        rep     movsb
        pop     ecx
        mov     [esp+40], edi
        add     [esp+28], ebx
        add     dword [ecx+_FILE.pos], ebx
        adc     dword [ecx+_FILE.pos+4], 0
        test    dword [ecx+_FILE.pos], 2047
        jnz     @f
        and     [ecx+_FILE.bufsize], 0
@@:
        sub     [esp+44], ebx
        jz      .ret
.nobuf0:
        test    dword [ecx+_FILE.pos], 2047
        jz      .aligned
        cmp     dword [ecx+_FILE.bufsize], 0
        jnz     .ret
        lea     ebx, [ecx+_FILE.fileinfo]
        mov     dword [ebx+12], 2048
        lea     eax, [ecx+_FILE.buf]
        mov     dword [ebx+16], eax
        mov     eax, dword [ecx+_FILE.fioffset]
        mov     dword [ecx+_FILE.bufpos], eax
        mov     eax, dword [ecx+_FILE.fioffset+4]
        mov     dword [ecx+_FILE.bufpos+4], eax
        call    .doread
        test    eax, eax
        jnz     .ret
        mov     [ecx+_FILE.bufsize], ebx
        mov     eax, dword [ecx+_FILE.pos]
        and     eax, 2047
        sub     ebx, eax
        jbe     .ret
        cmp     ebx, [esp+44]
        jbe     @f
        mov     ebx, [esp+44]
@@:
        push    ecx
        lea     esi, [ecx+eax+_FILE.buf]
        mov     ecx, ebx
        mov     edi, [esp+44]
        rep     movsb
        pop     ecx
        add     dword [ecx+_FILE.pos], ebx
        adc     dword [ecx+_FILE.pos+4], 0
        mov     [esp+40], edi
        add     [esp+28], ebx
        sub     [esp+44], ebx
        jz      .ret
        test    dword [ecx+_FILE.pos], 2047
        jnz     .ret
.aligned:
        lea     ebx, [ecx+_FILE.fileinfo]
        mov     eax, [esp+44]
        and     eax, not 2047
        jz      .finish
        and     [ecx+_FILE.bufsize], 0
        mov     [ebx+12], eax
        mov     eax, [esp+40]
        mov     [ebx+16], eax
        call    .doread
        test    eax, eax
        jnz     .ret
        add     dword [ecx+_FILE.pos], ebx
        adc     dword [ecx+_FILE.pos+4], 0
        add     [esp+28], ebx
        add     [esp+40], ebx
        sub     [esp+44], ebx
        jz      .ret
        cmp     ebx, [ecx+_FILE.fisize]
        jb      .ret
.finish:
        lea     ebx, [ecx+_FILE.fileinfo]
        mov     dword [ebx+12], 2048
        lea     eax, [ecx+_FILE.buf]
        mov     [ebx+16], eax
        and     [ecx+_FILE.bufsize], 0
        mov     eax, dword [ecx+_FILE.fioffset]
        mov     dword [ecx+_FILE.bufpos], eax
        mov     eax, dword [ecx+_FILE.fioffset+4]
        mov     dword [ecx+_FILE.bufpos+4], eax
        call    .doread
        test    eax, eax
        jnz     .ret
        mov     [ecx+_FILE.bufsize], ebx
        cmp     ebx, [esp+44]
        jb      @f
        mov     ebx, [esp+44]
@@:
        add     [esp+28], ebx
        add     dword [ecx+_FILE.pos], ebx
        adc     dword [ecx+_FILE.pos+4], 0
        lea     esi, [ecx+_FILE.buf]
        mov     edi, [esp+40]
        mov     ecx, ebx
        rep     movsb
        popad
        ret     12
.doread:
        mov     eax, [ecx+_FILE.hPlugin]
        test    eax, eax
        jz      .native
        push    ecx
        push    [ecx+_FILE.fisize]
        push    [ecx+_FILE.fibuf]
        push    [ecx+_FILE.hFile]
        call    [eax+PluginInfo.read]
        pop     ecx
        cmp     eax, -1
        jz      @f
        mov     ebx, eax
        xor     eax, eax
        jmp     .addpos
@@:
        ret
.native:
        push    70
        pop     eax
        int     0x40
        test    eax, eax
        jz      .addpos
        cmp     eax, 6
        jnz     @b
        xor     eax, eax
.addpos:
        add     dword [ecx+_FILE.fioffset], ebx
        adc     dword [ecx+_FILE.fioffset+4], 0
        ret

; void __stdcall seek(HANDLE hFile, int method, __int64 newpos);
seek:
        pushad
        mov     ecx, [esp+36]
        mov     eax, [esp+44]
        mov     edx, [esp+48]
        cmp     dword [esp+40], 1
        jb      .set
        ja      .end
        add     eax, dword [ecx+_FILE.pos]
        adc     edx, dword [ecx+_FILE.pos+4]
        jmp     .set
.end:
        add     eax, dword [ecx+_FILE.attr+32]
        adc     edx, dword [ecx+_FILE.attr+36]
.set:
        mov     dword [ecx+_FILE.pos], eax
        mov     dword [ecx+_FILE.pos+4], edx
        and     eax, not 2047
        cmp     eax, dword [ecx+_FILE.bufpos]
        jnz     @f
        cmp     edx, dword [ecx+_FILE.bufpos+4]
        jz      .bufposok
@@:
        and     [ecx+_FILE.bufsize], 0
        mov     dword [ecx+_FILE.bufpos], eax
        mov     dword [ecx+_FILE.bufpos+4], edx
.bufposok:
        cmp     [ecx+_FILE.bufsize], 0
        jnz     .ret
        cmp     eax, dword [ecx+_FILE.fioffset]
        jnz     @f
        cmp     edx, dword [ecx+_FILE.fioffset+4]
        jz      .ret
@@:
        mov     dword [ecx+_FILE.fioffset], eax
        mov     dword [ecx+_FILE.fioffset+4], edx
        mov     eax, [ecx+_FILE.hPlugin]
        test    eax, eax
        jz      @f
        push    dword [ecx+_FILE.fioffset+4]
        push    dword [ecx+_FILE.fioffset]
        push    [ecx+_FILE.hFile]
        call    [eax+PluginInfo.setpos]
@@:
.ret:
        popad
        ret     16

setpos_default:
        push    dword [esp+12]
        push    dword [esp+12]
        push    0
        push    dword [esp+16]
        call    seek
        ret     12

; __int64 __stdcall tell(HANDLE hFile);
tell:
        mov     eax, [esp+4]
        mov     edx, dword [eax+_FILE.pos+4]
        mov     eax, dword [eax+_FILE.pos]
        ret     4

; __int64 __stdcall filesize(HANDLE hFile);
filesize:
        mov     eax, [esp+4]
        mov     edx, dword [eax+_FILE.attr+36]
        mov     eax, dword [eax+_FILE.attr+32]
        ret     4

; void __stdcall close(HANDLE hFile);
close:
        pushad
        mov     ecx, [esp+24h]
        mov     eax, [ecx+_FILE.hPlugin]
        test    eax, eax
        jz      @f
        push    ecx
        push    [ecx+_FILE.hFile]
        call    [eax+PluginInfo.close]
        pop     ecx
@@:
        call    pgfree
        popad
        ret     4

getattr_default:
        mov     eax, 2
        ret     12
