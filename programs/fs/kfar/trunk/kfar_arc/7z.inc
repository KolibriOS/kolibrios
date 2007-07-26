; Loading of 7z archives was ported from 7z sources (file 7zip\Archive\7z\7zIn.cpp).
; The version 7-Zip 4.42 was used.
; 7-Zip is copyright (C) 1999-2006 Igor Pavlov.
; Assembler version as KFar plugin has been written by diamond, 2007.

virtual at 0
file_in_7z:
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
.attr           dd      ?
.FileCRC        dd      ?
.startPos       dq      ?
.CreationTime   dq      ?
.LastAccessTime dq      ?
.LastWriteTime  dq      ?
.UnPackSize     dq      ?
.folder         dd      ?
.folder_index   dd      ?
.folderStart    dq      ?
.bAttrDefined           db      ?
.bStartPosDefined       db      ?
.bCRCDefined            db      ?
.bCreationTimeDefined   db      ?
.bLastAccessTimeDefined db      ?
.bLastWriteTimeDefined  db      ?
.bHasStream             db      ?
.bAnti                  db      ?
.stamp          dd      ?
.size = $
end virtual

virtual at 0
handle_7z:
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
; streams info
.numPackStreams dd      ?
.packSizes      dd      ?
.numFolders     dd      ?
.unpackCRCsDefined dd   ?
.unpackCRCs     dd      ?
.folders        dd      ?
.last_folder    dd      ?
.last_pos       dq      ?
.last_context   dd      ?
.last_main_stream dd    ?
.password_len   dd      ?       ; -1 if no password defined; in characters
.password       rw      password_maxlen
.basesize = $
; .size = .basesize + .NumFiles * file_in_7z.size
end virtual

iglobal
align 4
empty_7z_handle:
        dd      type_7z
        dd      0,0,0,0,0,0
endg

kEnd = 0
kHeader = 1
kArchiveProperties = 2
kAdditionalStreamsInfo = 3
kMainStreamsInfo = 4
kFilesInfo = 5
kPackInfo = 6
kUnPackInfo = 7
kSubStreamsInfo = 8
kSize = 9
kCRC = 10
kFolder = 11
kCodersUnPackSize = 12
kNumUnPackStream = 13
kEmptyStream = 14
kEmptyFile = 15
kAnti = 16
kName = 17
kCreationTime = 18
kLastAccessTime = 19
kLastWriteTime = 20
kWinAttributes = 21
kComment = 22
kEncodedHeader = 23
kStartPos = 24

_7z.ReadByte:
        cmp     edi, [bufend]
        jae     return.err
        mov     al, [edi]
        inc     edi
        ret
_7z.ReadWord:
        add     edi, 2
        cmp     edi, [bufend]
        ja      return.err
        mov     ax, [edi-2]
        ret
_7z.ReadDword:
        add     edi, 4
        cmp     edi, [bufend]
        ja      return.err
        mov     eax, [edi-4]
        ret

_7z.SkipSize:
        test    edx, edx
        jnz     return.err
        add     edi, eax
        jc      return.err
        cmp     edi, [bufend]
        ja      return.err
        ret

_7z.ReadNumber:
        xor     eax, eax
        push    eax
        push    eax
        call    _7z.ReadByte
        push    ecx
        xor     ecx, ecx
@@:
        add     al, al
        jnc     @f
        inc     ecx
        jmp     @b
@@:
        shr     eax, cl
        shr     eax, 1
        jz      @f
        mov     [esp+4+ecx], al
@@:
        jecxz   .ret
        push    edx
        xor     edx, edx
@@:
        call    _7z.ReadByte
        mov     [esp+8+edx], al
        inc     edx
        loop    @b
        pop     edx
.ret:
        pop     ecx
        pop     eax
        pop     edx
        ret

_7z.ReadNum:
        push    edx
        call    _7z.ReadNumber
        test    edx, edx
        jnz     return.err
        test    eax, eax
        js      return.err
        pop     edx
        ret

open_7z:
; CInArchive::ReadDatabase()
        cmp     byte [esi+6], 0         ; major version must be 0
                                        ; minor version does not be checked ([byte +7]=2 for now)
        jz      @f
        xor     eax, eax
        ret
@@:
        and     [hOut], 0
        and     [_7z.FreeSubStreams], 0
        and     [_7z.tmpVector], 0
        push    esi
        add     esi, 12
        mov     ecx, 20
        call    crc
        pop     esi
        cmp     eax, [esi+8]
        jnz     .header_crc_error
        cmp     dword [esi+24], 0
        jnz     .header_error
        mov     edi, buffer
        mov     ecx, [esi+20]
        mov     [bufsize], ecx
        test    ecx, ecx
        jnz     .notempty
        mov     eax, empty_7z_handle
        push    ebp
        call    [close]
        ret
.notempty:
        cmp     ecx, 1024
        jbe     @f
        call    [pgalloc]
        test    eax, eax
        jz      .reterr
        mov     edi, eax
@@:
        mov     [bufptr], edi
        lea     eax, [edi+ecx]
        mov     [bufend], eax
        mov     eax, [esi+12]
        mov     edx, [esi+16]
        add     eax, 20h        ; end of main header
        adc     edx, 0
        push    edx
        push    eax
        push    0
        push    ebp
        call    [seek]
        push    ecx
        push    edi
        push    ebp
        call    [read]
        cmp     eax, ecx
        jnz     .header_error
        push    esi
        mov     esi, edi
        call    crc
        pop     esi
        cmp     eax, [esi+28]
        jz      .header_ok
.header_crc_error:
.header_error:
        push    ContinueBtn
        push    1
        push    HeaderError_ptr
        push    1
        call    [SayErr]
.clear:
        mov     ecx, [_7z.tmpVector]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [hOut]
        jecxz   @f
        call    _7z.CloseHandle
@@:
        mov     ebx, [_7z.dataVectorSize]
        test    ebx, ebx
        jz      @f
        call    _7z.ReadAndDecodePackedStreams.free2
@@:
        mov     ecx, [_7z.FreeSubStreams]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [bufptr]
        cmp     ecx, buffer
        jz      .reterr
        call    [pgfree]
.reterr:
        or      eax, -1
        ret
.header_ok:
        mov     [_esp], esp
        mov     [inStream], ebp
.loop:
        mov     [error_proc], .header_error
        mov     [clear_proc], .clear
        and     [_7z.dataVectorSize], 0
        call    _7z.ReadNum
        cmp     eax, kHeader
        jz      .found_header
        cmp     eax, kEncodedHeader
        jnz     .header_error
        call    _7z.ReadAndDecodePackedStreams
        cmp     [_7z.StreamsInfo.numFolders], 0
        jz      .free_empty
        cmp     [_7z.StreamsInfo.numFolders], 1
        jnz     return.err
        mov     ecx, [bufptr]
        cmp     ecx, buffer
        jz      @f
        call    [pgfree]
@@:
        mov     edi, [_7z.unpbuf+4]
        mov     [bufptr], edi
        mov     ecx, [_7z.unpbuf]
        add     ecx, edi
        mov     [bufend], ecx
        mov     [error_proc], .header_error
        mov     [clear_proc], .clear
        and     [_7z.dataVectorSize], 0
        call    _7z.ReadNum
        cmp     eax, kHeader
        jz      .found_header
        jmp     return.err
.free_empty:
        mov     ecx, [bufptr]
        cmp     ecx, buffer
        jz      @f
        call    [pgfree]
@@:
        push    [inStream]
        call    [close]
        mov     eax, empty_7z_handle
        ret
.found_header:
        call    _7z.ReadNum
        cmp     eax, kArchiveProperties
        jnz     .no_archive_props
@@:
        call    _7z.ReadNum
        test    eax, eax
        jz      @f
        call    _7z.SkipData
        jmp     @b
@@:
        call    _7z.ReadNum
.no_archive_props:
        cmp     eax, kAdditionalStreamsInfo
        jnz     .no_additional_streams
        call    _7z.ReadAndDecodePackedStreams
        mov     eax, [_7z.unpacked]
        mov     [_7z.dataVector], eax
        mov     eax, [_7z.StreamsInfo.numFolders]
        mov     [_7z.dataVectorSize], eax
        mov     [error_proc], .header_error
        mov     [clear_proc], .clear
        call    _7z.ReadNum
.no_additional_streams:
        cmp     eax, kMainStreamsInfo
        jnz     return.err
        call    _7z.ReadStreamsInfo
        call    _7z.ReadNum
        test    eax, eax
        jnz     .notempty2
.empty2:
        mov     ebx, [_7z.dataVectorSize]
        test    ebx, ebx
        jz      @f
        call    _7z.ReadAndDecodePackedStreams.free2
@@:
        mov     ecx, [_7z.FreeSubStreams]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [bufptr]
        cmp     ecx, buffer
        jz      @f
        call    [pgfree]
@@:
        push    [inStream]
        call    [close]
        mov     eax, empty_7z_handle
        ret
.notempty2:
        cmp     eax, kFilesInfo
        jnz     return.err
        call    _7z.ReadNum
        test    eax, eax
        jz      .empty2
        mov     ebx, eax        ; number of files in archive
        imul    ecx, ebx, file_in_7z.size
        add     ecx, handle_7z.basesize
        mov     [hOut.allocated], ecx
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     [hOut], eax
        mov     [eax+handle_7z.type], type_7z
        mov     [eax+handle_7z.NumFiles], ebx
        push    edi
        lea     edi, [eax+handle_7z.numPackStreams]
        mov     esi, _7z.StreamsInfo
        mov     ecx, 6
        rep     movsd
        or      [eax+handle_7z.password_len], -1
        or      [eax+handle_7z.last_folder], -1
        mov     [eax+handle_7z.last_context], ecx
        mov     [eax+handle_7z.names_buf], ecx
        mov     [eax+handle_7z.host], ecx
        cmp     [bPasswordDefined], cl
        jz      @f
        mov     ecx, [password_size]
        mov     [eax+handle_7z.password_len], ecx
        lea     edi, [eax+handle_7z.password]
        mov     esi, password_unicode
        shr     ecx, 1
        rep     movsd
        adc     ecx, ecx
        rep     movsw
@@:
        lea     edi, [eax+handle_7z.basesize]
        imul    ecx, ebx, file_in_7z.size/4
        xor     eax, eax
        rep     stosd
        pop     edi
        mov     eax, [_7z.StreamsInfo.SubStreamsSizes]
        mov     [_7z.FreeSubStreams], eax
        mov     [error_proc], .header_error
        mov     [clear_proc], .clear
        mov     ecx, ebx
        shl     ecx, 2
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        push    edi
        mov     edi, eax
        mov     [_7z.tmpVector], eax
        add     eax, ebx
        mov     [_7z.emptyStreams], eax
        add     eax, ebx
        mov     [_7z.emptyFiles], eax
        add     eax, ebx
        mov     [_7z.antiFiles], eax
        mov     ecx, ebx
        xor     eax, eax
        rep     stosd
        pop     edi
        mov     [_7z.numEmptyStreams], eax
.parse_header:
        call    _7z.ReadNum
        test    eax, eax
        jz      .parse_header_done
        mov     ecx, eax        ; ecx = type
        call    _7z.ReadNum     ; eax = size
        cmp     ecx, kName
        jz      .parse_names
        cmp     ecx, kWinAttributes
        jz      .parse_winAttr
        cmp     ecx, kStartPos
        jz      .parse_startPos
        cmp     ecx, kEmptyStream
        jz      .parse_emptyStream
        cmp     ecx, kEmptyFile
        jz      .parse_emptyFile
        cmp     ecx, kAnti
        jz      .parse_anti
        cmp     ecx, kCreationTime
        jz      .parse_time
        cmp     ecx, kLastWriteTime
        jz      .parse_time
        cmp     ecx, kLastAccessTime
        jz      .parse_time
        xor     edx, edx
        call    _7z.SkipSize
        jmp     .parse_header
.parse_names:
        push    eax
        call    _7z.SwitchStream
        pop     eax
        jz      @f
        mov     eax, [bufend]
        sub     eax, edi
@@:
        shr     eax, 1          ; size of ANSI string is a half of size of UNICODE string
        mov     ecx, eax
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     ebp, eax
        mov     esi, [hOut]
        mov     [esi+handle_7z.names_buf], eax
        add     eax, ecx
        push    eax     ; remember limit
        mov     ecx, ebx
        add     esi, handle_7z.basesize + file_in_7z.fullname
        mov     [bWasWarning], 0
.parse_names_loop:
        mov     [esi], ebp
        add     esi, file_in_7z.size
.parse_names_loop_int:
        cmp     ebp, [esp]
        jae     return.err
        call    _7z.ReadWord
; UNICODE char -> cp866 char
        cmp     ax, 0x80
        jb      .id
        cmp     ax, 0x401
        jz      .yo1
        cmp     ax, 0x451
        jz      .yo2
        cmp     ax, 0x410
        jb      .unk
        cmp     ax, 0x440
        jb      .rus1
        cmp     ax, 0x450
        jb      .rus2
.unk:
        cmp     [bWasWarning], 0
        jnz     @f
        push    ContinueBtn
        push    1
        push    aEncodingProblem_str_ptr
        push    3
        push    aEncodingProblem
        call    [Message]
        inc     [bWasWarning]
@@:
        mov     al, '_'
        jmp     .id
.yo1:
        mov     al, 'Ё'
        jmp     .id
.yo2:
        mov     al, 'ё'
        jmp     .id
.rus1:
; 0x410-0x43F -> 0x80-0xAF
        add     al, 0x70
        jmp     .id
.rus2:
; 0x440-0x44F -> 0xE0-0xEF
        add     al, 0xA0
.id:
        mov     [ebp], al
        inc     ebp
        test    al, al
        jnz     .parse_names_loop_int
        loop    .parse_names_loop
        pop     eax
        call    _7z.UnswitchStream
        jmp     .parse_header
.parse_winAttr:
        mov     edx, [_7z.tmpVector]
        push    ebx
        mov     ecx, ebx
        mov     ebx, edx
        call    _7z.ReadBoolVector2
        pop     ebx
        call    _7z.SwitchStream
        mov     esi, [hOut]
        add     esi, handle_7z.basesize
        mov     ecx, ebx
.winAttrLoop:
        mov     al, [edx]
        inc     edx
        mov     [esi+file_in_7z.bAttrDefined], al
        test    al, al
        jz      @f
        call    _7z.ReadDword
        mov     [esi+file_in_7z.attr], eax
@@:
        add     esi, file_in_7z.size
        loop    .winAttrLoop
        call    _7z.UnswitchStream
        jmp     .parse_header
.parse_startPos:
        mov     edx, [_7z.tmpVector]
        push    ebx
        mov     ecx, ebx
        mov     ebx, edx
        call    _7z.ReadBoolVector2
        pop     ebx
        call    _7z.SwitchStream
        mov     esi, [hOut]
        add     esi, handle_7z.basesize
        mov     ecx, ebx
.startPosLoop:
        mov     al, [edx]
        inc     edx
        mov     [esi+file_in_7z.bStartPosDefined], al
        test    al, al
        jz      @f
        call    _7z.ReadDword
        mov     dword [esi+file_in_7z.startPos], eax
        call    _7z.ReadDword
        mov     dword [esi+file_in_7z.startPos+4], eax
@@:
        add     esi, file_in_7z.size
        loop    .startPosLoop
        call    _7z.UnswitchStream
        jmp     .parse_header
.parse_emptyStream:
        mov     edx, [_7z.emptyStreams]
        push    ebx
        mov     ecx, ebx
        mov     ebx, edx
        call    _7z.ReadBoolVector
        pop     ebx
        and     [_7z.numEmptyStreams], 0
        mov     ecx, ebx
.emptyStrLoop:
        cmp     byte [edx], 0
        jz      @f
        inc     [_7z.numEmptyStreams]
@@:
        inc     edx
        loop    .emptyStrLoop
        jmp     .parse_header
.parse_emptyFile:
        push    ebx
        mov     ecx, [_7z.numEmptyStreams]
        mov     ebx, [_7z.emptyFiles]
        call    _7z.ReadBoolVector
        pop     ebx
        jmp     .parse_header
.parse_anti:
        push    ebx
        mov     ecx, [_7z.numEmptyStreams]
        mov     ebx, [_7z.antiFiles]
        call    _7z.ReadBoolVector
        pop     ebx
        jmp     .parse_header
.parse_time:
        push    ecx
        push    ebx
        mov     ecx, ebx
        mov     ebx, [_7z.tmpVector]
        call    _7z.ReadBoolVector2
        pop     ebx
        call    _7z.SwitchStream
        mov     esi, [hOut]
        add     esi, handle_7z.basesize
        xor     ecx, ecx
.timeLoop:
        mov     eax, [_7z.tmpVector]
        mov     al, [eax+ecx]
        pop     edx
        push    edx
        mov     [esi + file_in_7z.bCreationTimeDefined + edx - kCreationTime], al
        test    al, al
        jz      @f
        call    _7z.ReadDword
        push    eax
        call    _7z.ReadDword
        mov     edx, eax
        pop     eax
        push    edi
        mov     edi, [esp+4]
        lea     edi, [esi + file_in_7z.CreationTime + (edi-kCreationTime)*8]
        call    ntfs_datetime_to_bdfe
        pop     edi
@@:
        add     esi, file_in_7z.size
        inc     ecx
        cmp     ecx, ebx
        jb      .timeLoop
        call    _7z.UnswitchStream
        pop     eax
        jmp     .parse_header
.parse_header_done:
        xor     ecx, ecx        ; index in emptyFiles
        xor     edx, edx        ; index in subStreams
        xor     ebp, ebp        ; index in files
        mov     esi, [hOut]
        add     esi, handle_7z.basesize
.filesLoop:
        mov     eax, [_7z.emptyStreams]
        mov     al, [eax+ebp]
        xor     al, 1
        mov     [esi+file_in_7z.bHasStream], al
        jz      .nostream
        mov     [esi+file_in_7z.bIsDirectory], 0
        mov     [esi+file_in_7z.bAnti], 0
        mov     eax, [_7z.StreamsInfo.SubStreamsSizes]
        push    dword [eax+edx*8]
        pop     dword [esi+file_in_7z.UnPackSize]
        push    dword [eax+edx*8+4]
        pop     dword [esi+file_in_7z.UnPackSize+4]
        mov     eax, [_7z.StreamsInfo.SubStreamsCRCsDefined]
        mov     al, [eax+edx]
        mov     [esi+file_in_7z.bCRCDefined], al
        test    al, al
        jz      @f
        mov     eax, [_7z.StreamsInfo.SubStreamsCRCs]
        mov     eax, [eax+edx*4]
        mov     [esi+file_in_7z.FileCRC], eax
@@:
        inc     edx
        jmp     .filesCont
.nostream:
        mov     eax, [_7z.emptyFiles]
        cmp     byte [eax+ecx], 0
        setz    [esi+file_in_7z.bIsDirectory]
        mov     eax, [_7z.antiFiles]
        cmp     byte [eax+ecx], 0
        setnz   [esi+file_in_7z.bAnti]
        and     dword [esi+file_in_7z.UnPackSize], 0
        and     dword [esi+file_in_7z.UnPackSize+4], 0
        mov     [esi+file_in_7z.bCRCDefined], 0
        inc     ecx
.filesCont:
        inc     ebp
        add     esi, file_in_7z.size
        dec     ebx
        jnz     .filesLoop
        mov     ecx, [_7z.tmpVector]
        call    [pgfree]
        and     [_7z.tmpVector], 0
        mov     ebx, [_7z.dataVectorSize]
        test    ebx, ebx
        jz      @f
        call    _7z.ReadAndDecodePackedStreams.free2
        and     [_7z.dataVectorSize], 0
@@:
        mov     ecx, [_7z.FreeSubStreams]
        jecxz   @f
        call    [pgfree]
        and     [_7z.FreeSubStreams], 0
@@:
        mov     ecx, [bufptr]
        cmp     ecx, buffer
        jz      @f
        call    [pgfree]
        mov     [bufptr], buffer
@@:
        xor     esi, esi        ; index of folder
        mov     edi, [hOut]
        mov     ebx, [edi+handle_7z.NumFiles]
        add     edi, handle_7z.basesize ; pointer to fileinfo
        push    0               ; start index of packed stream for folder
.getfoldersloop0:
        xor     edx, edx        ; index in folder
        push    edx edx         ; position in folder
.getfoldersloop:
        dec     ebx
        js      .getfoldersdone
        cmp     [edi+file_in_7z.bHasStream], 0
        jnz     @f
        test    edx, edx
        jz      .nofolder
@@:
        test    edx, edx
        jnz     .notfirst
; skip empty folders
@@:
        mov     eax, [hOut]
        cmp     esi, [eax+handle_7z.numFolders]
        jae     return.err
        mov     eax, [eax+handle_7z.folders]
        mov     eax, [eax+esi*4]
        cmp     edx, [eax+_7z.StreamsInfo.numUnPackStreams]
        jb      @f
        inc     esi
        jmp     @b
@@:
.notfirst:
        mov     [edi+file_in_7z.folder], esi
        mov     [edi+file_in_7z.folder_index], edx
        mov     eax, [esp]
        mov     dword [edi+file_in_7z.folderStart], eax
        mov     eax, [esp+4]
        mov     dword [edi+file_in_7z.folderStart+4], eax
        mov     ecx, dword [edi+file_in_7z.UnPackSize]
        add     [esp], ecx
        mov     ecx, dword [edi+file_in_7z.UnPackSize+4]
        adc     [esp+4], ecx
        add     edi, file_in_7z.size
        inc     edx
        mov     eax, [hOut]
        mov     eax, [eax+handle_7z.folders]
        mov     eax, [eax+esi*4]
        cmp     edx, [eax+_7z.StreamsInfo.numUnPackStreams]
        jb      .getfoldersloop
        inc     esi
        pop     edx edx
        jmp     .getfoldersloop0
.nofolder:
        push    -1
        pop     eax
        mov     [edi+file_in_7z.folder], eax
        mov     [edi+file_in_7z.folder_index], eax
        mov     dword [edi+file_in_7z.folderStart], eax
        mov     dword [edi+file_in_7z.folderStart+4], eax
        add     edi, file_in_7z.size
        jmp     .getfoldersloop
.getfoldersdone:
        pop     eax eax eax
        mov     ebp, [hOut]
        mov     esi, [ebp+handle_7z.folders]
        xor     ebx, ebx
        cmp     ebx, [ebp+handle_7z.numFolders]
        jae     .getoffsdone
.getoffsloop:
        lodsd
        test    ebx, ebx
        jnz     .getoffs2
        mov     edx, dword [_7z.StreamsInfo.dataOffset]
        mov     dword [eax+_7z.StreamsInfo.packOffset], edx
        mov     edx, dword [_7z.StreamsInfo.dataOffset+4]
        mov     dword [eax+_7z.StreamsInfo.packOffset+4], edx
        jmp     .getoffscont
.getoffs2:
        mov     ecx, [esi-8]
        mov     edx, dword [ecx+_7z.StreamsInfo.packOffset]
        mov     dword [eax+_7z.StreamsInfo.packOffset], edx
        mov     edx, dword [ecx+_7z.StreamsInfo.packOffset+4]
        mov     dword [eax+_7z.StreamsInfo.packOffset+4], edx
        mov     edi, [ecx+_7z.StreamsInfo.startPackedStream]
        mov     ecx, [ecx+_7z.StreamsInfo.numPackedStreams]
        push    ecx
        add     ecx, edi
        cmp     ecx, [ebp+handle_7z.numPackStreams]
        ja      return.err
        pop     ecx
        shl     edi, 3
        add     edi, [ebp+handle_7z.packSizes]
@@:
        dec     ecx
        js      @f
        mov     edx, [edi]
        add     dword [eax+_7z.StreamsInfo.packOffset], edx
        mov     edx, [edi+4]
        adc     dword [eax+_7z.StreamsInfo.packOffset+4], edx
        add     edi, 8
        jmp     @b
@@:
.getoffscont:
        inc     ebx
        cmp     ebx, [ebp+handle_7z.numFolders]
        jb      .getoffsloop
.getoffsdone:
        mov     edx, [hOut]
        mov     ebx, [edx+handle_7z.NumFiles]
        lea     edi, [edx+handle_7z.root.subfolders]
        add     edx, handle_7z.basesize
        push    file_in_7z.size
        call    init_file_links
        mov     eax, [hOut]
        and     [eax+handle_7z.curdir], 0       ; set root directory
        mov     esi, [inStream]
        mov     [eax+handle_7z.host], esi
        lea     edi, [eax+handle_7z.host_datetime]
        mov     esi, [esp+16]
        add     esi, 8
        mov     ecx, 6
        rep     movsd
        ret

_7z.CloseHandle:
        mov     esi, ecx
        push    [esi+handle_7z.host]
        call    [close]
        mov     ecx, [esi+handle_7z.packSizes]
        call    [pgfree]
        mov     ecx, [esi+handle_7z.folders]
        call    [pgfree]
        mov     ecx, [esi+handle_7z.names_buf]
        call    [pgfree]
        mov     ecx, [esi+handle_7z.last_context]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, esi
        call    [pgfree]
        ret

close_7z:
        mov     ecx, [esp+4]
        cmp     ecx, empty_7z_handle
        jz      @f
        call    _7z.CloseHandle
@@:
        ret     4

uglobal
align 4
_7z.unpacked    dd      ?
_7z.unpbuf      rd      2*8
_7z.CurInStream dd      ?
_7z.dataVector  dd      ?
_7z.dataVectorSize dd   ?
_7z.FreeSubStreams dd   ?
_7z.tmpVector   dd      ?
_7z.emptyStreams dd     ?
_7z.numEmptyStreams dd  ?
_7z.emptyFiles  dd      ?
_7z.antiFiles   dd      ?
endg

_7z.ReadAndDecodePackedStreams:
        call    _7z.ReadStreamsInfo
        mov     ecx, [_7z.StreamsInfo.numFolders]
        test    ecx, ecx
        jz      .ret2
        push    ecx
        mov     ecx, [_7z.StreamsInfo.SubStreamsSizes]
        jecxz   @f
        call    [pgfree]
        and     [_7z.StreamsInfo.SubStreamsSizes], 0
@@:
        pop     ecx
        mov     eax, _7z.unpbuf
        cmp     ecx, 8
        jbe     .1
        shl     ecx, 3
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     esi, eax
@@:
        and     dword [esi], 0
        add     esi, 4
        sub     ecx, 4
        jnz     @b
.1:
        mov     [_7z.unpacked], eax
        mov     esi, eax
        xor     ecx, ecx
        mov     [_7z.CurInStream], ecx
        mov     [error_proc], .error
        mov     [clear_proc], .clear
.loop:
        mov     eax, [_7z.StreamsInfo.folders]
        mov     ebx, [eax+ecx*4]
        call    _7z.GetUnPackSize
        test    edx, edx
        jnz     return.err
        test    eax, eax
        js      return.err
        push    ecx
        mov     [esi], eax
        mov     [_7z.decode.outBufferSize], eax
        mov     ecx, eax
        call    [pgalloc]
        test    eax, eax
        jz      return.err
        mov     [esi+4], eax
        mov     [_7z.decode.outBufferPtr], eax
        push    esi
        call    _7z.decode
        pop     esi
        pop     ecx
        push    ecx
        mov     eax, [_7z.StreamsInfo.unpackCRCsDefined]
        cmp     byte [eax+ecx], 0
        jz      @f
        push    esi
        mov     ecx, [esi]
        mov     esi, [esi+4]
        call    crc
        pop     esi
        pop     ecx
        push    ecx
        mov     edx, [_7z.StreamsInfo.unpackCRCs]
        cmp     [edx+ecx*4], eax
        jnz     return.err
@@:
        add     esi, 8
        pop     ecx
        inc     ecx
        cmp     ecx, [_7z.StreamsInfo.numFolders]
        jb      .loop
.ret:
        mov     ecx, [_7z.StreamsInfo.folders]
        call    [pgfree]
        mov     [error_proc], .error2
        mov     [clear_proc], .clear2
.ret2:
        mov     ecx, [_7z.StreamsInfo.packSizes]
        call    [pgfree]
        ret

.error:
        call    .free
        jmp     _7z.ReadStreamsInfo.error
.clear:
        call    .free
        jmp     _7z.ReadStreamsInfo.clear
.error2:
        call    .free
        jmp     open_7z.header_error
.clear2:
        call    .free
        jmp     open_7z.clear
.free:
        mov     ebx, [_7z.StreamsInfo.numFolders]
.free2:
        dec     ebx
        js      @f
        mov     ecx, [_7z.unpacked]
        mov     ecx, [ecx+ebx*8+4]
        call    [pgfree]
        jmp     .free2
@@:
        mov     ecx, [_7z.unpacked]
        cmp     ecx, _7z.unpbuf
        jz      @f
        call    [pgfree]
@@:
        ret

_7z.GetUnPackSize:
        xor     edx, edx
        mov     eax, [ebx+_7z.StreamsInfo.numOutStreams]
        test    eax, eax
        jz      .ret
.scan:
        dec     eax
        js      return.err
        push    ecx
        mov     ecx, [ebx+_7z.StreamsInfo.numBindPairs]
        mov     edx, [ebx+_7z.StreamsInfo.bindPairs]
@@:
        add     edx, 8
        dec     ecx
        js      .found
        cmp     [edx-4], eax
        jnz     @b
        pop     ecx
        jmp     .scan
.found:
        mov     [ebx+_7z.StreamsInfo.mainOutStream], eax
        mov     ecx, [ebx+_7z.StreamsInfo.unpackSizes]
        mov     edx, [ecx+eax*8+4]
        mov     eax, [ecx+eax*8]
        pop     ecx
.ret:
        ret

uglobal
align 4
_7z.StreamsInfo:
.numPackStreams dd      ?
.packSizes      dd      ?       ; 8 bytes per item
.numFolders     dd      ?
.unpackCRCsDefined dd   ?
.unpackCRCs     dd      ?
.folders        dd      ?
.folders.size   dd      ?
.folders.alloc  dd      ?
.dataOffset     dq      ?
.packCRCsDefined dd     ?       ; 1 byte per item
.packCRCs       dd      ?       ; 4 bytes per item
virtual at 0
.numCoders      dd      ?
.bindPairs      dd      ?
.numBindPairs   dd      ?
.packedStreams  dd      ?
.numPackedStreams dd    ?
.startPackedStream dd   ?
.numOutStreams  dd      ?
.unpackSizes    dd      ?
.mainOutStream  dd      ?
.numUnPackStreams dd    ?
.packOffset     dq      ?
.fsz = $
end virtual
.numSubStreams  dd      ?
.SubStreamsSizes dd     ?
.SubStreamsCRCsDefined dd ?
.SubStreamsCRCs dd      ?
.tmpSubStreamsCRCsDefined dd ?
.tmpSubStreamsCRCs dd   ?
endg

_7z.SkipData:
        call    _7z.ReadNumber
        call    _7z.SkipSize
        ret

_7z.WaitAttribute:
        call    _7z.ReadNumber
        cmp     eax, ecx
        jz      @f
        test    eax, eax
        jz      return.err
        call    _7z.SkipData
        jmp     _7z.WaitAttribute
@@:
        ret

_7z.ReadBoolVector2:
; in: ebx->vector, ecx=size
        call    _7z.ReadByte
        test    al, al
        jz      _7z.ReadBoolVector
        push    ebx ecx
        jecxz   .ret
@@:
        mov     byte [ebx], 1
        inc     ebx
        loop    @b
.ret:
        pop     ecx ebx
        ret

_7z.ReadBoolVector:
        push    ebx ecx
        jecxz   .ret
        xor     eax, eax
.1:
        add     al, al
        jnz     @f
        call    _7z.ReadByte
        stc
        adc     al, al
@@:
        setc    byte [ebx]
        inc     ebx
        loop    .1
.ret:
        pop     ecx ebx
        ret

_7z.ReadHashDigests:
; in: ebx->bool vector, esi->crc, ecx=size
        call    _7z.ReadBoolVector2
        jecxz   .ret
.1:
        xor     eax, eax
        cmp     byte [ebx], al
        jz      @f
        call    _7z.ReadDword
@@:
        mov     [esi], eax
        add     esi, 4
        inc     ebx
        loop    .1
.ret:
        ret

uglobal
_7z.saved_edi   dd      ?
_7z.saved_edi_end dd    ?
endg

_7z.SwitchStream:
        mov     eax, [bufend]
        mov     [_7z.saved_edi_end], eax
        call    _7z.ReadByte
        test    al, al
        jnz     @f
        ret
@@:
        call    _7z.ReadNum
        mov     [_7z.saved_edi], edi
        cmp     eax, [_7z.dataVectorSize]
        jae     return.err
        shl     eax, 3
        add     eax, [_7z.dataVector]
        mov     edi, [eax+4]
        mov     eax, [eax]
        add     eax, edi
        mov     [bufend], eax
        ret

_7z.UnswitchStream:
        mov     eax, [_7z.saved_edi_end]
        cmp     eax, [bufend]
        jnz     @f
        ret
@@:
        mov     [bufend], eax
        mov     edi, [_7z.saved_edi]
@@:
        ret

; 6 программистов пытались код понять,
; Один из них сошёл с ума, и их осталось 5.

_7z.ReadStreamsInfo:
        xor     eax, eax
        mov     [_7z.StreamsInfo.numPackStreams], eax
        mov     [_7z.StreamsInfo.packSizes], eax
        mov     [_7z.StreamsInfo.numFolders], eax
        mov     [_7z.StreamsInfo.folders], eax
        mov     [_7z.StreamsInfo.numSubStreams], eax
        mov     [_7z.StreamsInfo.SubStreamsSizes], eax
        mov     [_7z.StreamsInfo.tmpSubStreamsCRCsDefined], eax
        mov     [error_proc], .error
        mov     [clear_proc], .clear
.mainloop:
        call    _7z.ReadNum
        test    eax, eax
        jz      @b
        cmp     eax, kSubStreamsInfo
        jz      .SubStreamsInfo
        cmp     eax, kUnPackInfo
        jz      .UnpackInfo
        cmp     eax, kPackInfo
        jnz     return.err

.PackInfo:
        cmp     [_7z.StreamsInfo.numPackStreams], 0
        jnz     return.err
        call    _7z.ReadNumber
        add     eax, 0x20
        adc     edx, 0
        mov     dword [_7z.StreamsInfo.dataOffset], eax
        mov     dword [_7z.StreamsInfo.dataOffset+4], edx
        call    _7z.ReadNum
        mov     [_7z.StreamsInfo.numPackStreams], eax
        mov     ecx, kSize
        call    _7z.WaitAttribute
        mov     ecx, [_7z.StreamsInfo.numPackStreams]
        imul    ecx, 8+1+4
        jecxz   .nostreams1
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     [_7z.StreamsInfo.packSizes], eax
        mov     ecx, [_7z.StreamsInfo.numPackStreams]
        push    eax
        lea     eax, [eax+ecx*8]
        mov     [_7z.StreamsInfo.packCRCsDefined], eax
        mov     ebx, eax
        add     eax, ecx
        mov     [_7z.StreamsInfo.packCRCs], eax
@@:
        mov     byte [ebx], 0
        inc     ebx
        loop    @b
        pop     eax
.nostreams1:
        mov     ecx, [_7z.StreamsInfo.numPackStreams]
        jecxz   .noloop1
        mov     esi, eax
@@:
        call    _7z.ReadNumber
        mov     [esi], eax
        mov     [esi+4], edx
        add     esi, 8
        loop    @b
.noloop1:
        call    _7z.ReadNum
        test    eax, eax
        jz      .mainloop
        cmp     eax, kCRC
        jz      .packInfo.crc
        call    _7z.SkipData
        jmp     .noloop1
.packInfo.crc:
        mov     esi, [_7z.StreamsInfo.packCRCs]
        mov     ebx, [_7z.StreamsInfo.packCRCsDefined]
        mov     ecx, [_7z.StreamsInfo.numPackStreams]
        call    _7z.ReadHashDigests
        jmp     .noloop1

.UnpackInfo:
        cmp     [_7z.StreamsInfo.folders], 0
        jnz     return.err
        mov     ecx, kFolder
        call    _7z.WaitAttribute
        call    _7z.ReadNum
        mov     ecx, eax
        mov     [_7z.StreamsInfo.numFolders], eax
        call    _7z.SwitchStream
        test    ecx, ecx
        jz      .nofolders
        push    ecx
        mov     ecx, 0x1000
        and     [_7z.StreamsInfo.folders.size], 0
        mov     [_7z.StreamsInfo.folders.alloc], ecx
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     [_7z.StreamsInfo.folders], eax
        pop     ecx
        lea     ecx, [ecx*9]
        call    .folders_alloc
        xor     ecx, ecx
.folderloop:
        mov     eax, [_7z.StreamsInfo.folders]
        push    [_7z.StreamsInfo.folders.size]
        pop     dword [eax+ecx*4]
        push    ebp
        mov     [_ebp], ebp
        push    ecx
        mov     ecx, _7z.StreamsInfo.fsz
        call    .folders_alloc
        mov     ebp, eax
        add     ebp, [_7z.StreamsInfo.folders]
        call    _7z.ReadNum
        mov     [ebp+_7z.StreamsInfo.numCoders], eax
        test    eax, eax
        jz      return.err
        mov     ecx, eax
        push    ecx
        shl     ecx, 2
        sub     ebp, [_7z.StreamsInfo.folders]
        call    .folders_alloc
        add     ebp, [_7z.StreamsInfo.folders]
        pop     ecx
        add     eax, [_7z.StreamsInfo.folders]
@@:
        and     dword [eax], 0
        add     eax, 4
        loop    @b
        push    0       ; numOutStreams
        push    0       ; numInStreams
; [folders] = array of pointers to coders info
; Format of coder info:
; dd NextCoder
; db MethodID.IDSize
; times IDSize db MethodID.ID
; dd NumInStreams
; dd NumOutStreams
; dd PropSize
; times PropSize db Properties
        xor     esi, esi
.coders_loop:
        call    _7z.ReadByte
        mov     dl, al
        and     eax, 0xF
        lea     ecx, [eax+17]
        sub     ebp, [_7z.StreamsInfo.folders]
        call    .folders_alloc
        add     ebp, [_7z.StreamsInfo.folders]
        push    eax
        mov     ecx, [_7z.StreamsInfo.folders]
        lea     ebx, [ebp+esi*4+_7z.StreamsInfo.fsz]
@@:
        cmp     dword [ebx], 0
        jz      @f
        mov     ebx, [ebx]
        add     ebx, ecx
        jmp     @b
@@:
        mov     [ebx], eax
        pop     ebx
        add     ebx, ecx
        and     dword [ebx], 0
        add     ebx, 4
        mov     ecx, edx
        and     ecx, 0xF
        mov     byte [ebx], cl
        inc     ebx
        jecxz   .2
@@:
        call    _7z.ReadByte
        mov     byte [ebx], al
        inc     ebx
        loop    @b
.2:
        test    dl, 10h
        jnz     .coder_multiple_streams
        mov     dword [ebx], 1
        mov     dword [ebx+4], 1
        inc     dword [esp]
        inc     dword [esp+4]
        jmp     @f
.coder_multiple_streams:
        call    _7z.ReadNum
        mov     [ebx], eax
        add     dword [esp], eax
        call    _7z.ReadNum
        mov     [ebx+4], eax
        add     dword [esp+4], eax
; all currently defined 7z coders have 1 unpacked stream
; moreover, all reasonable coders have 1 unpacked stream
; so assume 1 output stream for unpacker - this simplifies decoding procedure
        cmp     eax, 1
        jnz     return.err
@@:
        and     dword [ebx+8], 0
        add     ebx, 12
        test    dl, 20h
        jz      .coder_no_properties
        call    _7z.ReadNum
        test    eax, eax
        jz      .coder_no_properties
        mov     [ebx-4], eax
        mov     ecx, eax
        sub     ebx, [_7z.StreamsInfo.folders]
        sub     ebp, [_7z.StreamsInfo.folders]
        call    .folders_alloc
        add     ebp, [_7z.StreamsInfo.folders]
        add     ebx, [_7z.StreamsInfo.folders]
@@:
        call    _7z.ReadByte
        mov     [ebx], al
        inc     ebx
        loop    @b
.coder_no_properties:
        test    dl, dl
        js      .coders_loop
        inc     esi
        cmp     esi, [ebp+_7z.StreamsInfo.numCoders]
        jb      .coders_loop
        mov     [ebp+_7z.StreamsInfo.numUnPackStreams], 1
        mov     eax, [esp+4]
        dec     eax
        js      return.err
        mov     [ebp+_7z.StreamsInfo.numBindPairs], eax
        push    eax
        mov     ecx, eax
        shl     ecx, 3
        sub     ebp, [_7z.StreamsInfo.folders]
        call    .folders_alloc
        add     ebp, [_7z.StreamsInfo.folders]
        mov     [ebp+_7z.StreamsInfo.bindPairs], eax
        pop     ecx
        jecxz   .noloop3
        mov     esi, eax
        add     esi, [_7z.StreamsInfo.folders]
@@:
        call    _7z.ReadNum
        mov     [esi], eax
        cmp     eax, [esp]
        jae     return.err
        call    _7z.ReadNum
        mov     [esi+4], eax
        cmp     eax, [esp+4]
        jae     return.err
        add     esi, 8
        loop    @b
.noloop3:
        pop     eax     ; numInStreams
        pop     edx     ; numOutStreams
        mov     [ebp+_7z.StreamsInfo.numOutStreams], edx
        sub     eax, edx
        js      return.err
        inc     eax
        mov     [ebp+_7z.StreamsInfo.numPackedStreams], eax
        mov     ecx, eax
        push    eax
        shl     ecx, 2
        sub     ebp, [_7z.StreamsInfo.folders]
        call    .folders_alloc
        add     ebp, [_7z.StreamsInfo.folders]
        mov     [ebp+_7z.StreamsInfo.packedStreams], eax
        pop     ecx
        mov     esi, eax
        add     esi, [_7z.StreamsInfo.folders]
        cmp     ecx, 1
        jnz     .multiplePackedStreams
        mov     ebx, [ebp+_7z.StreamsInfo.bindPairs]
        add     ebx, [_7z.StreamsInfo.folders]
        push    edx
        xor     ecx, ecx
.3:
        push    ebx
        mov     edx, [ebp+_7z.StreamsInfo.numBindPairs]
@@:
        dec     edx
        js      .5
        cmp     dword [ebx], ecx
        jz      .4
        add     ebx, 8
        jmp     @b
.4:
        pop     ebx
        inc     ecx
        jmp     .3
.5:
        pop     ebx
        pop     edx
        mov     [esi], ecx
        jmp     .commonPackedStreams
.multiplePackedStreams:
        call    _7z.ReadNum
        mov     [esi], eax
        add     esi, 4
        loop    .multiplePackedStreams
.commonPackedStreams:
        mov     ecx, edx
        shl     ecx, 3
        sub     ebp, [_7z.StreamsInfo.folders]
        call    .folders_alloc
        add     ebp, [_7z.StreamsInfo.folders]
        mov     [ebp+_7z.StreamsInfo.unpackSizes], eax
        pop     ecx
        xor     edx, edx
        jecxz   @f
        mov     eax, [_7z.StreamsInfo.folders]
        mov     eax, [eax+(ecx-1)*4]
        add     eax, [_7z.StreamsInfo.folders]
        mov     edx, [eax+_7z.StreamsInfo.numPackedStreams]
        add     edx, [eax+_7z.StreamsInfo.startPackedStream]
@@:
        mov     [ebp+_7z.StreamsInfo.startPackedStream], edx
        pop     ebp
        inc     ecx
        cmp     ecx, [_7z.StreamsInfo.numFolders]
        jb      .folderloop
.nofolders:
        call    _7z.UnswitchStream
        mov     ecx, kCodersUnPackSize
        call    _7z.WaitAttribute
        xor     ecx, ecx
.6:
        cmp     ecx, [_7z.StreamsInfo.numFolders]
        jae     .7
        mov     eax, [_7z.StreamsInfo.folders]
        push    ebp
        mov     ebp, [eax+ecx*4]
        add     ebp, eax
        mov     esi, [ebp+_7z.StreamsInfo.numOutStreams]
        mov     ebp, [ebp+_7z.StreamsInfo.unpackSizes]
        add     ebp, [_7z.StreamsInfo.folders]
@@:
        call    _7z.ReadNumber
        mov     dword [ebp], eax
        mov     dword [ebp+4], edx
        add     ebp, 8
        dec     esi
        jnz     @b
        pop     ebp
        inc     ecx
        jmp     .6
.7:
; convert all relative addresses to pointers
        xor     ecx, ecx
        mov     edx, [_7z.StreamsInfo.folders]
.8:
        cmp     ecx, [_7z.StreamsInfo.numFolders]
        jae     .9
        lea     eax, [edx+ecx*4]
        add     [eax], edx
        mov     eax, [eax]
        add     [eax+_7z.StreamsInfo.bindPairs], edx
        add     [eax+_7z.StreamsInfo.packedStreams], edx
        add     [eax+_7z.StreamsInfo.unpackSizes], edx
        mov     ebx, [eax+_7z.StreamsInfo.numCoders]
        add     eax, _7z.StreamsInfo.fsz
        inc     ecx
.11:
        dec     ebx
        js      .8
        push    eax
@@:
        cmp     dword [eax], 0
        jz      @f
        add     [eax], edx
        mov     eax, [eax]
        jmp     @b
@@:
        pop     eax
        add     eax, 4
        jmp     .11
.9:
; done
        mov     eax, [_7z.StreamsInfo.numFolders]
        lea     edx, [edx+eax*4]
        mov     [_7z.StreamsInfo.unpackCRCsDefined], edx
        add     edx, eax
        mov     [_7z.StreamsInfo.unpackCRCs], edx
@@:
        call    _7z.ReadNum
        test    eax, eax
        jz      .mainloop
        cmp     eax, kCRC
        jz      .unpackInfo.crc
        call    _7z.SkipData
        jmp     @b
.unpackInfo.crc:
        mov     esi, [_7z.StreamsInfo.unpackCRCs]
        mov     ebx, [_7z.StreamsInfo.unpackCRCsDefined]
        mov     ecx, [_7z.StreamsInfo.numFolders]
        call    _7z.ReadHashDigests
        jmp     @b

.SubStreamsInfo:
        cmp     [_7z.StreamsInfo.numFolders], 0
        jz      return.err
        cmp     [_7z.StreamsInfo.numSubStreams], 0
        jnz     return.err
        mov     ebx, [_7z.StreamsInfo.folders]
.ssloop1:
        call    _7z.ReadNum
        cmp     eax, kNumUnPackStream
        jz      .NumUnPackStream
        cmp     eax, kCRC
        jz      .break1
        cmp     eax, kSize
        jz      .break1
        test    eax, eax
        jz      .break1
        call    _7z.SkipData
        jmp     .ssloop1
.NumUnPackStream:
        xor     ecx, ecx
@@:
        call    _7z.ReadNum
        mov     edx, [ebx+ecx*4]
        mov     [edx+_7z.StreamsInfo.numUnPackStreams], eax
        inc     ecx
        cmp     ecx, [_7z.StreamsInfo.numFolders]
        jb      @b
        jmp     .ssloop1
.break1:
; calculate number of substreams and number of defined digests
        xor     ecx, ecx
        xor     ebp, ebp
        push    0
        push    eax
        xor     eax, eax
.ssloop2:
        mov     edx, [ebx+ecx*4]
        mov     edx, [edx+_7z.StreamsInfo.numUnPackStreams]
        cmp     edx, 1
        jz      @f
        inc     ebp
@@:
        add     eax, edx
        jc      return.err
        cmp     edx, 1
        jnz     @f
        push    eax
        mov     eax, [_7z.StreamsInfo.unpackCRCsDefined]
        cmp     byte [eax+ecx], 0
        pop     eax
        jz      @f
        xor     edx, edx
@@:
        add     [esp+4], edx
        inc     ecx
        cmp     ecx, [_7z.StreamsInfo.numFolders]
        jb      .ssloop2
        mov     [_7z.StreamsInfo.numSubStreams], eax
        imul    ecx, eax, 4+8+1
        jecxz   .noss1
        push    eax
        call    [pgalloc]
        pop     ecx
        test    eax, eax
        jz      return.clear
        mov     [_7z.StreamsInfo.SubStreamsSizes], eax
        lea     eax, [eax+ecx*8]
        mov     [_7z.StreamsInfo.SubStreamsCRCsDefined], eax
        add     eax, ecx
        mov     [_7z.StreamsInfo.SubStreamsCRCs], eax
@@:
        dec     eax
        mov     byte [eax], 0
        loop    @b
.noss1:
        mov     ecx, [esp+4]
        jecxz   @f
        imul    ecx, 4+1
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     [_7z.StreamsInfo.tmpSubStreamsCRCsDefined], eax
        add     eax, [esp+4]
        mov     [_7z.StreamsInfo.tmpSubStreamsCRCs], eax
@@:
        test    ebp, ebp
        jz      @f
        cmp     dword [esp], kSize
        jnz     return.err
@@:
        xor     ecx, ecx
        mov     ebp, [_7z.StreamsInfo.SubStreamsSizes]
.ssloop3:
        mov     ebx, [_7z.StreamsInfo.folders]
        mov     ebx, [ebx+ecx*4]
        mov     eax, [ebx+_7z.StreamsInfo.numUnPackStreams]
        dec     eax
        js      .sscont3
        push    0
        push    0
@@:
        dec     eax
        js      @f
        push    eax
        call    _7z.ReadNumber
        mov     [ebp], eax
        mov     [ebp+4], edx
        add     ebp, 8
        add     [esp+4], eax
        adc     [esp+8], edx
        jc      return.err
        pop     eax
        jmp     @b
@@:
        call    _7z.GetUnPackSize
        sub     eax, [esp]
        sbb     edx, [esp+4]
        jc      return.err
        mov     [ebp], eax
        mov     [ebp+4], edx
        add     ebp, 8
        add     esp, 8
.sscont3:
        inc     ecx
        cmp     ecx, [_7z.StreamsInfo.numFolders]
        jb      .ssloop3
        pop     eax
        cmp     eax, kSize
        jnz     @f
        call    _7z.ReadNum
@@:
        cmp     eax, kCRC
        jz      .sscrc
        test    eax, eax
        jz      .ssend
        call    _7z.SkipData
.ssloop4:
        call    _7z.ReadNum
        jmp     @b
.sscrc:
        pop     ecx
        push    ecx
        mov     ebx, [_7z.StreamsInfo.tmpSubStreamsCRCsDefined]
        mov     esi, [_7z.StreamsInfo.tmpSubStreamsCRCs]
        call    _7z.ReadHashDigests
        xor     ecx, ecx        ; index in out
        xor     edx, edx        ; index in tmp
        push    -1
        pop     ebx             ; index in folders
.ssloop5:
        inc     ebx
        cmp     ebx, [_7z.StreamsInfo.numFolders]
        jae     .ssloop4
        mov     eax, [_7z.StreamsInfo.folders]
        mov     ebp, [eax+ebx*4]
        mov     ebp, [ebp+_7z.StreamsInfo.numUnPackStreams]
        cmp     ebp, 1
        jnz     @f
        mov     eax, [_7z.StreamsInfo.unpackCRCsDefined]
        cmp     byte [eax+ebx], 0
        jz      @f
        mov     esi, [_7z.StreamsInfo.SubStreamsCRCsDefined]
        mov     byte [esi+ecx], 1
        mov     esi, [_7z.StreamsInfo.unpackCRCs]
        mov     eax, [esi+ebx*4]
        mov     esi, [_7z.StreamsInfo.SubStreamsCRCs]
        mov     [esi+ecx*4], eax
        inc     ecx
        jmp     .ssloop5
@@:
        dec     ebp
        js      .ssloop5
        mov     esi, [_7z.StreamsInfo.tmpSubStreamsCRCsDefined]
        mov     al, [esi+edx]
        mov     esi, [_7z.StreamsInfo.SubStreamsCRCsDefined]
        mov     [esi+ecx], al
        mov     esi, [_7z.StreamsInfo.tmpSubStreamsCRCs]
        mov     eax, [esi+edx*4]
        mov     esi, [_7z.StreamsInfo.SubStreamsCRCs]
        mov     [esi+ecx*4], eax
        inc     edx
        inc     ecx
        jmp     @b
.ssend:
        mov     ecx, [_7z.StreamsInfo.tmpSubStreamsCRCsDefined]
        call    [pgfree]
        and     [_7z.StreamsInfo.tmpSubStreamsCRCsDefined], 0
        pop     ecx
        jmp     .mainloop

.folders_alloc:
        push    ecx
        add     ecx, [_7z.StreamsInfo.folders.size]
        sub     ecx, [_7z.StreamsInfo.folders.alloc]
        jbe     @f
        push    edx
        or      ecx, 0xFFF
        inc     ecx
        add     ecx, [_7z.StreamsInfo.folders.alloc]
        mov     [_7z.StreamsInfo.folders.alloc], ecx
        mov     edx, [_7z.StreamsInfo.folders]
        call    [pgrealloc]
        pop     edx
        test    eax, eax
        jz      return.clear
        mov     [_7z.StreamsInfo.folders], eax
@@:
        pop     ecx
        mov     eax, [_7z.StreamsInfo.folders.size]
        add     [_7z.StreamsInfo.folders.size], ecx
        ret

.error:
        mov     ecx, [_7z.StreamsInfo.packSizes]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [_7z.StreamsInfo.folders]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [_7z.StreamsInfo.SubStreamsSizes]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [_7z.StreamsInfo.tmpSubStreamsCRCsDefined]
        jecxz   @f
        call    [pgfree]
@@:
        jmp     open_7z.header_error
.clear:
        mov     ecx, [_7z.StreamsInfo.packSizes]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [_7z.StreamsInfo.folders]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [_7z.StreamsInfo.SubStreamsSizes]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [_7z.StreamsInfo.tmpSubStreamsCRCsDefined]
        jecxz   @f
        call    [pgfree]
@@:
        jmp     open_7z.clear

uglobal
align 4
_7z.decode.mainbuf      dd      ?
_7z.decode.pStreams     dd      ?
_7z.decode.pStreams_def rd      8
_7z.decode.outBufferSize dd     ?
_7z.decode.outBufferPtr dd      ?
_7z.decode.outBufferRead dd     ?
endg

virtual at 0
streamInfo:
.fullSize       dq      ?
.fillBuf        dd      ?       ; procedure which fills the buffer
.bufSize        dd      ?
.bufDataLen     dd      ?
.bufPtr         dd      ?
.size = $
end virtual

_7z.decode:
; in: ebx->folder, outBufferSize, outBufferPtr
; N.B. We assume that every coder has 1 output (unpacked) stream.
        and     [.mainbuf], 0
        mov     [.pStreams], .pStreams_def
        mov     [error_proc], .error
        mov     [clear_proc], .clear
        call    .init
        and     [.outBufferRead], 0
@@:
        push    edi
        mov     eax, [ebx+_7z.StreamsInfo.mainOutStream]
        mov     esi, [.pStreams]
        mov     eax, [esi+eax*4]
        call    fillBuf
        mov     ecx, [eax+streamInfo.bufSize]
        mov     esi, [eax+streamInfo.bufPtr]
        mov     edi, [.outBufferPtr]
        add     edi, [.outBufferRead]
        add     [.outBufferRead], ecx
        mov     edx, ecx
        shr     ecx, 2
        rep     movsd
        mov     ecx, edx
        and     ecx, 3
        rep     movsb
        pop     edi
        mov     eax, [.outBufferRead]
        cmp     eax, [.outBufferSize]
        jb      @b
        mov     ecx, [.pStreams]
        cmp     ecx, .pStreams_def
        jz      @f
        call    [pgfree]
@@:
        mov     ecx, [.mainbuf]
        jecxz   @f
        call    [pgfree]
@@:
        mov     [error_proc], _7z.ReadAndDecodePackedStreams.error
        mov     [clear_proc], _7z.ReadAndDecodePackedStreams.clear
        ret

.error:
        mov     ecx, [.pStreams]
        cmp     ecx, .pStreams_def
        jz      @f
        call    [pgfree]
@@:
        mov     ecx, [.mainbuf]
        jecxz   @f
        call    [pgfree]
@@:
        jmp     _7z.ReadAndDecodePackedStreams.error
.clear:
        mov     ecx, [.pStreams]
        cmp     ecx, .pStreams_def
        jz      @f
        call    [pgfree]
@@:
        mov     ecx, [.mainbuf]
        jecxz   @f
        call    [pgfree]
@@:
        jmp     _7z.ReadAndDecodePackedStreams.clear

.init:
; We use stack to keep some information on streams,
;   to avoid stack overflow we limit possible number of coders.
; Anyway, in real life maximum number of coders is 5
;   (password protection filter + BCJ2 filter + 3 LZMA coders),
;   from command line user can define more coders, but
;   archive with, say, 32 coders is most likely bogus.
        cmp     [ebx+_7z.StreamsInfo.numCoders], 128
        ja      return.err
; allocate space for streams info array
        mov     eax, .pStreams_def
        mov     ecx, [ebx+_7z.StreamsInfo.numPackedStreams]
        add     ecx, [ebx+_7z.StreamsInfo.numCoders]
        cmp     ecx, 8
        jbe     @f
        shl     ecx, 2
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
@@:
        mov     [.pStreams], eax
; calculate size of buffers required for decoders
        xor     ecx, ecx
        push    ecx
        mov     ebp, [ebx+_7z.StreamsInfo.unpackSizes]
.calcbufsz:
; LZMA decoder: method ID = [size=03] 03 01 01, NumInStreams = NumOutStreams = 1
        mov     eax, [ebx+_7z.StreamsInfo.fsz+ecx*4]
        call    _7z.GetCoder
        jnc     .known_id
.unknown_method_id:
        movzx   esi, byte [eax+4]
        lea     ecx, [eax+5]
        mov     ebx, aUnknownMethod.z
@@:
        dec     esi
        js      @f
        mov     byte [ebx], ' '
        inc     ebx
        mov     al, [ecx]
        push    eax
        shr     al, 4
        cmp     al, 10
        sbb     al, 69h
        das
        mov     [ebx], al
        inc     ebx
        pop     eax
        and     al, 0xF
        cmp     al, 10
        sbb     al, 69h
        das
        mov     [ebx], al
        inc     ebx
        inc     ecx
        jmp     @b
@@:
        mov     byte [ebx], 0
        push    ContinueBtn
        push    1
        push    aUnknownMethod_ptr
        push    1
        call    [SayErr]
        jmp     return.clear
iglobal
if lang eq ru
aUnknownMethod  db      'Неизвестный метод:'
else
aUnknownMethod  db      'Unknown method:'
end if
.z:
                times 16*3+1 db 0
aUnknownMethod_ptr dd   aUnknownMethod
endg
.known_id:
        movzx   esi, byte [eax+4]
        lea     esi, [esi+eax+17]
        call    [_7z.GetBufSizeTable+edx*4]
        add     eax, 3
        and     al, not 3
        add     [esp], eax
        jc      .nomem
        cmp     dword [ebp+4], 0
        jnz     @f
        cmp     dword [ebp], edx
        jae     @f
        mov     edx, [ebp]
@@:
        add     edx, 0xF
        and     edx, not 0xF
        add     [esp], edx
        jc      .nomem
        add     ebp, 8
        inc     ecx
        cmp     ecx, [ebx+_7z.StreamsInfo.numCoders]
        jb      .calcbufsz
; add 4 pages for each of not binded input streams
        mov     ecx, [ebx+_7z.StreamsInfo.numPackedStreams]
        shl     ecx, 14
        add     [esp], ecx
;; calculate free RAM
;        call    [getfreemem]
;        sub     eax, 64         ; keep at least 64 Kb of free RAM
;        jbe     .nomem
;        pop     ecx
;        push    ecx
;        add     ecx, 0x3FF
;        shr     ecx, 10
;        cmp     eax, ecx
;        jae     .memok
        jmp     .memok
.nomem:
        push    ContinueBtn
        push    1
        push    aNoFreeRam_ptr
        push    1
        call    [SayErr]
        jmp     return.clear
.memok:
; allocate streamInfos
        pop     ecx
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     [.mainbuf], eax
; initialize streamInfos for decoders
        xor     ecx, ecx
        mov     ebp, [ebx+_7z.StreamsInfo.unpackSizes]
        mov     edx, [.pStreams]
.init1:
        mov     [edx], eax
        add     edx, 4
        push    edx
        push    dword [ebp]
        pop     dword [eax]
        push    dword [ebp+4]
        pop     dword [eax+4]
        and     dword [eax+streamInfo.size], 0
        push    eax
        mov     eax, [ebx+_7z.StreamsInfo.fsz+ecx*4]
        call    _7z.GetCoder
        movzx   esi, byte [eax+4]
        lea     esi, [esi+eax+17]
        call    [_7z.GetBufSizeTable+edx*4]
        pop     esi
        push    esi
        add     eax, 3
        and     al, not 3
        add     [esp], eax
        pop     [esi+streamInfo.bufPtr]
        push    [esi+streamInfo.bufPtr]
        cmp     dword [ebp+4], 0
        jnz     @f
        cmp     dword [ebp], edx
        jae     @f
        mov     edx, [ebp]
@@:
        add     edx, 0xF
        and     edx, not 0xF
        mov     [esi+streamInfo.bufSize], edx
        and     [esi+streamInfo.bufDataLen], 0
        pop     eax
        add     eax, edx
        pop     edx
        add     ebp, 8
        inc     ecx
        cmp     ecx, [ebx+_7z.StreamsInfo.numCoders]
        jb      .init1
; initialize streamInfos for input streams
        xor     ecx, ecx
@@:
        mov     [edx], eax
        add     edx, 4
        mov     esi, [_7z.CurInStream]
        inc     [_7z.CurInStream]
        shl     esi, 3
        add     esi, [_7z.StreamsInfo.packSizes]
        push    dword [esi]
        pop     dword [eax]
        push    dword [esi+4]
        pop     dword [eax+4]
        mov     [eax+streamInfo.fillBuf], fillBufFromInStream
        lea     esi, [eax+streamInfo.size+8]
        mov     [eax+streamInfo.bufPtr], esi
        mov     [eax+streamInfo.bufSize], 0x4000 - streamInfo.size - 8
        and     [eax+streamInfo.bufDataLen], 0
        mov     esi, dword [_7z.StreamsInfo.dataOffset]
        mov     dword [eax+streamInfo.size], esi
        mov     esi, dword [_7z.StreamsInfo.dataOffset+4]
        mov     dword [eax+streamInfo.size+4], esi
        mov     esi, [eax]
        add     dword [_7z.StreamsInfo.dataOffset], esi
        mov     esi, [eax+4]
        adc     dword [_7z.StreamsInfo.dataOffset+4], esi
        add     eax, 0x4000
        inc     ecx
        cmp     ecx, [ebx+_7z.StreamsInfo.numPackedStreams]
        jb      @b
; initialize links between streams
        push    -1
        mov     edx, [ebx+_7z.StreamsInfo.mainOutStream]
        xor     ecx, ecx
.init_links:
        mov     eax, [ebx+_7z.StreamsInfo.fsz+edx*4]
        movzx   esi, byte [eax+4]
        cmp     ecx, [eax+esi+5]
        jb      @f
        pop     ecx
        cmp     ecx, -1
        jz      .inited_links
        pop     edx
        jmp     .init_links
@@:
        push    edx
        push    ecx
@@:
        dec     edx
        js      @f
        mov     eax, [ebx+_7z.StreamsInfo.fsz+edx*4]
        movzx   esi, byte [eax+4]
        add     ecx, [eax+esi+5]
        jmp     @b
@@:
        mov     esi, [ebx+_7z.StreamsInfo.bindPairs]
        mov     edx, [ebx+_7z.StreamsInfo.numBindPairs]
@@:
        add     esi, 8
        dec     edx
        js      .link_to_in
        cmp     ecx, [esi-8]
        jnz     @b
        pop     ecx
        pop     edx
        mov     eax, [.pStreams]
        mov     eax, [eax+edx*4]
        lea     eax, [eax+streamInfo.size+ecx*4]
        push    edx
        inc     ecx
        push    ecx
        mov     edx, [esi-4]
        mov     ecx, [.pStreams]
        mov     ecx, [ecx+edx*4]
        mov     [eax], ecx
        cmp     dword [ecx+streamInfo.size], 0
        jnz     return.err
        xor     ecx, ecx
        jmp     .init_links
.link_to_in:
        mov     esi, [ebx+_7z.StreamsInfo.packedStreams]
        mov     edx, [ebx+_7z.StreamsInfo.numPackedStreams]
@@:
        dec     edx
        js      return.err
        cmp     ecx, [esi+edx*4]
        jnz     @b
        add     edx, [ebx+_7z.StreamsInfo.numCoders]
        mov     esi, [.pStreams]
        mov     eax, [esi+edx*4]
        pop     ecx
        pop     edx
        mov     esi, [esi+edx*4]
        mov     [esi+streamInfo.size+ecx*4], eax
        inc     ecx
        jmp     .init_links
.inited_links:
; initialize all decoders
; bool zzz_init_decoder(stream_info* ebp, esi->params, [esi-4]=params_size);
        xor     ecx, ecx
.init_decoders:
        mov     eax, [ebx+_7z.StreamsInfo.fsz+ecx*4]
        call    _7z.GetCoder
        movzx   esi, byte [eax+4]
        mov     ebp, [.pStreams]
        mov     ebp, [ebp+ecx*4]
        cmp     dword [esi+eax], 0
        jz      @f
        cmp     dword [ebp+streamInfo.size], 0
        jz      return.err
@@:
        lea     esi, [esi+eax+17]
        push    ebx ecx edi
        xor     ebx, ebx
        call    [_7z.InitTable+edx*4]
        pop     edi ecx ebx
        inc     ecx
        cmp     ecx, [ebx+_7z.StreamsInfo.numCoders]
        jb      .init_decoders
        ret

fillBuf:
        and     [eax+streamInfo.bufDataLen], 0
fillBufNozero:
        pushad
	mov	edx, [eax+streamInfo.bufSize]
	sub	edx, [eax+streamInfo.bufDataLen]
        cmp     dword [eax+streamInfo.fullSize+4], 0
        jnz     .full
        mov     ecx, dword [eax+streamInfo.fullSize]
        cmp     ecx, edx
        jbe     @f
.full:
        mov     ecx, edx
@@:
        sub     dword [eax+streamInfo.fullSize], ecx
        sbb     dword [eax+streamInfo.fullSize+4], 0
        mov     edi, [eax+streamInfo.bufPtr]
        add     edi, [eax+streamInfo.bufDataLen]
	add	[eax+streamInfo.bufDataLen], ecx
        jmp     [eax+streamInfo.fillBuf]

fillBufFromInStream:
        push    eax
        push    dword [eax+streamInfo.size+4]
        push    dword [eax+streamInfo.size]
        push    0
        push    [inStream]
        call    [seek]
        pop     eax
        push    eax
        push    ecx
        push    edi
        push    [inStream]
        call    [read]
        cmp     eax, ecx
        pop     eax
        jnz     .error
        add     dword [eax+streamInfo.size], ecx
        adc     dword [eax+streamInfo.size+4], 0
        popad
        ret
.error:
        push    ContinueBtn
        push    1
        push    aReadError_ptr
        push    1
        call    [SayErr]
        jmp     return.clear

_7z.GetCoder:
; in: eax->coder
; out: edx = coder type
        xor     edx, edx
; Copy decoder: method ID = [size=01] 00, NumInStreams = NumOutStreams = 1
        cmp     word [eax+4], 0x0001
        jnz     @f
        cmp     dword [eax+6], 1
        jz      .ret
@@:
        inc     edx
; LZMA decoder: method ID = [size=03] 03 01 01, NumInStreams = NumOutStreams = 1
        cmp     dword [eax+4], 0x01010303
        jnz     @f
        cmp     dword [eax+8], 1
        jz      .ret
@@:
        inc     edx
; PPMD decoder: method ID = [size=03] 03 04 01, NumInStreams = NumOutStreams = 1
        cmp     dword [eax+4], 0x01040303
        jnz     @f
        cmp     dword [eax+8], 1
        jz      .ret
@@:
        inc     edx
; BCJ filter: method ID = [size=04] 03 03 01 03, NumInStreams = NumOutStreams = 1
        cmp     byte [eax+4], 4
        jnz     @f
        cmp     dword [eax+5], 0x03010303
        jnz     @f
        cmp     dword [eax+9], 1
        jz      .ret
@@:
        inc     edx
; BCJ2 filter: method ID = [size=04] 03 03 01 1B, NumInStreams = 4, NumOutStreams = 1
        cmp     byte [eax+4], 4
        jnz     @f
        cmp     dword [eax+5], 0x1B010303
        jnz     @f
        cmp     dword [eax+9], 4
        jz      .ret
@@:
        inc     edx
; 7z-AES cryptor: method ID = [size=04] 06 F1 07 01, NumInStreams = NumOutStreams = 1
        cmp     byte [eax+4], 4
        jnz     @f
        cmp     dword [eax+5], 0x0107F106
        jnz     @f
        cmp     dword [eax+9], 1
        jz      .ret
@@:
        stc
.ret:
        ret

iglobal
align 4
label _7z.GetBufSizeTable dword
        dd      copy_get_buf_size
        dd      lzma_get_buf_size
        dd      ppmd_get_buf_size
        dd      bcj_get_buf_size
        dd      bcj2_get_buf_size
        dd      aes7z_get_buf_size
label _7z.InitTable dword
        dd      copy_init_decoder
        dd      lzma_init_decoder
        dd      ppmd_init_decoder
        dd      bcj_init_decoder
        dd      bcj2_init_decoder
        dd      aes7z_init_decoder
endg

copy_get_buf_size:
        mov     eax, streamInfo.size+8
        mov     edx, 0x10000
        ret

copy_init_decoder:
        mov     [ebp+streamInfo.fillBuf], copy_fillBuf
        and     dword [ebp+streamInfo.size+4], 0
        ret

copy_fillBuf:
        push    eax
        mov     esi, [eax+streamInfo.size+4]
        mov     eax, [eax+streamInfo.size]
.redo:
        push    ecx
        mov     edx, [eax+streamInfo.bufDataLen]
        sub     edx, esi
        cmp     ecx, edx
        jb      @f
        mov     ecx, edx
@@:
        sub     [esp], ecx
        add     esi, [eax+streamInfo.bufPtr]
        mov     edx, ecx
        shr     ecx, 2
        rep     movsd
        mov     ecx, edx
        and     ecx, 3
        rep     movsb
        pop     ecx
        jecxz   .done
        cmp     dword [eax+streamInfo.fullSize+4], 0
        jnz     @f
        cmp     dword [eax+streamInfo.fullSize], ecx
        jb      return.err
@@:
        call    fillBuf
        xor     esi, esi
        jmp     .redo
.done:
        sub     esi, [eax+streamInfo.bufPtr]
        pop     eax
        mov     [eax+streamInfo.size+4], esi
        popad
        ret

; int __stdcall ReadFolder([esp+4] = HANDLE hPlugin,
;       [esp+8] = unsigned dirinfo_start, [esp+12] = unsigned dirinfo_size,
;       [esp+16] = void* dirdata);
ReadFolder_7z:
; init header
        mov     edi, [esp+16]
        mov     ecx, 32/4
        xor     eax, eax
        rep     stosd
        mov     byte [edi-32], 1        ; version
        mov     ebp, [esp+4]
; get current directory
        lea     ebx, [ebp+handle_7z.root.subfolders]
        cmp     [ebp+handle_7z.curdir], 0
        jz      @f
        mov     ebx, [ebp+handle_7z.curdir]
        add     ebx, file_in_7z.subfolders
@@:
        mov     ecx, [ebx+16]
        mov     [edi-24], ecx           ; number of files
; edi points to BDFE
        push    6               ; assume EOF
        pop     eax
        sub     ecx, [esp+8]
        ja      @f
        and     dword [edi-28], 0       ; number of files read
        ret     10h
@@:
        cmp     ecx, [esp+12]
        jb      @f
        mov     ecx, [esp+12]
        xor     eax, eax        ; OK
@@:
        mov     [edi-28], ecx
        push    eax
; copy files data
        test    ecx, ecx
        jz      .done
; seek to required item
        mov     eax, [esp+8+4]
        mov     esi, [ebx]
.0:
        test    esi, esi
        jnz     .1
        mov     esi, [ebx+8]
.1:
        add     esi, ebp
        dec     eax
        js      .2
        mov     esi, [esi+file_in_7z.next]
        jmp     .0
.2:
.copy:
        cmp     [esi+file_in_7z.bPseudoFolder], 0
        jnz     .pseudo
        xor     eax, eax
        cmp     [esi+file_in_7z.bAttrDefined], 0
        jz      @f
        mov     eax, [esi+file_in_7z.attr]
@@:
        stosd
        xor     eax, eax
        stosd
        push    esi ecx
        add     esi, file_in_7z.CreationTime
        mov     ecx, 8
        rep     movsd
        pop     ecx esi
        mov     edx, [esi+file_in_7z.name]
        push    edi
@@:
        mov     al, [edx]
        inc     edx
        stosb
        test    al, al
        jnz     @b
        pop     edi
.cont:
        add     edi, 264
        mov     esi, [esi+file_in_7z.next]
        test    esi, esi
        jnz     @f
        mov     esi, [ebx+8]
@@:
        add     esi, ebp
        loop    .copy
        jmp     .done
.pseudo:
        push    0x10            ; attributes: folder
        pop     eax
        stosd
        xor     eax, eax
        stosd
        push    ecx esi
        lea     esi, [ebp+handle_7z.host_datetime]
        mov     ecx, 6
        rep     movsd
        pop     esi
        stosd
        stosd
        push    edi esi
        mov     ecx, [esi+file_in_7z.namelen]
        mov     esi, [esi+file_in_7z.name]
        rep     movsb
        mov     byte [edi], 0
        pop     esi edi ecx
        jmp     .cont
.done:
        pop     eax
        ret     10h

; bool __stdcall SetFolder([esp+4] = HANDLE hPlugin,
;       [esp+8] = const char* relative_path, [esp+12] = const char* absolute_path);
SetFolder_7z:
        mov     ebp, [esp+4]
        mov     edx, [ebp+handle_7z.curdir]
        mov     esi, [esp+8]
        cmp     dword [esi], '..'
        jz      .toparent
        xor     ecx, ecx
@@:
        inc     ecx
        cmp     byte [esi+ecx], 0
        jnz     @b
        mov     ebx, [ebp+handle_7z.root.subfolders]
        test    edx, edx
        jz      .scan
        mov     ebx, [edx+file_in_7z.subfolders]
.scan:
        test    ebx, ebx
        jz      .err
        add     ebx, ebp
        cmp     [ebx+file_in_7z.namelen], ecx
        jnz     .cont
        push    ecx esi
        mov     edi, [ebx+file_in_7z.name]
        repz    cmpsb
        pop     esi ecx
        jz      .set
.cont:
        mov     ebx, [ebx+file_in_7z.next]
        jmp     .scan
.toparent:
        test    edx, edx
        jz      .err
        mov     ebx, [edx+file_in_7z.parent]
        test    ebx, ebx
        jz      @f
        add     ebx, ebp
@@:
.set:
        mov     [ebp+handle_7z.curdir], ebx
        mov     al, 1
        ret     12
.err:
        xor     eax, eax
        ret     12

; ebp=hPlugin, eax->item, edi->info
getattr_7z:
        cmp     [eax+file_in_7z.bPseudoFolder], 0
        jnz     .pseudo
        lea     esi, [eax+file_in_7z.attr]
        movsd
        xor     eax, eax
        stosd
        add     esi, -(file_in_7z.attr+4) + file_in_7z.CreationTime
        mov     ecx, 8
        rep     movsd
        ret
.pseudo:
        mov     esi, [eax+file_in_7z.name]
        mov     ecx, [eax+file_in_7z.namelen]
        push    0x10    ; attributes: folder
        pop     eax
        stosd
        xor     eax, eax
        stosd
        push    ecx esi
        lea     esi, [ebp+handle_7z.host_datetime]
        push    6
        pop     ecx
        rep     movsd
        pop     esi ecx
        stosd
        stosd
        rep     movsb
        mov     byte [edi], 0
        ret

virtual at 0
file_handle_7z:
.type   dd      ?
.context dd     ?
.pos    dq      ?
.base   dd      ?       ; handle of archive
.item   dd      ?       ; pointer to file_in_7z
.mainStream dd  ?       ; (pointer in context)
.bError db      ?
        rb      3
.size = $
end virtual

iglobal
align 4
handle_table_7z dd      handle_table_7z,handle_table_7z
endg

; ebp=hPlugin, eax->item, edi=mode
open_file_7z:
        cmp     [eax+file_in_7z.bHasStream], 0
        jnz     @f
        mov     eax, empty_7z_handle
        ret
@@:
        mov     ecx, [ebp+handle_7z.password_len]
        inc     ecx
        setnz   [bPasswordDefined]
        jz      @f
        dec     ecx
        lea     esi, [ebp+handle_7z.password]
        push    edi
        mov     edi, password_unicode
        shr     ecx, 1
        rep     movsd
        adc     ecx, ecx
        rep     movsw
        pop     edi
@@:
        mov     [hOut], ecx
        mov     [_7z.decode.mainbuf], ecx
        mov     ecx, [ebp+handle_7z.host]
        mov     [inStream], ecx
        mov     [_7z.decode.pStreams], _7z.decode.pStreams_def
        push    eax
        mov     [_esp], esp
        mov     [_ebp], ebp
        mov     [error_proc], .error
        mov     [clear_proc], .clear
        mov     ebx, [ebp+handle_7z.folders]
        mov     ecx, [eax+file_in_7z.folder]
        cmp     [ebp+handle_7z.last_folder], ecx
        jnz     .nolast
        mov     edx, dword [eax+file_in_7z.folderStart]
        sub     edx, dword [ebp+handle_7z.last_pos]
        mov     esi, dword [eax+file_in_7z.folderStart+4]
        sbb     esi, dword [ebp+handle_7z.last_pos+4]
        jb      .nolast
        xor     ecx, ecx
        xchg    ecx, [ebp+handle_7z.last_context]
        mov     [_7z.decode.mainbuf], ecx
        mov     eax, [ebp+handle_7z.last_main_stream]
        mov     dword [eax+streamInfo.fullSize], edx
        mov     dword [eax+streamInfo.fullSize+4], esi
        or      [ebp+handle_7z.last_folder], -1
        pop     esi
        push    esi
        jmp     .commonl
.nolast:
        mov     ebx, [ebx+ecx*4]
        mov     eax, [ebx+_7z.StreamsInfo.startPackedStream]
        mov     [_7z.CurInStream], eax
        mov     eax, dword [ebx+_7z.StreamsInfo.packOffset]
        mov     dword [_7z.StreamsInfo.dataOffset], eax
        mov     eax, dword [ebx+_7z.StreamsInfo.packOffset+4]
        mov     dword [_7z.StreamsInfo.dataOffset+4], eax
        mov     eax, [ebp+handle_7z.packSizes]
        mov     [_7z.StreamsInfo.packSizes], eax
        push    ebp
        call    _7z.decode.init
        pop     ebp
        mov     eax, [ebx+_7z.StreamsInfo.mainOutStream]
        mov     esi, [_7z.decode.pStreams]
        mov     eax, [esi+eax*4]
        pop     esi
        push    esi
        mov     ecx, dword [esi+file_in_7z.folderStart]
        mov     dword [eax+streamInfo.fullSize], ecx
        mov     ecx, dword [esi+file_in_7z.folderStart+4]
        mov     dword [eax+streamInfo.fullSize+4], ecx
.commonl:
        call    skip_7z
        mov     ecx, dword [esi+file_in_7z.UnPackSize]
        mov     dword [eax+streamInfo.fullSize], ecx
        mov     edx, dword [esi+file_in_7z.UnPackSize+4]
        mov     dword [eax+streamInfo.fullSize+4], edx
        test    edx, edx
        jnz     .nomemstream
if defined LIMIT_FOR_MEM_STREAM
        cmp     ecx, LIMIT_FOR_MEM_STREAM
        ja      .nomemstream
end if
        push    eax ecx
        add     ecx, 0x3FF
        shr     ecx, 10         ; get size in Kb
        call    [getfreemem]
        shr     eax, 2
        cmp     ecx, eax
        pop     ecx eax
        ja      .nomemstream
; create memory stream and unpack to memory
        push    eax ecx
        add     ecx, mem_stream.buf
        call    [pgalloc]
        test    eax, eax
        jz      return.clear
        mov     edi, eax
        pop     ecx
        push    edi
        mov     [hOut], eax
        xor     eax, eax        ; type_mem_stream
        stosd                   ; mem_stream.type
        mov     eax, ecx
        stosd                   ; mem_stream.size
        xor     eax, eax
        stosd                   ; mem_stream.pos
        mov     eax, [esp+4]
        push    esi
        push    ecx edi
        call    read_7z_to_buf
        pop     esi ecx
        mov     ebx, [esp+12]
        cmp     [ebx+file_in_7z.bCRCDefined], 0
        jz      @f
        call    crc
        cmp     eax, [ebx+file_in_7z.FileCRC]
        jnz     return.err
@@:
        pop     esi
        xor     ecx, ecx
        xchg    ecx, [_7z.decode.mainbuf]
        xchg    ecx, [ebp+handle_7z.last_context]
        jecxz   @f
        call    [pgfree]
@@:
        and     [hOut], 0
        mov     eax, [esi+file_in_7z.folder]
        mov     [ebp+handle_7z.last_folder], eax
        mov     eax, [esp+4]
        mov     [ebp+handle_7z.last_main_stream], eax
        mov     ecx, dword [esi+file_in_7z.UnPackSize]
        add     ecx, dword [esi+file_in_7z.folderStart]
        mov     dword [ebp+handle_7z.last_pos], ecx
        mov     ecx, dword [esi+file_in_7z.UnPackSize+4]
        adc     ecx, dword [esi+file_in_7z.folderStart+4]
        mov     dword [ebp+handle_7z.last_pos+4], ecx
        call    .done
        pop     eax edi ecx
        ret
.nomemstream:
        mov     edi, eax
        push    esi
        mov     esi, handle_table_7z
        push    file_handle_7z.size
        pop     ecx
        call    alloc_handle
        pop     esi
        test    eax, eax
        jz      .clear
        mov     [eax+file_handle_7z.type], type_7z
        xor     edx, edx
        mov     dword [eax+file_handle_7z.pos], edx
        mov     dword [eax+file_handle_7z.pos+4], edx
        mov     [eax+file_handle_7z.bError], dl
        xchg    edx, [_7z.decode.mainbuf]
        mov     [eax+file_handle_7z.context], edx
        mov     [eax+file_handle_7z.mainStream], edi
        mov     [eax+file_handle_7z.base], ebp
        mov     [eax+file_handle_7z.item], esi
        push    eax
        call    .done
        pop     eax ecx
        ret

.done:
        cmp     [bPasswordDefined], 0
        jz      @f
        mov     ecx, [password_size]
        mov     [ebp+handle_7z.password_len], ecx
        mov     esi, password_unicode
        lea     edi, [ebp+handle_7z.password]
        shr     ecx, 1
        rep     movsd
        adc     ecx, ecx
        rep     movsw
@@:
        jmp     .clear

.error:
        cmp     [bPasswordDefined], 0
        jz      .realerror
        push    CancelPassBtn
        push    2
        push    aArchiveDataErrorPass_ptr
        push    1
        call    [SayErr]
        cmp     al, 1
        jnz     .clear
; user wants to re-enter password
        call    .clear
        pop     eax
        jmp     open_file_7z
.realerror:
        push    ContinueBtn
        push    1
        push    aArchiveDataError_ptr
        push    1
        call    [SayErr]
.clear:
        mov     ecx, [_7z.decode.pStreams]
        cmp     ecx, _7z.decode.pStreams_def
        jz      @f
        call    [pgfree]
@@:
        mov     ecx, [_7z.decode.mainbuf]
        jecxz   @f
        call    [pgfree]
@@:
        mov     ecx, [hOut]
        jecxz   @f
        call    [pgfree]
@@:
        cmp     esp, [_esp]
        jnz     @f
        pop     eax
@@:
        xor     eax, eax
        ret

read_7z_to_buf:
        mov     esi, [eax+streamInfo.bufDataLen]
        cmp     esi, [eax+streamInfo.bufSize]
        jnz     @f
        xor     esi, esi
        mov     [eax+streamInfo.bufDataLen], esi
@@:
        call    fillBufNozero
        mov     ecx, [eax+streamInfo.bufDataLen]
        sub     ecx, esi
        add     esi, [eax+streamInfo.bufPtr]
        mov     edx, ecx
        shr     ecx, 2
        rep     movsd
        mov     ecx, edx
        and     ecx, 3
        rep     movsb
        cmp     dword [eax+streamInfo.fullSize], 0
        jnz     read_7z_to_buf
        ret

skip_7z:
        push    edx
        mov     edx, [eax+streamInfo.bufDataLen]
        cmp     edx, [eax+streamInfo.bufSize]
        jnz     @f
        and     [eax+streamInfo.bufDataLen], 0
@@:
        pop     edx
        call    fillBufNozero
        cmp     dword [eax+streamInfo.fullSize], 0
        jnz     skip_7z
        cmp     dword [eax+streamInfo.fullSize+4], 0
        jnz     skip_7z
        ret

; unsigned __stdcall read(HANDLE hFile, void* buf, unsigned size);
read_7z:
        mov     eax, [ebx+file_handle_7z.mainStream]
        test    eax, eax
        jz      .ret    ; empty stream - return 0 bytes read
        cmp     [ebx+file_handle_7z.bError], 0
        jnz     .reterr
        mov     ecx, [ebx+file_handle_7z.base]
        mov     ecx, [ecx+handle_7z.host]
        mov     [inStream], ecx
        mov     edi, [esp+8]
        mov     ecx, [esp+12]
        mov     dword [eax+streamInfo.fullSize], ecx
        and     dword [eax+streamInfo.fullSize+4], 0
        jecxz   .nodata
        mov     [_esp], esp
        mov     [_ebp], ebp
        mov     [error_proc], .error
        mov     [clear_proc], .clear    ; should not be called
        mov     esi, [ebx+file_handle_7z.item]
        mov     edx, dword [esi+file_in_7z.UnPackSize]
        mov     esi, dword [esi+file_in_7z.UnPackSize+4]
        sub     edx, dword [ebx+file_handle_7z.pos]
        sbb     esi, dword [ebx+file_handle_7z.pos+4]
        jnz     .do
        cmp     edx, ecx
        jae     .do
        mov     dword [eax+streamInfo.fullSize], edx
.do:
        call    read_7z_to_buf
.nodata:
        sub     edi, [esp+8]
        mov     eax, edi
        add     dword [ebx+file_handle_7z.pos], eax
        adc     dword [ebx+file_handle_7z.pos+4], 0
.ret:
        ret     12

.error:
.clear:
        mov     ebx, [esp+4]
        mov     [ebx+file_handle_7z.bError], 1
        push    ContinueBtn
        push    1
        push    aArchiveDataError_ptr
        push    1
        call    [SayErr]
.reterr:
        or      eax, -1
        ret     12

; void __stdcall setpos(HANDLE hFile, __int64 pos);
setpos_7z:
        cmp     [ebx+file_handle_7z.context], 0
        jz      .ret
        mov     edi, [ebx+file_handle_7z.base]
        mov     ecx, [edi+handle_7z.host]
        mov     [inStream], ecx
        mov     [_esp], esp
        mov     [_ebp], ebp
        mov     [error_proc], read_7z.error
        mov     [clear_proc], read_7z.clear     ; should not be called
        cmp     [ebx+file_handle_7z.bError], 0
        jnz     .backward       ; if was error, force reinitialization
        mov     ecx, [esp+8]
        mov     edx, [esp+12]
        sub     ecx, dword [ebx+file_handle_7z.pos]
        sbb     edx, dword [ebx+file_handle_7z.pos+4]
        jb      .backward
; move forward - skip some data
        mov     eax, [ebx+file_handle_7z.mainStream]
        mov     dword [eax+streamInfo.fullSize], ecx
        mov     dword [eax+streamInfo.fullSize+4], edx
        call    skip_7z
        add     dword [ebx+file_handle_7z.pos], ecx
        adc     dword [ebx+file_handle_7z.pos], edx
.ret:
        ret     12
.backward:
; move backward - reinitialize and skip start data
        mov     [ebx+file_handle_7z.bError], 0
        mov     ebp, [ebx+file_handle_7z.context]
        mov     eax, [ebx+file_handle_7z.item]
        mov     ebx, [edi+handle_7z.folders]
        mov     eax, [eax+file_in_7z.folder]
        mov     ebx, [ebx+eax*4]
; initialize streamInfos for decoders
        xor     ecx, ecx
        mov     esi, [ebx+_7z.StreamsInfo.unpackSizes]
@@:
        lodsd
        mov     [ebp], eax
        lodsd
        mov     [ebp+4], eax
        and     [ebp+streamInfo.bufDataLen], 0
        push    esi
        mov     eax, [ebx+_7z.StreamsInfo.fsz+ecx*4]
        call    _7z.GetCoder
        movzx   esi, byte [eax+4]
        lea     esi, [esi+eax+17]
        push    ebx ecx edi
        mov     bl, 1
        call    [_7z.InitTable+edx*4]
        pop     edi ecx ebx esi
        mov     edx, [ebp+streamInfo.bufSize]
        mov     ebp, [ebp+streamInfo.bufPtr]
        add     ebp, edx
        inc     ecx
        cmp     ecx, [ebx+_7z.StreamsInfo.numCoders]
        jb      @b
; initialize streamInfos for input streams
        xor     ecx, ecx
        mov     esi, [ebx+_7z.StreamsInfo.startPackedStream]
        shl     esi, 3
        add     esi, [edi+handle_7z.packSizes]
        mov     edi, dword [ebx+_7z.StreamsInfo.packOffset]
        mov     edx, dword [ebx+_7z.StreamsInfo.packOffset+4]
@@:
        mov     dword [ebp+streamInfo.size], edi
        mov     dword [ebp+streamInfo.size+4], edx
        and     [ebp+streamInfo.bufDataLen], 0
        lodsd
        add     edi, eax
        mov     [ebp], eax
        lodsd
        adc     edx, eax
        mov     [ebp+4], eax
        add     ebp, 0x4000
        inc     ecx
        cmp     ecx, [ebx+_7z.StreamsInfo.numPackedStreams]
        jb      @b
        mov     ebx, [esp+4]
        mov     esi, [ebx+file_handle_7z.item]
        mov     eax, [ebx+file_handle_7z.mainStream]
        mov     ecx, dword [esi+file_in_7z.folderStart]
        add     ecx, [esp+8]
        mov     dword [eax+streamInfo.fullSize], ecx
        mov     ecx, dword [esi+file_in_7z.folderStart+4]
        adc     ecx, [esp+12]
        mov     dword [eax+streamInfo.fullSize+4], ecx
        and     dword [ebx+file_handle_7z.pos], 0
        and     dword [ebx+file_handle_7z.pos+4], 0
        and     [eax+streamInfo.bufDataLen], 0
        call    skip_7z
        mov     eax, [esp+8]
        mov     dword [ebx+file_handle_7z.pos], eax
        mov     eax, [esp+12]
        mov     dword [ebx+file_handle_7z.pos+4], eax
        ret     12

close_file_7z:
        mov     ecx, [ebx+file_handle_7z.context]
        jecxz   .ret
        cmp     [ebx+file_handle_7z.bError], 0
        jnz     @f
        push    ebp
        mov     ebp, [ebx+file_handle_7z.base]
        xchg    ecx, [ebp+handle_7z.last_context]
        mov     eax, [ebx+file_handle_7z.item]
        mov     edx, [eax+file_in_7z.folder]
        mov     [ebp+handle_7z.last_folder], edx
        mov     edx, [ebx+file_handle_7z.mainStream]
        mov     [ebp+handle_7z.last_main_stream], edx
        mov     edx, dword [eax+file_in_7z.folderStart+4]
        mov     eax, dword [eax+file_in_7z.folderStart]
        add     eax, dword [ebx+file_handle_7z.pos]
        adc     edx, dword [ebx+file_handle_7z.pos+4]
        mov     dword [ebp+handle_7z.last_pos], eax
        mov     dword [ebp+handle_7z.last_pos+4], edx
        pop     ebp
@@:
        jecxz   @f
        call    [pgfree]
@@:
        mov     esi, ebx
        call    free_handle
.ret:
        ret     4

iglobal
cur_stamp dd 0
endg

uglobal
tmp_bdfe        rb      304
endg

GetFiles_7z:
        mov     ecx, [ebp+handle_7z.NumFiles]
        test    ecx, ecx
        jz      .ret
        lea     ebx, [ebp+handle_7z.basesize]
        inc     [cur_stamp]
.loop:
        push    ecx
        mov     esi, [ebx+file_in_7z.fullname]
        mov     edx, [ebp+handle_7z.curdir]
        test    edx, edx
        jz      .incur
        mov     eax, [cur_stamp]
        mov     [edx+file_in_7z.stamp], eax
        mov     edi, [edx+file_in_7z.fullname]
        mov     ecx, [edx+file_in_7z.namelen]
        add     ecx, [edx+file_in_7z.name]
        sub     ecx, edi
        repz    cmpsb
        jnz     .cont
.incur:
        cmp     byte [esi], '/'
        jnz     @f
        inc     esi
@@:
        mov     ecx, [esp+12]   ; NumItems
        mov     edx, [esp+16]   ; items
        cmp     ecx, -1
        jz      .ok
.check:
        sub     ecx, 1
        js      .cont
        push    esi
        mov     edi, [edx]
        add     edi, 40
@@:
        lodsb
        scasb
        jnz     @f
        test    al, al
        jz      .ok2
        jmp     @b
@@:
        pop     esi
        cmp     al, '/'
        jnz     @f
        cmp     byte [edi-1], 0
        jz      .ok
@@:
        add     edx, 4
        jmp     .check
.ok2:
        pop     esi
.ok:
; add all parents directories if needed
.parloope:
        mov     ecx, [ebx+file_in_7z.parent]
        jecxz   .pardone
        add     ecx, ebp
        mov     eax, [cur_stamp]
        cmp     [ecx+file_in_7z.stamp], eax
        jz      .pardone
.parloopi:
        mov     edx, ecx
        mov     ecx, [ecx+file_in_7z.parent]
        jecxz   @f
        add     ecx, ebp
        cmp     [ecx+file_in_7z.stamp], eax
        jnz     .parloopi
@@:
        mov     [edx+file_in_7z.stamp], eax
        push    esi
        mov     eax, edx
        mov     edi, tmp_bdfe
        push    edi
        sub     esi, [ebx+file_in_7z.fullname]
        add     esi, [edx+file_in_7z.fullname]
        push    esi
        call    getattr_7z
        mov     eax, [esp+16+20]
        call    eax
        pop     esi
        test    al, al
        jz      .forced_exit
        jmp     .parloope
.pardone:
        cmp     [ebx+file_in_7z.bIsDirectory], 0
        jz      .addfile
        mov     eax, [cur_stamp]
        cmp     [ebx+file_in_7z.stamp], eax
        jz      .cont
        mov     [ebx+file_in_7z.stamp], eax
        push    esi
        mov     eax, ebx
        mov     edi, tmp_bdfe
        push    edi
        push    esi
        call    getattr_7z
        mov     eax, [esp+16+20]
        call    eax
        pop     esi
        test    al, al
        jz      .forced_exit
        jmp     .cont
.addfile:
        push    ebx esi
        push    11h
        pop     edi
        mov     eax, ebx
        call    open_file_7z
        pop     esi ebx
        test    eax, eax
        jz      .cont
        push    eax
        push    eax
        mov     edi, tmp_bdfe
        push    edi
        push    esi
        mov     eax, ebx
        call    getattr_7z
        mov     eax, [esp+20+16]
        call    eax
        pop     ecx
        push    eax
        push    ebx
        push    ecx
        call    myclose
        pop     ebx
        pop     eax
        test    al, al
        jz      .forced_exit
.cont:
        add     ebx, file_in_7z.size
        pop     ecx
        dec     ecx
        jnz     .loop
.ret:
        ret     20
.forced_exit:
        pop     ecx
        jmp     .ret
