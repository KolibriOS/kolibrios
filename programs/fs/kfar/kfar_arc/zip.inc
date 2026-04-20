; Loader of ZIP archives for KFar_Arc.
; Written by diamond in 2007.

virtual at 0
file_in_zip:
.fullname       dd      ?       ; pointer to cp866 string
.name           dd      ?
.namelen        dd      ?
.bIsDirectory   db      ?
.bPseudoFolder  db      ?
                rb      2
.parent         dd      ?       ; pointer to parent directory record
.subfolders     dd      ?       ; head of L2-list of subfolders [for folders]
.subfolders.end dd      ?
.subfiles       dd      ?       ; head of L2-list of files [for folders]
.subfiles.end   dd      ?
.NumSubItems    dd      ?
.next           dd      ?       ; next item of list of subfolders or files
.prev           dd      ?       ; previous item of list of subfolders or files
.stamp          dd      ?
.version_made_by dw     ?
.version_needed dw      ?
.flags          dw      ?
.method         dw      ?
.time           dw      ?
.date           dw      ?
.crc            dd      ?
.compressed_size dq     ?
.uncompressed_size dq   ?
.local_hdr      dq      ?
.disk           dd      ?
.attr           dd      ?
.size = $
end virtual

virtual at 0
handle_zip:
.type           dd      ?
.root.subfolders dd     ?
.root.subfolders.end dd ?
.root.subfiles  dd      ?
.root.subfiles.end dd   ?
.root.NumSubItems dd    ?
.curdir         dd      ?
.NumFiles       dd      ?
.names_buf      dd      ?
.host           dd      ?
.host_datetime  rd      6
.num_disks      dd      ?
.cur_disk       dd      ?
.cur_handle     dd      ?
.host_hPlugin   dd      ?
.host_idPlugin  dd      ?
.password_len   dd      ?       ; -1 if no password defined; in characters
.password       rb      password_maxlen
.basesize = $
end virtual

uglobal
align 4
zip.num_disks   dd      ?
zip.cur_disk    dd      ?
zip.cur_handle  dd      ?
zip.NumAllocated dd     ?
zip.NamesSize   dd      ?
zip.disk_name   rb      1024

zip64eocd:      ; zip64 end of central directory record
.sig            dd      ?       ; signature
.size           dq      ?       ; size of this record
.version_made_by dw     ?       ; version made by
.version_needed dw      ?       ; version needed to extract
.disk           dd      ?       ; number of this disk
.cdir_disk      dd      ?       ; number of the disk with the start
                                ; of the central directory
.cdir_entries_disk dq   ?       ; total number of entries in the central
                                ; directory on this disk
.cdir_entries   dq      ?       ; total number of entries in the central directory
.cdir_size      dq      ?       ; size of the central directory
.cdir_start     dq      ?       ; offset of start of central directory
.sz = $ - zip64eocd

zip.file_header:
.sig            dd      ?
.version_made_by dw     ?       ; (absent in local file header)
.version_needed dw      ?
.flags          dw      ?       ; general purpose bit flag
.method         dw      ?       ; compression method
.time           dw      ?       ; last modified file time
.date           dw      ?       ; last modified file date
.crc            dd      ?
.compressed_size dd     ?
.uncompressed_size dd   ?
.fname_len      dw      ?       ; file name length
.extra_len      dw      ?       ; extra field length
.sz_local = $ - (zip.file_header+2) ; size of local file header
.fcmt_len       dw      ?       ; file comment length
.disk           dw      ?       ; disk number start
.internal_attr  dw      ?
.external_attr  dd      ?
.local_hdr      dd      ?       ; relative offset of local header
.sz_cdir = $ - zip.file_header  ; size of file header in central directory

zip.extra:
.id     dw      ?
.size   dw      ?
zip.extra.zip64:
.uncompressed_size      dq      ?
.compressed_size        dq      ?
.local_hdr              dq      ?
.disk                   dd      ?
.sz = $ - zip.extra.zip64
endg

open_zip:
        and     [zip.cur_handle], 0
; look for end of central directory record
        push    ebp
        call    [filesize]
        sub     eax, 1024
        sbb     edx, 0
        jnb     @f
        xor     eax, eax
        xor     edx, edx
@@:
        push    edx
        push    eax
        push    0
        push    ebp
        call    [seek]
        push    1024
        push    buffer
        push    ebp
        call    [read]
        cmp     eax, 22
        jge     @f
        xor     eax, eax
.ret:
        ret
@@:
        mov     [inStream], ebp
        lea     edi, [buffer+eax-22]
@@:
        cmp     dword [edi], 0x06054B50
        jz      .eocd_found
        dec     edi
        cmp     edi, buffer
        jae     @b
        xor     eax, eax
        ret
.eocd_found:
        cmp     edi, buffer+20
        jb      .no_eocd64
        cmp     dword [edi-20], 0x07064B50
        jnz     .no_eocd64
        mov     ebx, ebp
        mov     ecx, [edi-16]
        mov     edx, [edi-4]
        mov     [zip.num_disks], edx
        dec     edx
        mov     [zip.cur_disk], edx
        cmp     edx, ecx
        jz      @f
        push    1
        push    dword [esp+8+28]
        push    dword [esp+12+24]
        push    dword [esp+16+20]
        call    zip.open_splitted
        test    eax, eax
        jz      .ret
        mov     ebx, eax
@@:
        push    dword [edi-8]
        push    dword [edi-12]
        push    0
        push    ebx
        call    [seek]
        push    zip64eocd.sz
        push    zip64eocd
        push    ebx
        call    [read]
        cmp     eax, zip64eocd.sz
        jnz     .close_err
        cmp     [zip64eocd.sig], 0x06064B50
        jnz     .close_err
        mov     ecx, [zip64eocd.cdir_disk]
        cmp     ecx, [zip.cur_disk]
        jz      @f
        push    1
        push    dword [esp+8+28]
        push    dword [esp+12+24]
        push    dword [esp+16+20]
        call    zip.open_splitted
        test    eax, eax
        jz      .ret
        mov     ebx, eax
@@:
        push    dword [zip64eocd.cdir_start+4]
        push    dword [zip64eocd.cdir_start]
        push    0
        push    ebx
        call    [seek]
        mov     ecx, dword [zip64eocd.cdir_entries]
        jmp     .parse_cdir
.close_err:
        call    close_cur_handle
        or      eax, -1
        ret
.no_eocd64:
        mov     ebx, ebp
        movzx   ecx, word [edi+6]
        movzx   edx, word [edi+4]
        inc     edx
        mov     [zip.num_disks], edx
        dec     edx
        mov     [zip.cur_disk], edx
        cmp     edx, ecx
        jz      @f
        push    1
        push    dword [esp+8+28]
        push    dword [esp+12+24]
        push    dword [esp+16+20]
        call    zip.open_splitted
        test    eax, eax
        jz      .ret
        mov     ebx, eax
@@:
        push    0
        push    dword [edi+16]
        push    0
        push    ebx
        call    [seek]
        movzx   ecx, word [edi+10]
.parse_cdir:
        mov     [zip.NumAllocated], ecx
        imul    ecx, file_in_zip.size
        jc      .close_err
        add     ecx, handle_zip.basesize
        jc      .close_err
        mov     [hOut.allocated], ecx
        call    [pgalloc]
        test    eax, eax
        jz      .close_err
        mov     [hOut], eax
        push    eax
        mov     edi, eax
        shr     ecx, 2
        xor     eax, eax
        rep     stosd
        mov     edi, [esp+8+28]
        dec     ecx
        repnz   scasb
        not     ecx
        mov     [zip.NamesSize], ecx
        call    [pgalloc]
        mov     ebp, eax
        pop     edi
        test    eax, eax
        jz      .free_err
        push    edi
        mov     edi, ebp
        mov     esi, [esp+8+28]
        rep     movsb
        pop     edi
        mov     byte [edi+handle_zip.type], type_zip
        add     edi, handle_zip.basesize + file_in_zip.version_made_by
.cdir_loop:
        mov     esi, zip.file_header
        push    zip.file_header.sz_cdir
        push    esi
        push    ebx
        call    [read]
        cmp     eax, zip.file_header.sz_cdir
        jz      .cdir_item_ok
        mov     ecx, [zip.cur_disk]
        inc     ecx
        cmp     ecx, [zip.num_disks]
        jz      .cdir_done
        push    1
        push    dword [esp+8+28]
        push    dword [esp+12+24]
        push    dword [esp+16+20]
        call    zip.open_splitted
        test    eax, eax
        jz      .free_err
        mov     ebx, eax
        jmp     .cdir_loop
.cdir_item_ok:
        lodsd
        cmp     eax, 0x02014B50
        jnz     .cdir_done
        mov     eax, [hOut]
        inc     [eax+handle_zip.NumFiles]
        mov     eax, [eax+handle_zip.NumFiles]
        cmp     eax, [zip.NumAllocated]
        jbe     .norealloc
        inc     [zip.NumAllocated]
        mov     ecx, [hOut.allocated]
        add     ecx, file_in_zip.size
        push    ecx
        and     ecx, 0xFFF
        cmp     ecx, file_in_zip.size
        pop     ecx
        mov     [hOut.allocated], ecx
        ja      .norealloc
        mov     edx, [hOut]
        call    [pgrealloc]
        test    eax, eax
        jnz     @f
.free_err:
        mov     ecx, [hOut]
        call    [pgfree]
        mov     ecx, ebp
        call    [pgfree]
        jmp     .close_err
@@:
        mov     [hOut], eax
        sub     edi, edx
        add     edi, eax
.norealloc:
        push    5
        pop     ecx
        rep     movsd
        xor     eax, eax
        stosd
        movsd
        stosd
        lodsw
        mov     ecx, [zip.NamesSize]
        lea     edx, [ecx+0xFFF]
        and     edx, not 0xFFF
        lea     ecx, [ecx+eax+1]
        mov     [zip.NamesSize], ecx
        push    ecx
        add     ecx, 0xFFF
        and     ecx, not 0xFFF
        cmp     ecx, edx
        pop     ecx
        jz      .noreallocname
        mov     edx, ebp
        call    [pgrealloc]
        test    eax, eax
        jz      .free_err
        mov     ebp, eax
.noreallocname:
        movzx   eax, word [esi-2]
        inc     eax
        mov     edx, [zip.NamesSize]
        sub     edx, eax
        mov     dword [edi-(file_in_zip.uncompressed_size+8)+file_in_zip.fullname], edx
        add     edx, ebp
        dec     eax
        mov     byte [edx+eax], 0
        push    eax
        push    edx
        push    ebx
        call    [read]
        movzx   eax, word [esi-2]
        cmp     byte [edx+eax-1], '/'
        jnz     @f
        mov     byte [edx+eax-1], 0
        mov     byte [edi-(file_in_zip.uncompressed_size+8)+file_in_zip.bIsDirectory], 1
@@:
        mov     eax, [esi+12]
        mov     [edi], eax
        movzx   eax, word [esi+4]
        mov     [edi+8], eax
        lodsw
        push    esi
        movzx   ecx, ax
        mov     edx, ecx
.parse_extra:
        sub     ecx, 4
        jb      .extra_done
        mov     esi, zip.extra
        push    4
        push    esi
        push    ebx
        call    [read]
        cmp     eax, 4
        jnz     .extra_done
        sub     edx, eax
        cmp     word [esi], 1
        jnz     .extra_skip
        mov     esi, zip.extra.zip64
        push    esi edi
        sub     edi, 8
        xchg    esi, edi
        movsd
        movsd
        sub     esi, 16
        movsd
        movsd
        add     esi, 8
        movsd
        movsd
        movsd
        pop     edi esi
        movzx   eax, [zip.extra.size]
        cmp     eax, zip.extra.zip64.sz
        jb      @f
        mov     al, zip.extra.zip64.sz
@@:
        sub     ecx, eax
        jb      .extra_done
        push    eax
        push    esi
        push    ebx
        call    [read]
        sub     edx, eax
        sub     [zip.extra.size], ax
        sub     edi, 8
        movsd
        movsd
        sub     edi, 16
        movsd
        movsd
        add     edi, 8
        movsd
        movsd
        movsd
        sub     edi, 12
.extra_skip:
        movzx   eax, word [zip.extra.size]
        sub     ecx, eax
        jb      .extra_done
        sub     edx, eax
        push    0
        push    eax
        push    1
        push    ebx
        call    [seek]
        jmp     .parse_extra
.extra_done:
        pop     esi
        xor     eax, eax
        lodsw
        add     edx, eax
        jz      @f
        push    0
        push    edx
        push    1
        push    ebx
        call    [seek]
@@:
        mov     eax, [esi+4]
        mov     dword [edi+file_in_zip.attr-(file_in_zip.uncompressed_size+8)], eax
;        test    al, 10h
;        setnz   byte [edi+file_in_zip.bIsDirectory-(file_in_zip.uncompressed_size+8)]
        add     edi, file_in_zip.size-file_in_zip.uncompressed_size-8+file_in_zip.version_made_by
        jmp     .cdir_loop
.cdir_done:
        mov     edi, [hOut]
        mov     esi, zip.num_disks
        push    edi
        add     edi, handle_zip.num_disks
        movsd
        movsd
        movsd
        pop     edi
        mov     [edi+handle_zip.names_buf], ebp
        mov     ecx, [edi+handle_zip.NumFiles]
        jecxz   .nofiles
        add     edi, handle_zip.basesize + file_in_zip.fullname
@@:
        add     [edi], ebp
        add     edi, file_in_zip.size
        loop    @b
.nofiles:
        mov     edx, [hOut]
        mov     ebx, [edx+handle_zip.NumFiles]
        lea     edi, [edx+handle_zip.root.subfolders]
        add     edx, handle_zip.basesize
        push    file_in_zip.size
        call    init_file_links
        mov     eax, [hOut]
        and     [eax+handle_zip.curdir], 0      ; set root directory
        mov     esi, [inStream]
        mov     [eax+handle_zip.host], esi
        or      [eax+handle_zip.password_len], -1
        mov     esi, [esp+4+20]
        mov     [eax+handle_zip.host_idPlugin], esi
        mov     esi, [esp+4+24]
        mov     [eax+handle_zip.host_hPlugin], esi
        lea     edi, [eax+handle_zip.host_datetime]
        mov     esi, [esp+12]
        add     esi, 8
        mov     ecx, 6
        rep     movsd
        ret

close_cur_handle:
        mov     edx, zip.cur_handle
        or      dword [edx-4], -1
        cmp     dword [edx], 0
        jz      @f
        push    dword [edx]
        call    [close]
        and     dword [edx], 0
@@:
        ret

zip.open_splitted:
        call    close_cur_handle
        mov     [zip.cur_disk], ecx
        inc     ecx
        mov     edx, ecx
        cmp     ecx, [zip.num_disks]
        jnz     @f
        mov     ecx, [inStream]
        push    0
        push    0
        push    0
        push    ecx
        call    [seek]
        mov     eax, ecx
        ret     16
@@:
        push    esi edi
        mov     esi, [esp+8+12]
        mov     edi, zip.disk_name
        mov     [esp+8+12], edi
        mov     ecx, 1024
@@:
        lodsb
        stosb
        test    al, al
        loopnz  @b
        jnz     .ret0
        cmp     ecx, 4
        jb      .ret0
        dec     edi
        mov     al, '.'
        stosb
        cmp     edx, 100
        jae     .big
        mov     al, 'z'
        stosb
        mov     eax, edx
        aam
        add     ax, '00'
        xchg    al, ah
        stosw
        mov     byte [edi], 0
.nameok:
        pop     edi esi
        push    dword [esp+16]
        push    dword [esp+16]
        push    dword [esp+16]
        push    dword [esp+16]
        call    [open2]
        mov     [zip.cur_handle], eax
        ret     16
.ret0:
        mov     byte [edi-1], 0
        pop     edi esi
        xor     eax, eax
        ret     16
.big:
        cmp     ecx, 12
        jb      .ret0
        push    10
        pop     ecx
        push    -'0'
        mov     eax, edx
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     al, '0'
        jz      .nameok
        stosb
        jmp     @b

close_zip:
        mov     esi, [esp+4]
        push    [esi+handle_zip.host]
        call    [close]
        mov     ecx, [esi+handle_zip.names_buf]
        call    [pgfree]
        mov     ecx, [esi+handle_zip.cur_handle]
        jecxz   @f
        push    ecx
        call    [close]
@@:
        mov     ecx, esi
        call    [pgfree]
        ret     4

; ebp=hPlugin, eax->item, edi->info
getattr_zip:
        cmp     [eax+file_in_zip.bPseudoFolder], 0
        jnz     .pseudo
        mov     esi, eax
        mov     eax, [eax+file_in_zip.attr]
        stosd
        xor     eax, eax
        stosd
        stosd
        stosd
        stosd
        stosd
        movzx   eax, word [esi+file_in_zip.time]
        mov     ecx, eax
        and     al, 0x1F
        shr     ecx, 5
        add     eax, eax
        stosb
        mov     eax, ecx
        and     al, 0x3F
        shr     ecx, 6
        stosb
        mov     eax, ecx
        stosw
        movzx   eax, word [esi+file_in_zip.date]
        mov     ecx, eax
        and     al, 0x1F
        shr     ecx, 5
        stosb
        mov     eax, ecx
        and     al, 0xF
        shr     ecx, 4
        stosb
        lea     eax, [ecx+1980]
        stosw
        mov     eax, dword [esi+file_in_zip.uncompressed_size]
        stosd
        mov     eax, dword [esi+file_in_zip.uncompressed_size+4]
        stosd
        ret
.pseudo:
        push    0x10    ; attributes: folder
        pop     eax
        stosd
        xor     eax, eax
        stosd
        lea     esi, [ebp+handle_zip.host_datetime]
        push    6
        pop     ecx
        rep     movsd
        stosd
        stosd
        ret

virtual at 0
file_handle_zip:
.type   dd      ?
.item   dd      ?
.base   dd      ?
.start_disk     dd      ?
.start          dq      ?
.pos            dq      ?
.posc           dq      ?
.decoder        dd      ?
.cur_disk       dd      ?
.cur_disk_start dq      ?
.bError         db      ?
.bPassProtected db      ?
.bPassInited    db      ?
                rb      1
.keys           rd      3
.saved_keys     rd      3
.size = $
end virtual

iglobal
handle_table_zip        dd      handle_table_zip, handle_table_zip
endg

; ebp=hPlugin, eax->item, edi=mode
open_file_zip:
        mov     [_esp], esp
        mov     [_ebp], ebp
        mov     [error_proc], .error
        mov     [clear_proc], .clear
        mov     ecx, [ebp+handle_zip.password_len]
        inc     ecx
        setnz   [bPasswordDefined]
        jz      @f
        dec     ecx
        mov     [password_size], ecx
        lea     esi, [ebp+handle_zip.password]
        mov     edi, password_ansi
        rep     movsb
@@:
        xor     edi, edi
        mov     [hOut], edi
        mov     [hOut.allocated], edi
        mov     ebx, eax
        movzx   eax, [ebx+file_in_zip.method]
        test    eax, eax
        jz      .common
.notcopy:
        cmp     eax, 8
        jz      .deflate
        cmp     eax, 9
        jz      .deflate
        push    -'0'
        push    10
        pop     ecx
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
        mov     edi, aUnknownMethod.z
        mov     al, ' '
        stosb
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
        push    ContinueBtn
        push    1
        push    aUnknownMethod_ptr
        push    1
        call    [SayErr]
        xor     eax, eax
.ret1:
        ret
.deflate:
        mov     ecx, 0x12000
        call    [pgalloc]
        test    eax, eax
        jz      .ret1
        mov     [hOut], eax
        mov     ecx, dword [ebx+file_in_zip.compressed_size]
        mov     dword [eax+streamInfo.fullSize], ecx
        mov     ecx, dword [ebx+file_in_zip.compressed_size+4]
        mov     dword [eax+streamInfo.fullSize+4], ecx
        mov     [eax+streamInfo.fillBuf], zipin.fillBuf
        mov     [eax+streamInfo.bufSize], 0x2000 - (streamInfo.size+4) - deflate_decoder.size
        and     [eax+streamInfo.bufDataLen], 0
        lea     edx, [eax+streamInfo.size+4]
        mov     [eax+streamInfo.bufPtr], edx
        add     edx, [eax+streamInfo.bufSize]
        mov     ecx, dword [ebx+file_in_zip.uncompressed_size]
        mov     dword [edx+streamInfo.fullSize], ecx
        mov     ecx, dword [ebx+file_in_zip.uncompressed_size+4]
        mov     dword [edx+streamInfo.fullSize+4], ecx
        mov     [edx+streamInfo.bufSize], 0x10000
        and     [edx+streamInfo.bufDataLen], 0
        mov     [edx+streamInfo.size], eax
        lea     eax, [edx+deflate_decoder.size]
        mov     [edx+streamInfo.bufPtr], eax
        push    ebp
        mov     ebp, edx
        call    deflate_init_decoder
        mov     edi, ebp
        pop     ebp
        cmp     [ebx+file_in_zip.method], 9
        setz    [edi+deflate_decoder.bDeflate64]
.common:
        mov     esi, handle_table_zip
        push    file_handle_zip.size
        pop     ecx
        call    alloc_handle
        test    eax, eax
        jz      return.clear
        mov     edx, [hOut]
        mov     [edx+streamInfo.size], eax
        mov     [eax+file_handle_zip.type], type_zip
        xor     edx, edx
        mov     dword [eax+file_handle_zip.pos], edx
        mov     dword [eax+file_handle_zip.pos+4], edx
        mov     dword [eax+file_handle_zip.posc], edx
        mov     dword [eax+file_handle_zip.posc+4], edx
        mov     [eax+file_handle_zip.item], ebx
        mov     [eax+file_handle_zip.base], ebp
        mov     [eax+file_handle_zip.decoder], edi
        mov     [eax+file_handle_zip.bError], dl
        mov     [eax+file_handle_zip.bPassInited], dl
        test    [ebx+file_in_zip.flags], 1
        setnz   [eax+file_handle_zip.bPassProtected]
        mov     ecx, [ebx+file_in_zip.disk]
        mov     [eax+file_handle_zip.start_disk], ecx
        mov     [eax+file_handle_zip.cur_disk], ecx
        mov     ecx, dword [ebx+file_in_zip.local_hdr]
        mov     dword [eax+file_handle_zip.start], ecx
        mov     ecx, dword [ebx+file_in_zip.local_hdr+4]
        mov     dword [eax+file_handle_zip.start+4], ecx
        mov     dword [eax+file_handle_zip.cur_disk_start], edx
        mov     dword [eax+file_handle_zip.cur_disk_start+4], edx
        mov     [hOut.allocated], eax
        mov     [hOut], edx
        push    ebx
        mov     ebx, eax
        push    zip.file_header.sz_local
        pop     ecx
        mov     edi, zip.file_header+2
        call    zip.read
        cmp     dword [edi-zip.file_header.sz_local], 0x04034B50
        jnz     return.err
        movzx   ecx, word [edi-4]
        movzx   eax, word [edi-2]
        lea     esi, [ecx+eax+zip.file_header.sz_local]
        xor     edi, edi
        call    zip.setpos
        call    zip.update_start
        cmp     [ebx+file_handle_zip.bPassProtected], 0
        jz      .nopass
.redo_pass:
        call    query_password
        jz      return.clear
        call    zip.init_keys
        push    12
        pop     ecx
        mov     edi, buffer
        call    zip.read
        mov     eax, [ebx+file_handle_zip.item]
        mov     al, byte [eax+file_in_zip.crc+3]
        cmp     [edi-1], al
        jz      .passok
        push    CancelPassBtn
        push    2
        push    aArchiveDataErrorPass_ptr
        push    1
        call    [SayErr]
        cmp     al, 1
        jnz     return.clear
        xor     eax, eax
        mov     dword [ebx+file_handle_zip.posc], eax
        mov     dword [ebx+file_handle_zip.posc+4], eax
        mov     dword [ebx+file_handle_zip.cur_disk_start], eax
        mov     dword [ebx+file_handle_zip.cur_disk_start+4], eax
        mov     eax, [ebx+file_handle_zip.start_disk]
        mov     [ebx+file_handle_zip.cur_disk], eax
        mov     [bPasswordDefined], 0
        jmp     .redo_pass
.passok:
        mov     ecx, [password_size]
        mov     [ebp+handle_zip.password_len], ecx
        mov     esi, password_ansi
        lea     edi, [ebp+handle_zip.password]
        rep     movsb
        lea     esi, [ebx+file_handle_zip.keys]
        lea     edi, [ebx+file_handle_zip.saved_keys]
        movsd
        movsd
        movsd
        mov     esi, 12
        xor     edi, edi
        call    zip.update_start
.nopass:
        pop     ebx
        mov     eax, [hOut.allocated]
        cmp     dword [ebx+file_in_zip.uncompressed_size+4], 0
        jnz     .ret
        mov     ecx, dword [ebx+file_in_zip.uncompressed_size]
if defined LIMIT_FOR_MEM_STREAM
        cmp     ecx, LIMIT_FOR_MEM_STREAM
        ja      .ret
end if
        push    eax ecx
        add     ecx, 0x3FF
        shr     ecx, 10         ; get size in Kb
        call    [getfreemem]
        shr     eax, 2
        cmp     ecx, eax
        pop     ecx eax
        ja      .ret
; create memory stream and unpack to memory
        mov     ebx, eax
        add     ecx, mem_stream.buf
        call    [pgalloc]
        sub     ecx, mem_stream.buf
        test    eax, eax
        jz      return.clear
        mov     edi, eax
        mov     [hOut], eax
        xor     eax, eax        ; type_mem_stream
        stosd                   ; mem_stream.type
        mov     eax, ecx
        stosd                   ; mem_stream.size
        xor     eax, eax
        stosd                   ; mem_stream.pos
        push    ecx edi
        push    ecx
        push    edi
        push    ebx
        call    read_zip
        pop     esi ecx
        mov     [_esp], esp
        mov     [_ebp], ebp
        mov     [error_proc], .error
        mov     [clear_proc], .clear
        cmp     eax, -1
        jz      .clear
;        cmp     eax, ecx
;        jnz     .error
;        call    crc
;        mov     edx, [ebx+file_handle_zip.item]
;        cmp     eax, [edx+file_in_zip.crc]
;        jnz     .error
        call    .1
        mov     eax, [hOut]
.ret:
        ret

.error:
        push    ContinueBtn
        push    1
        push    aArchiveDataError_ptr
        push    1
        call    [SayErr]
.clear:
        mov     ecx, [hOut]
        call    [pgfree]
.1:
        mov     ebx, [hOut.allocated]
        test    ebx, ebx
        jz      @f
        push    ebx
        call    close_file_zip
@@:
        xor     eax, eax
        ret

close_file_zip:
        mov     ecx, [ebx+file_handle_zip.decoder]
        jecxz   @f
        mov     ecx, [ecx+streamInfo.size]
        call    [pgfree]
@@:
        mov     esi, ebx
        call    free_handle
        ret     4

zip.init_keys:
        mov     [ebx+file_handle_zip.keys+0], 305419896
        mov     [ebx+file_handle_zip.keys+4], 591751049
        mov     [ebx+file_handle_zip.keys+8], 878082192
        mov     ecx, [password_size]
        jecxz   .empty
        mov     esi, password_ansi
        xor     eax, eax
@@:
        lodsb
        call    zip.update_keys
        loop    @b
.empty:
        mov     [ebx+file_handle_zip.bPassInited], 1
        ret

zip.update_keys:
        mov     dl, byte [ebx+file_handle_zip.keys]
        xor     dl, al
        movzx   edx, dl
        mov     eax, [ebx+file_handle_zip.keys]
        shr     eax, 8
        xor     eax, [crc_table+edx*4]
        mov     [ebx+file_handle_zip.keys], eax
        movzx   eax, al
        add     eax, [ebx+file_handle_zip.keys+4]
        imul    eax, 134775813
        inc     eax
        mov     [ebx+file_handle_zip.keys+4], eax
        shr     eax, 24
        mov     edx, [ebx+file_handle_zip.keys+8]
        xor     al, dl
        shr     edx, 8
        xor     edx, [crc_table+eax*4]
        mov     [ebx+file_handle_zip.keys+8], edx
        ret

zip.decrypt_byte:
; out: ah = result
        mov     eax, [ebx+file_handle_zip.keys+8]
        or      eax, 2
        mov     edx, eax
        xor     edx, 1
        mul     edx
        ret

zip.get_cur_handle:
        mov     edx, [ebp+handle_zip.host]
        mov     eax, [ebp+handle_zip.num_disks]
        dec     eax
        cmp     eax, [ebx+file_handle_zip.cur_disk]
        jz      .handleok
        inc     eax
        mov     [zip.num_disks], eax
        mov     edx, [ebp+handle_zip.cur_handle]
        mov     eax, [ebp+handle_zip.cur_disk]
        cmp     eax, [ebx+file_handle_zip.cur_disk]
        jz      .handleok
        mov     [zip.cur_handle], edx
        push    ecx
        mov     ecx, [ebx+file_handle_zip.cur_disk]
        push    1
        push    [ebp+handle_zip.names_buf]
        push    [ebp+handle_zip.host_hPlugin]
        push    [ebp+handle_zip.host_idPlugin]
        call    zip.open_splitted
        pop     ecx
        mov     edx, eax
        test    eax, eax
        jnz     @f
        push    zip.disk_name
        push    aCannotOpenFile
        mov     eax, esp
        push    ContinueBtn
        push    1
        push    eax
        push    2
        call    [SayErr]
        jmp     return.clear
@@:
        mov     [ebp+handle_zip.cur_handle], eax
        mov     eax, [ebx+file_handle_zip.cur_disk]
        mov     [ebp+handle_zip.cur_disk], eax
.handleok:
        ret

zip.read:
; in: ebx=hFile, edi->buf, ecx=size
        push    ebp edi
        mov     ebp, [ebx+file_handle_zip.base]
.s:
        call    zip.get_cur_handle
        mov     eax, dword [ebx+file_handle_zip.posc]
        mov     esi, dword [ebx+file_handle_zip.posc+4]
        sub     eax, dword [ebx+file_handle_zip.cur_disk_start]
        sbb     esi, dword [ebx+file_handle_zip.cur_disk_start+4]
        push    eax
        mov     eax, dword [ebx+file_handle_zip.cur_disk]
        cmp     eax, dword [ebx+file_handle_zip.start_disk]
        pop     eax
        jnz     @f
        add     eax, dword [ebx+file_handle_zip.start]
        adc     esi, dword [ebx+file_handle_zip.start+4]
@@:
        push    esi
        push    eax
        push    0
        push    edx
        call    [seek]
        push    ecx
        push    edi
        push    edx
        call    [read]
        cmp     eax, -1
        jz      .error
        add     edi, eax
        add     dword [ebx+file_handle_zip.posc], eax
        adc     dword [ebx+file_handle_zip.posc+4], 0
        sub     ecx, eax
        jz      .ok
        inc     dword [ebx+file_handle_zip.cur_disk]
        mov     eax, dword [ebx+file_handle_zip.posc]
        mov     dword [ebx+file_handle_zip.cur_disk_start], eax
        mov     eax, dword [ebx+file_handle_zip.posc+4]
        mov     dword [ebx+file_handle_zip.cur_disk_start+4], eax
        jmp     .s
.ok:
        cmp     [ebx+file_handle_zip.bPassInited], 0
        jnz     .decrypt
        pop     eax ebp
        ret
.decrypt:
        mov     ecx, edi
        pop     edi
        sub     ecx, edi
        jz      .nodata
@@:
        call    zip.decrypt_byte
        xor     [edi], ah
        mov     al, [edi]
        inc     edi
        call    zip.update_keys
        loop    @b
.nodata:
        pop     ebp
        ret
.error:
        push    ContinueBtn
        push    1
        push    aReadError_ptr
        push    1
        call    [SayErr]
        jmp     return.clear

zip.setpos:
; in: ebx=hFile, edi:esi=newpos
        cmp     [ebx+file_handle_zip.bPassInited], 0
        jnz     .pass
        push    ebp
        mov     ebp, [ebx+file_handle_zip.base]
        push    edi esi
        sub     esi, dword [ebx+file_handle_zip.cur_disk_start]
        sbb     edi, dword [ebx+file_handle_zip.cur_disk_start+4]
        jae     .fwd
        mov     eax, [ebx+file_handle_zip.start_disk]
        mov     [ebx+file_handle_zip.cur_disk], eax
        and     dword [ebx+file_handle_zip.cur_disk_start], 0
        and     dword [ebx+file_handle_zip.cur_disk_start+4], 0
        pop     esi edi
        push    edi esi
.fwd:
        call    zip.get_cur_handle
        push    edx
        call    [filesize]
        push    eax
        mov     eax, [ebx+file_handle_zip.cur_disk]
        cmp     eax, [ebx+file_handle_zip.start_disk]
        pop     eax
        jnz     @f
        sub     eax, dword [ebx+file_handle_zip.start]
        sbb     edx, dword [ebx+file_handle_zip.start+4]
@@:
        sub     esi, eax
        sbb     edi, edx
        jb      @f
        inc     [ebx+file_handle_zip.cur_disk]
        add     dword [ebx+file_handle_zip.cur_disk_start], eax
        adc     dword [ebx+file_handle_zip.cur_disk_start+4], edx
        jmp     .fwd
@@:
        pop     esi edi
        mov     dword [ebx+file_handle_zip.posc], esi
        mov     dword [ebx+file_handle_zip.posc+4], edi
        pop     ebp
        ret
.pass:
        push    edi esi
        sub     esi, dword [ebx+file_handle_zip.posc]
        sbb     edi, dword [ebx+file_handle_zip.posc+4]
        jae     .pfwd
        lea     esi, [ebx+file_handle_zip.saved_keys]
        lea     edi, [ebx+file_handle_zip.keys]
        movsd
        movsd
        movsd
        and     dword [ebx+file_handle_zip.posc], 0
        and     dword [ebx+file_handle_zip.posc+4], 0
        mov     eax, [ebx+file_handle_zip.start_disk]
        mov     [ebx+file_handle_zip.cur_disk], eax
        and     dword [ebx+file_handle_zip.cur_disk_start], 0
        and     dword [ebx+file_handle_zip.cur_disk_start+4], 0
        pop     esi edi
        push    edi esi
.pfwd:
        push    edi esi
        mov     ecx, 1024
        sub     esi, ecx
        sbb     edi, 0
        jae     @f
        add     ecx, esi
@@:
        push    ecx
        mov     edi, buffer
        call    zip.read
        pop     ecx
        pop     esi edi
        sub     esi, ecx
        sbb     edi, 0
        mov     eax, esi
        or      eax, edi
        jnz     .pfwd
        pop     esi edi
        ret

zip.update_start:
; edi=0, esi=pos
        sub     esi, dword [ebx+file_handle_zip.cur_disk_start]
        mov     dword [ebx+file_handle_zip.cur_disk_start], edi
        add     dword [ebx+file_handle_zip.start], esi
        adc     dword [ebx+file_handle_zip.start+4], edi
        mov     dword [ebx+file_handle_zip.posc], edi
        mov     dword [ebx+file_handle_zip.posc+4], edi
        mov     eax, [ebx+file_handle_zip.cur_disk]
        cmp     eax, [ebx+file_handle_zip.start_disk]
        jz      @f
        mov     [ebx+file_handle_zip.start_disk], eax
        mov     dword [ebx+file_handle_zip.start], esi
        mov     dword [ebx+file_handle_zip.start+4], edi
@@:
        ret

; unsigned __stdcall read(HANDLE hFile, void* buf, unsigned size);
read_zip:
        mov     [_esp], esp
        mov     [_ebp], ebp
        mov     [error_proc], .error
        mov     [clear_proc], .clear
        cmp     [ebx+file_handle_zip.bError], 0
        jnz     .reterr
        mov     edi, [esp+8]
        mov     ecx, [esp+12]
        mov     esi, [ebx+file_handle_zip.item]
        mov     edx, dword [esi+file_in_zip.uncompressed_size]
        mov     esi, dword [esi+file_in_zip.uncompressed_size+4]
        sub     edx, dword [ebx+file_handle_zip.pos]
        sbb     esi, dword [ebx+file_handle_zip.pos+4]
        jnz     @f
        cmp     edx, ecx
        jae     @f
        mov     ecx, edx
@@:
        jecxz   .done
        mov     eax, [ebx+file_handle_zip.decoder]
        test    eax, eax
        jnz     .notcopy
        call    zip.read
        jmp     .done
.notcopy:
        mov     dword [eax+streamInfo.fullSize], ecx
        and     dword [eax+streamInfo.fullSize+4], 0
        call    read_7z_to_buf
.done:
        sub     edi, [esp+8]
        mov     eax, edi
        add     dword [ebx+file_handle_zip.pos], eax
        adc     dword [ebx+file_handle_zip.pos+4], 0
.ret:
        ret     12

.error:
        push    ContinueBtn
        push    1
        push    aArchiveDataError_ptr
        push    1
        call    [SayErr]
.clear:
        mov     ebx, [esp+4]
        mov     [ebx+file_handle_zip.bError], 1
.reterr:
        or      eax, -1
        ret     12

zipin.fillBuf:
        mov     ebx, [eax+streamInfo.size]
        call    zip.read
        popad
        ret

; void __stdcall setpos(HANDLE hFile, __int64 pos);
setpos_zip:
        mov     [_esp], esp
        mov     [_ebp], ebp
        mov     [error_proc], read_zip.error
        mov     [clear_proc], read_zip.clear
        cmp     [ebx+file_handle_zip.decoder], 0
        jnz     .notcopy
        mov     esi, [esp+8]
        mov     edi, [esp+12]
        call    zip.setpos
        mov     dword [ebx+file_handle_zip.pos], esi
        mov     dword [ebx+file_handle_zip.pos+4], edi
        ret     12
.notcopy:
        cmp     [ebx+file_handle_zip.bError], 0
        jnz     .backward
        mov     ecx, [esp+8]
        mov     edx, [esp+12]
        sub     ecx, dword [ebx+file_handle_zip.pos]
        sbb     edx, dword [ebx+file_handle_zip.pos+4]
        jb      .backward
        mov     eax, [ebx+file_handle_zip.decoder]
        mov     dword [eax+streamInfo.fullSize], ecx
        mov     dword [eax+streamInfo.fullSize+4], edx
        call    skip_7z
        add     dword [ebx+file_handle_zip.pos], ecx
        adc     dword [ebx+file_handle_zip.pos+4], edx
        ret     12
.backward:
        xor     eax, eax
        mov     [ebx+file_handle_zip.bError], al
        mov     dword [ebx+file_handle_zip.pos], eax
        mov     dword [ebx+file_handle_zip.pos+4], eax
        mov     dword [ebx+file_handle_zip.posc], eax
        mov     dword [ebx+file_handle_zip.posc+4], eax
        mov     dword [ebx+file_handle_zip.cur_disk_start], eax
        mov     dword [ebx+file_handle_zip.cur_disk_start+4], eax
        lea     esi, [ebx+file_handle_zip.saved_keys]
        lea     edi, [ebx+file_handle_zip.keys]
        movsd
        movsd
        movsd
        mov     eax, [ebx+file_handle_zip.start_disk]
        mov     [ebx+file_handle_zip.cur_disk], eax
        mov     eax, [ebx+file_handle_zip.decoder]
        mov     eax, [eax+deflate_decoder.inStream]
        mov     esi, [ebx+file_handle_zip.item]
        mov     ecx, dword [esi+file_in_zip.compressed_size]
        mov     dword [eax+streamInfo.fullSize], ecx
        mov     ecx, dword [esi+file_in_zip.compressed_size+4]
        mov     dword [eax+streamInfo.fullSize+4], ecx
        and     [eax+streamInfo.bufDataLen], 0
        lea     ebp, [eax+streamInfo.size+4]
        add     ebp, [eax+streamInfo.bufSize]
        and     [ebp+streamInfo.bufDataLen], 0
        call    deflate_init_decoder
        cmp     [esi+file_in_zip.method], 9
        setz    [ebp+deflate_decoder.bDeflate64]
        mov     eax, [esp+8]
        mov     dword [ebp+streamInfo.fullSize], eax
        mov     eax, [esp+12]
        mov     dword [ebp+streamInfo.fullSize+4], eax
        mov     eax, ebp
        call    skip_7z
        mov     eax, [esp+8]
        mov     dword [ebx+file_handle_zip.pos], eax
        mov     eax, [esp+12]
        mov     dword [ebx+file_handle_zip.pos+4], eax
        ret     12
