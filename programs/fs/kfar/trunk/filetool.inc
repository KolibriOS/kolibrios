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
        pop     eax
        pop     ecx
        push    eax
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
        mov     edx, execdata
        call    recursive_read_folder_err
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
        jmp     .done

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

recursive_read_folder_err:
        push    edx
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
        ret

recursive_read_file_err:
	push	edx
	push	aCannotReadFile
	call	get_error_msg
	push	eax
	mov	eax, esp
	push	DeleteErrorBtn
	push	4
	push	eax
	push	3
	call	SayErr
	add	esp, 3*4
	test	al, al
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

search_dir_query_size = 32
search_filebuf_size = 32768

virtual at 0
filesearch_data:
.prev_screen_vtable     dd      ?       ; previous in window stack
.prev_screen_data       dd      ?
.tid                    dd      ?       ; -1 for query stage, secondary thread id for search stage
.slot                   dd      ?       ; 0 if secondary thread is not running, it's slot otherwise
.result_blocks          dd      ?       ; head of heap blocks for resulting data
.cur_result_ptr         dd      ?       ; offset of free data in the current heap block for result
.stop                   db      ?       ; set to 1 when secondary thread need to be stopped
.skip_errors_mask       db      ?
.skip_read_folder = 1
.skip_set_folder = 2
.skip_open_file = 4
.skip_read_file = 8
.skip_bigname = 10h
.updating               db      ?
.datachanged            db      ?
.list.curitemptr        dd      ?
.list.numitems          dd      ?
.list.head              dd      ?
.list.curitem           dd      ?
.list.end               dd      ?
.query_dlgdata          rb      filesearch_query_template.size
.search_dlgdata         rb      filesearch_search_template.size
.mask.maxlen            dd      ?
.mask.pos               dd      ?
.mask.start             dd      ?
.mask                   rb      512
.string.maxlen          dd      ?
.string.pos             dd      ?
.string.start           dd      ?
.string                 rb      256
.caption                rb      268
.statusstr              rb      80
.stack                  rb      8192
.stacktop = $
.fs.func                dd      ?
.fs.pos_low             dd      ?
.fs.pos_high            dd      ?
.fs.size                dd      ?
.fs.ptr                 dd      ?
.curfile                rb      1024
.curdir                 rb      1024
.lowername              rb      264
.dir_area               rb      32 + 304*search_dir_query_size
.filebuf                rb      search_filebuf_size
.size = $
end virtual

panels_OnKey.alt_f7:
        mov     ecx, filesearch_data.size
        mov     edx, filesearch_vtable
        call    new_screen
        test    eax, eax
        jnz     @f
        ret
@@:
        mov     [ebp+filesearch_data.prev_screen_vtable], panels_vtable
        mov     ecx, (filesearch_query_template.size + filesearch_search_template.size) / 4
        mov     esi, filesearch_query_template
        lea     edi, [ebp+filesearch_data.query_dlgdata]
        rep     movsd
        lea     eax, [ebp+filesearch_data.string.maxlen]
        xor     ebx, ebx
        mov     dword [ebp+filesearch_data.query_dlgdata+filesearch_query_template.editptr2-filesearch_query_template], eax
        mov     dword [eax], 253
        mov     [eax+4], ebx
        mov     [eax+8], ebx
        mov     [eax+12], bl
        sub     eax, filesearch_data.string - filesearch_data.mask
        mov     dword [ebp+filesearch_data.query_dlgdata+filesearch_query_template.editptr1-filesearch_query_template], eax
        mov     dword [eax], 511
        mov     [eax+8], ebx
        inc     ebx
        mov     word [eax+12], '*'
        mov     [eax+4], ebx
        mov     eax, [find_in_file_dlgdata.flags_case]
        and     al, 10h
        or      al, 8
        mov     dword [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_case-filesearch_query_template], eax
        mov     eax, [find_in_file_dlgdata.flags_whole]
        and     al, 10h
        or      al, 8
        mov     dword [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_whole-filesearch_query_template], eax
        lea     edi, [ebp+filesearch_data.caption]
        mov     dword [ebp+dlgtemplate.title+filesearch_data.search_dlgdata], edi
        lea     eax, [ebp+filesearch_data.list.curitemptr]
        mov     dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.data1-filesearch_search_template], eax
.reinit:
        xor     ebx, ebx
        or      [ebp+filesearch_data.tid], -1
        mov     [ebp+filesearch_data.slot], ebx
        mov     [ebp+filesearch_data.result_blocks], ebx
        mov     [ebp+filesearch_data.cur_result_ptr], 0x4000
        mov     [ebp+filesearch_data.list.curitemptr], ebx
        mov     [ebp+filesearch_data.list.numitems], ebx
        mov     [ebp+filesearch_data.list.head], ebx
        mov     [ebp+filesearch_data.list.curitem], ebx
        mov     [ebp+filesearch_data.list.end], ebx
        mov     [ebp+filesearch_data.stop], bl
        mov     al, [dialog_main_color]
        mov     [ebp+filesearch_data.query_dlgdata+dlgtemplate.main_color], al
        mov     [ebp+filesearch_data.search_dlgdata+dlgtemplate.main_color], al
        mov     al, [dialog_border_color]
        mov     [ebp+filesearch_data.query_dlgdata+dlgtemplate.border_color], al
        mov     [ebp+filesearch_data.search_dlgdata+dlgtemplate.border_color], al
        mov     al, [dialog_header_color]
        mov     [ebp+filesearch_data.query_dlgdata+dlgtemplate.header_color], al
        mov     [ebp+filesearch_data.search_dlgdata+dlgtemplate.header_color], al
filesearch_OnRedraw:
        push    ebp
        mov     byte [draw_image], 0xC3
        mov     eax, [ebp+filesearch_data.prev_screen_vtable]
        mov     ebp, [ebp+filesearch_data.prev_screen_data]
        call    dword [eax+screen_vtable.OnRedraw]
        mov     byte [draw_image], 0xC6
        pop     ebp
        cmp     [ebp+filesearch_data.tid], -1
        jnz     .prepare_search_dlg
        lea     ebx, [ebp+filesearch_data.query_dlgdata]
        mov     eax, [cur_width]
        mov     [ebx + dlgtemplate.x], 6
        sub     eax, 12
        mov     [ebx + dlgtemplate.width], eax
        dec     eax
        dec     eax
        mov     [ebx - filesearch_query_template + filesearch_query_template.width2], eax
        mov     [ebx - filesearch_query_template + filesearch_query_template.width3], eax
        shr     eax, 1
        dec     eax
        dec     eax
        mov     [ebx - filesearch_query_template + filesearch_query_template.search_x2], eax
        sub     eax, aSearchBLength-1
        mov     [ebx - filesearch_query_template + filesearch_query_template.search_x1], eax
        add     eax, aSearchBLength+3
        mov     [ebx - filesearch_query_template + filesearch_query_template.cnl_x1], eax
        add     eax, aCancelBLength-1
        mov     [ebx - filesearch_query_template + filesearch_query_template.cnl_x2], eax
        mov     eax, [cur_height]
        sub     eax, [ebx + dlgtemplate.height]
        shr     eax, 1
        mov     [ebx + dlgtemplate.y], eax
        jmp     .dlg_prepared
.prepare_search_dlg:
        lea     ebx, [ebp+filesearch_data.search_dlgdata]
; width: for big screens use all screen except for one column on each side,
;       that is, [cur_width] - 10 (standard frame width is 4);
; if [cur_width]-10 does not contain enough place for all buttons with two spaces between them,
;       but [cur_width]-2 does, use fixed size = place for all buttons with two spaces between them
; for small screens use [cur_width]-2 as maximum possible
.btnsize = 2 + aNewSearchLen + 2 + aGotoLen + 2 + aViewLen + 2 + aCancelB2Length + 2
        mov     eax, [cur_width]
        mov     edx, eax
        sub     eax, 10
        cmp     eax, .btnsize
        jae     @f
        add     eax, 8
        cmp     eax, .btnsize
        jb      @f
        mov     al, .btnsize
@@:
        mov     [ebx + dlgtemplate.width], eax
        sub     edx, eax
        shr     edx, 1
        mov     [ebx + dlgtemplate.x], edx
        dec     eax
        mov     [ebx + filesearch_search_template.width1 - filesearch_search_template], eax
        mov     [ebx + filesearch_search_template.width4 - filesearch_search_template], eax
        cmp     [ebp + filesearch_data.slot], 0
        jnz     @f
        mov     [ebx + filesearch_search_template.width3 - filesearch_search_template], eax
@@:
        sub     eax, aNewSearchLen + aGotoLen + aViewLen + aCancelB2Length - 1
        cdq
        push    5
        pop     ecx
        idiv    ecx
        sar     edx, 1
        add     edx, eax
        inc     eax
        mov     [ebx + filesearch_search_template.btn1x1 - filesearch_search_template], edx
        add     edx, aNewSearchLen-1
        mov     [ebx + filesearch_search_template.btn1x2 - filesearch_search_template], edx
        add     edx, eax
        mov     [ebx + filesearch_search_template.btn2x1 - filesearch_search_template], edx
        add     edx, aGotoLen-1
        mov     [ebx + filesearch_search_template.btn2x2 - filesearch_search_template], edx
        add     edx, eax
        mov     [ebx + filesearch_search_template.btn3x1 - filesearch_search_template], edx
        add     edx, aViewLen-1
        mov     [ebx + filesearch_search_template.btn3x2 - filesearch_search_template], edx
        add     edx, eax
        mov     [ebx + filesearch_search_template.btn4x1 - filesearch_search_template], edx
        add     edx, aCancelB2Length-1
        mov     [ebx + filesearch_search_template.btn4x2 - filesearch_search_template], edx
        mov     eax, [cur_height]
        mov     edx, eax
        sub     eax, 8
        cmp     eax, 6
        jae     @f
        mov     eax, 6
@@:
        mov     [ebx + dlgtemplate.height], eax
        sub     edx, eax
        shr     edx, 1
        mov     [ebx + dlgtemplate.y], edx
        dec     eax
        mov     [ebx + filesearch_search_template.btn1y - filesearch_search_template], eax
        mov     [ebx + filesearch_search_template.btn2y - filesearch_search_template], eax
        mov     [ebx + filesearch_search_template.btn3y - filesearch_search_template], eax
        mov     [ebx + filesearch_search_template.btn4y - filesearch_search_template], eax
        dec     eax
        mov     [ebx + filesearch_search_template.y5 - filesearch_search_template], eax
        dec     eax
        mov     [ebx + filesearch_search_template.y3 - filesearch_search_template], eax
        mov     [ebx + filesearch_search_template.y4 - filesearch_search_template], eax
        dec     eax
        mov     [ebx + filesearch_search_template.y2 - filesearch_search_template], eax
        dec     eax
        mov     [ebx + filesearch_search_template.height1 - filesearch_search_template], eax
.dlg_prepared:
        call    draw_dialog_shadow
        push    ebx
        call    DrawDialogBox
        ret

filesearch_OnActivate:
        mov     eax, [active_screen_data]
        cmp     eax, ebp
        jz      @f
        mov     [ebp+filesearch_data.prev_screen_data], eax
        mov     eax, [active_screen_vtable]
        mov     [ebp+filesearch_data.prev_screen_vtable], eax
@@:
        mov     eax, [ebp+filesearch_data.prev_screen_data]
        mov     ecx, [num_screens]
        mov     edx, [screens]
        push    edx
@@:
        cmp     [edx+4], eax
        jz      @f
        add     edx, 8
        loop    @b
@@:
        pop     edx
        jz      @f
        mov     eax, [active_screen]
        dec     eax
        mov     ecx, [edx+eax*8+4]
        mov     [ebp+filesearch_data.prev_screen_data], ecx
        mov     ecx, [edx+eax*8]
        mov     [ebp+filesearch_data.prev_screen_vtable], ecx
@@:
        cmp     [ebp+filesearch_data.slot], 0
        jz      @f
        mov     [idle_interval], 10
@@:
;        ret    ; continue to filesearch_OnIdle

filesearch_OnIdle:
        xor     eax, eax
        xchg    al, [ebp+filesearch_data.datachanged]
        cmp     al, 1
        ja      .done
        jz      .new
.ret:
        ret
.done:
        call    filesearch_wait_thread
        call    filesearch_done
.new:
        jmp     filesearch_OnRedraw

filesearch_OnKey:
        cmp     al, 0x58
        jz      F12
        cmp     [ebp+filesearch_data.tid], -1
        jz      .handle_generic
        cmp     al, 0x47
        jb      .notforlist
        cmp     al, 0x49
        jbe     @f
        cmp     al, 0x4F
        jb      .notforlist
        cmp     al, 0x51
        ja      .notforlist
@@:
        push    ebp
        add     ebp, filesearch_data.search_dlgdata
        lea     ebx, [ebp+dlgtemplate.size+12]
        call    listbox_key
        call    draw_listbox
        call    draw_image
        pop     ebp
        ret
.notforlist:
        cmp     al, 0x3D
        jz      .view
.handle_generic:
        push    ebp
        call    filesearch_getcurdlg
        push    0
        push    eax
        push    2
        push    ebx
        call    ManagerDlgProc
        pop     ebp
        test    eax, eax
        jz      filesearch_OnIdle.ret
        cmp     eax, -1
        jz      .esc
        sub     eax, ebp
        cmp     eax, filesearch_data.query_dlgdata+filesearch_query_template.search_btn-filesearch_query_template
        jz      .query
        cmp     eax, filesearch_data.search_dlgdata+filesearch_search_template.btn1-filesearch_search_template
        jz      .newsearch
        cmp     eax, filesearch_data.search_dlgdata+filesearch_search_template.btn2-filesearch_search_template
        jz      .goto
        cmp     eax, filesearch_data.search_dlgdata+filesearch_search_template.btn3-filesearch_search_template
        jz      .view
.esc:
        cmp     dword [ebp+filesearch_data.slot], 0
        jz      .exit
        mov     [ebp+filesearch_data.stop], 2
        call    ConfirmCancel
        setz    [ebp+filesearch_data.stop]
.ret:
        ret
.exit:
        call    filesearch_OnExit
        jmp     delete_active_screen
.view:
        mov     esi, [ebp+filesearch_data.list.curitemptr]
        test    esi, esi
        jz      .ret
        add     esi, 8
        mov     eax, esi
        jmp     view_file
.goto:
        mov     esi, [ebp+filesearch_data.list.curitemptr]
        test    esi, esi
        jz      .ret
        add     esi, 8
        cmp     byte [esi], '/'
        jnz     .ret
        cmp     byte [esi+1], 0
        jz      .ret
        push    ebp
        mov     ebp, [active_panel]
        push    esi
        call    close_plugin_panels
        pop     esi
        mov     ecx, esi
@@:
        lodsb
        test    al, al
        jnz     @b
@@:
        dec     esi
        cmp     byte [esi], '/'
        jnz     @b
        mov     byte [esi], 0
        inc     esi
        mov     edi, saved_file_name
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     esi, ecx
        lea     edi, [ebp + panel1_dir - panel1_data]
        push    esi edi
        call    strcmpi
        pop     edi esi
        jz      .goto.samedir
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        and     [ebp + panel1_start - panel1_data], 0
        and     [ebp + panel1_index - panel1_data], 0
.goto.samedir:
        push    @f
        push    [ebp + panel1_index - panel1_data]
        jmp     panels_OnKey.ctrl_r.doread
@@:
        pop     ebp
        jmp     .exit
.newsearch:
        cmp     [ebp+filesearch_data.slot], 0
        jz      .do.newsearch
        call    filesearch_stop_thread
.do.newsearch:
        call    filesearch_free_result
        jmp     panels_OnKey.alt_f7.reinit
.query:
        mov     esi, [active_panel]
        cmp     [esi + panel1_hPlugin - panel1_data], 0
        jz      @f
        push    aCannotSearchOnPlugin
        mov     eax, esp
        push    ContinueBtn
        push    1
        push    eax
        push    1
        call    SayErr
        pop     eax
        jmp     .exit
@@:
        add     esi, panel1_dir - panel1_data
        lea     edi, [ebp+filesearch_data.curdir]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        push    51
        pop     eax
        push    1
        pop     ebx
        mov     ecx, filesearch_thread
        lea     edx, [ebp+filesearch_data.stacktop-4]
        mov     [edx], ebp
        int     40h
        cmp     eax, -1
        jnz     @f
        push    ContinueBtn
        push    1
        push    aCannotCreateThread_ptr
        push    1
        call    SayErr
        ret
@@:
        mov     [ebp+filesearch_data.tid], eax
        mov     [ebp+filesearch_data.updating], 0
        mov     [ebp+filesearch_data.datachanged], 0
        xchg    eax, ecx
        push    18
        pop     eax
        push    21
        pop     ebx
        int     40h
        mov     [ebp+filesearch_data.slot], eax
        mov     [idle_interval], 10
        mov     esi, aFileSearch
        mov     edi, dword [ebp+filesearch_data.search_dlgdata+dlgtemplate.title]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     byte [edi-1], ':'
        mov     al, ' '
        stosb
        lea     esi, [ebp+filesearch_data.mask]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        lea     edi, [ebp+filesearch_data.statusstr]
        mov     dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.data3-filesearch_search_template], edi
        mov     esi, aSearchingIn
@@:
        lodsb
        stosb
        cmp     al, '"'
        jnz     @b
        push    esi
        push    10
        pop     ecx
        lea     esi, [ebp+filesearch_data.string]
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        loop    @b
@@:
        pop     esi
        movsb
        jz      @f
        mov     byte [edi-1], '.'
        mov     byte [edi-2], '.'
        mov     byte [edi-3], '.'
        mov     byte [edi-4], '"'
@@:
        cmp     byte [edi-2], '"'
        jnz     @f
        sub     edi, 3
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        sub     edi, dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.data3-filesearch_search_template]
        mov     dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.width3-filesearch_search_template], edi
        inc     edi
        mov     dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.x4-filesearch_search_template], edi
        lea     eax, [ebp+filesearch_data.curdir]
        mov     dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.data4-filesearch_search_template], eax
        call    filesearch_set_dlgflags         ; use it? I think, yes
        cmp     [ebp+filesearch_data.slot], 0
        jnz     @f
        call    filesearch_done
@@:
        jmp     filesearch_OnRedraw

filesearch_done:
        cmp     [active_screen_data], ebp
        jnz     @f
        or      [idle_interval], -1
@@:
        and     dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.data4-filesearch_search_template], 0
        mov     edi, dword [ebp+filesearch_data.search_dlgdata+filesearch_search_template.data3-filesearch_search_template]
        mov     esi, aSearchDone
@@:
        lodsb
        stosb
        cmp     al, '?'
        jnz     @b
        dec     edi
        mov     eax, [ebp+filesearch_data.list.numitems]
        push    -'0'
        push    10
        pop     ecx
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     al, '0'
        jz      @f
        stosb
        jmp     @b
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        ret

filesearch_getname:
if lang eq ru
        mov     eax, 'Диал'
        stosd
        mov     eax, 'ог п'
        stosd
        mov     eax, 'оиск'
        stosd
        mov     al, 'а'
        stosb
else
        mov     eax, 'Find'
        stosd
        mov     eax, ' dia'
        stosd
        mov     eax, 'log '
        stosd
        mov     al, ' '
        stosb
end if
        ret

filesearch_stop_thread:
        mov     [ebp+filesearch_data.stop], 1

filesearch_wait_thread:
        mov     ecx, [ebp+filesearch_data.slot]
        mov     edx, [ebp+filesearch_data.tid]
        jecxz   .secondary_thread_exited
@@:
        mov     ebx, procinfo
        push    9
        pop     eax
        int     40h
        cmp     word [ebx+50], 9
        jz      .secondary_thread_exited
        cmp     dword [ebx+30], edx
        jnz     .secondary_thread_exited
        push    5
        pop     eax
        push    1
        pop     ebx
        int     40h
        jmp     @b
.secondary_thread_exited:
        and     [ebp+filesearch_data.slot], 0
        ret

filesearch_OnExit:
        call    filesearch_stop_thread
        call    filesearch_free_result
;       call    filesearch_set_dlgflags         ; use it? I think, no
        ret

filesearch_IsHandleUsed:
        test    ebp, ebp
        ret

filesearch_set_dlgflags:
        mov     eax, dword [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_case-filesearch_query_template]
        and     al, 10h
        or      al, 8
        mov     [find_in_file_dlgdata.flags_case], eax
        mov     eax, dword [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_whole-filesearch_query_template]
        and     al, 10h
        or      al, 8
        mov     [find_in_file_dlgdata.flags_whole], eax
        lea     esi, [ebp+filesearch_data.string]
        mov     edi, SearchString
        and     dword [edi-4], 0
        and     dword [edi-8], 0
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        ret

filesearch_getcurdlg:
        lea     ebx, [ebp+filesearch_data.query_dlgdata]
        cmp     [ebp+filesearch_data.tid], -1
        jz      @f
        add     ebx, filesearch_data.search_dlgdata - filesearch_data.query_dlgdata
@@:
        ret

filesearch_free_result:
        mov     ecx, [ebp+filesearch_data.result_blocks]
@@:
        jecxz   .ret
        push    dword [ecx]
        call    pgfree
        pop     ecx
        jmp     @b
.ret:
        ret

filesearch_thread:
        pop     ebp
; initialize search for string
        xor     ecx, ecx
        xor     edx, edx
        mov     [ebp+filesearch_data.skip_errors_mask], cl
        lea     ebx, [ebp+filesearch_data.string]
        cmp     byte [ebx], dl
        jz      .noprepare
        mov     esi, tolower_table
        test    byte [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_case-filesearch_query_template], 10h
        jz      @f
        mov     esi, identical_table
@@:
        test    byte [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_whole-filesearch_query_template], 10h
        setnz   al
        push    eax
        push    ecx     ; force cp866
        call    search_string_pre
.noprepare:
        push    -1
.enter_recursion:
        xor     esi, esi        ; start position: zero
.read_folder_loop:
        push    esi
        lea     esi, [ebp+filesearch_data.curdir]
        lea     edi, [ebp+filesearch_data.curfile]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     esi
.read_retry:
        lea     ebx, [ebp+filesearch_data.fs.func]
        mov     dword [ebx], 1
        mov     dword [ebx+4], esi
        and     dword [ebx+8], 0
        mov     dword [ebx+12], search_dir_query_size
        lea     eax, [ebp+filesearch_data.dir_area]
        mov     [ebx+16], eax
        push    70
        pop     eax
        int     40h
        test    eax, eax
        jz      .read_folder_ok
        cmp     eax, 6
        jz      .read_folder_ok
        xor     ebx, ebx
        xchg    ebx, dword [ebp+filesearch_data.dir_area+4]
        test    [ebp+filesearch_data.skip_errors_mask], filesearch_data.skip_read_folder
        jnz     .skip1
        push    edx
        lea     edx, [ebp+filesearch_data.curdir]
        call    recursive_read_folder_err
        pop     edx
        jz      .read_retry
        cmp     al, 2
        jnz     @f
        dec     eax
        or      [ebp+filesearch_data.skip_errors_mask], filesearch_data.skip_read_folder
@@:
        cmp     al, 1
        jnz     .cancel
.skip1:
.read_folder_ok:
        imul    ebx, 304
        lea     eax, [ebp+filesearch_data.dir_area+32]
        add     ebx, eax
.scan_folder_loop:
        cmp     eax, ebx
        jae     .scan_folder_done
; ignore special entries "." and ".."
        cmp     word [eax+40], '.'
        jz      .scan_folder_next
        cmp     word [eax+40], '..'
        jnz     @f
        cmp     byte [eax+42], 0
        jz      .scan_folder_next
@@:
        call    .check_stop
; construct name
        push    esi edi ebx eax
        lea     ebx, [ebp+filesearch_data.curfile+1023]
        lea     esi, [eax+40]
        mov     byte [edi-1], '/'
@@:
        cmp     edi, ebx
        jae     .namebig
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     eax
        test    byte [eax], 10h
        jz      .scan_file
; it is nested folder, enter recursion
; to maintain ASCIIZ string coherency, copy backward
        lea     esi, [edi+filesearch_data.curdir-filesearch_data.curfile]
@@:
        dec     edi
        dec     esi
        mov     al, [edi]
        mov     [esi], al
        cmp     al, '/'
        jnz     @b
        mov     [ebp+filesearch_data.datachanged], 1
        pop     ebx edi
        jmp     .enter_recursion
.namebig:
        pop     eax
        mov     byte [edi], 0
        test    [ebp+filesearch_data.skip_errors_mask], filesearch_data.skip_bigname
        jnz     .namebig.skip
        push    eax
        push    edi
        test    byte [eax], 10h
        mov     eax, aFileNameTooBig
        jz      @f
        add     eax, aFolderNameTooBig - aFileNameTooBig
@@:
        push    eax
        mov     eax, esp
        push    SkipOrCancelBtn
        push    3
        push    eax
        push    2
        call    SayErr
        cmp     al, 1
        jnz     @f
        dec     eax
        or      [ebp+filesearch_data.skip_errors_mask], filesearch_data.skip_bigname
@@:
        test    al, al
        pop     eax eax eax
        jnz     .cancel
.namebig.skip:
        jmp     .scan_folder_next_pop
.scan_file:
; it is a file
; first check: does file name match the mask?
        push    eax edx
; обрюнетить
        lea     esi, [eax+40]
        lea     edi, [ebp+filesearch_data.lowername]
        push    edi
        xor     eax, eax
@@:
        lodsb
        mov     al, [tolower_table+eax]
        stosb
        test    al, al
        jnz     @b
; поехали
        lea     edx, [edi-1]
        pop     esi
        lea     edi, [ebp+filesearch_data.mask]
        call    match_mask_rev_lowercase
        pop     edx eax
        jc      .scan_folder_next_pop
; ok, second check: is the string present in this file?
        jecxz   .file_found
        push    eax
        call    filesearch_test_file
        pop     eax
        jc      .scan_folder_next_pop
.file_found:
        push    eax
; allocate memory for new result
        lea     esi, [ebp+filesearch_data.curfile]
        mov     ebx, esi
@@:
        lodsb
        test    al, al
        jnz     @b
        sub     esi, ebx
        mov     eax, [ebp+filesearch_data.cur_result_ptr]
        add     esi, 8
        push    ecx
        mov     ecx, 0x4000
        sub     ecx, eax
        cmp     ecx, esi
        jae     .nonewblock
        mov     ecx, 0x4000
        call    xpgalloc
        test    eax, eax
        jz      .cancel
        pushd   [ebp+filesearch_data.result_blocks]
        popd    [eax]
        mov     [ebp+filesearch_data.result_blocks], eax
        push    4
        pop     eax
        mov     [ebp+filesearch_data.cur_result_ptr], eax
.nonewblock:
        add     eax, [ebp+filesearch_data.result_blocks]
        add     [ebp+filesearch_data.cur_result_ptr], esi
        pop     ecx
; eax -> allocated memory, fill it
        push    eax
        lea     edi, [eax+8]
        mov     esi, ebx
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     eax
        mov     ebx, [ebp+filesearch_data.list.end]
        mov     [eax+4], ebx
        and     dword [eax], 0
        mov     [ebp+filesearch_data.list.end], eax
        test    ebx, ebx
        jz      @f
        mov     [ebx], eax
@@:
        inc     [ebp+filesearch_data.list.numitems]
        cmp     [ebp+filesearch_data.list.curitemptr], 0
        jnz     @f
        mov     [ebp+filesearch_data.list.curitemptr], eax
@@:
        cmp     [ebp+filesearch_data.list.head], 0
        jnz     @f
        mov     [ebp+filesearch_data.list.head], eax
@@:
        mov     [ebp+filesearch_data.datachanged], 1
        pop     eax
.scan_folder_next_pop:
        pop     ebx edi esi
        mov     byte [edi-1], 0
.scan_folder_next:
        inc     esi
        add     eax, 304
        jmp     .scan_folder_loop
.scan_folder_done:
        cmp     dword [ebp+filesearch_data.dir_area+4], search_dir_query_size
        jz      .read_folder_loop
        pop     esi
        inc     esi
        jz      .done
        lea     eax, [ebp+filesearch_data.curdir]
@@:
        inc     eax
        cmp     byte [eax], 0
        jnz     @b
@@:
        dec     eax
        cmp     byte [eax], '/'
        jnz     @b
        mov     byte [eax], 0
        mov     [ebp+filesearch_data.datachanged], 1
        jmp     .read_folder_loop
.cancel:
.done:
        mov     [ebp+filesearch_data.datachanged], 2
        or      eax, -1
        int     40h

.check_stop:
        cmp     [ebp+filesearch_data.stop], 1
        jz      .cancel
        ja      @f
        ret
@@:
        push    eax ebx
        push    5
        pop     eax
        push    1
        pop     ebx
        int     40h
        pop     ebx eax
        jmp     .check_stop

filesearch_test_file:
        lea     ebx, [ebp+filesearch_data.fs.func]
        lea     eax, [ebp+filesearch_data.filebuf]
        mov     [ebx+16], eax
        xor     eax, eax
        mov     [ebx], eax
        mov     [ebx+4], eax
        mov     [ebx+8], eax
        mov     dword [ebx+12], search_filebuf_size
        mov     edi, edx
        test    byte [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_whole-filesearch_query_template], 10h
        jz      .loop
        add     edi, 256
.loop:
.read_retry:
	call	filesearch_thread.check_stop
        push    70
        pop     eax
        lea     ebx, [ebp+filesearch_data.fs.func]
        int     40h
        test    eax, eax
        jz      .readok
        cmp     eax, 6
        jz      .readok
        test    [ebp+filesearch_data.skip_errors_mask], filesearch_data.skip_read_file
        jnz     .ret_failed
	push	edx
	lea	edx, [ebp+filesearch_data.curfile]
	call	recursive_read_file_err
	pop	edx
	jz	.read_retry
	cmp	al, 2
	jnz	@f
	dec	eax
	or	[ebp+filesearch_data.skip_errors_mask], filesearch_data.skip_read_file
@@:
	cmp	al, 1
	jnz	filesearch_thread.cancel
.ret_failed:
	stc
	ret
.readok:
	test	ebx, ebx
	jz	.eof
	push	ebx
	lea	esi, [ebp+filesearch_data.filebuf]
	add	ebx, esi
; edi = current state, edx -> FSM, ecx = last state,
; esi -> current data, ebx -> end of buffer
.scanloop:	 ; loop unrolled
; get current symbol
	movzx	eax, byte [esi]
; calculate next state
	movzx	edi, byte [edi+eax]
; done?
	cmp	edi, ecx
	jz	.ret_ok_pop
; no; proceed to next symbol
	shl	edi, 8
	add	edi, edx
	cmp	esi, ebx
	jae	.scandone
	movzx	eax, byte [esi+1]
	add	esi, 2
	movzx	edi, byte [edi+eax]
	cmp	edi, ecx
	jz	.ret_ok_pop
	shl	edi, 8
	add	edi, edx
	cmp	esi, ebx
	jb	.scanloop
.scandone:
	pop	ebx
.eof:
	add	[ebp+filesearch_data.fs.pos_low], ebx
	adc	[ebp+filesearch_data.fs.pos_high], 0
	cmp	ebx, search_filebuf_size
	jz	.loop
; EOF, last chance for whole-words-only search
	test	byte [ebp+filesearch_data.query_dlgdata+filesearch_query_template.flags_whole-filesearch_query_template], 10h
	jz	.ret_failed
	movzx	edi, byte [edi+' ']
	cmp	edi, ecx
	jnz	.ret_failed
	clc
	ret
.ret_ok_pop:
	pop	ebx
	clc
	ret

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
