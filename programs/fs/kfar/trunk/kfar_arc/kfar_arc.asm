;
; project name:         KFar_Arc - plugin for KFar, which supports various archives
; target platform:      KolibriOS
; compiler:             FASM 1.67.14
; version:              0.1
; last update:          2007-07-11 (Jul 11, 2007)
; minimal KFar version: 0.4
; minimal kernel:       no limit
;
; author:               Diamond
; email:                diamondz@land.ru
; web:                  http://diamondz.land.ru
;

; standard start of Kolibri dynamic library
format MS COFF
public EXPORTS

section '.flat' code readable align 16

; include auxiliary procedures
include 'kglobals.inc'          ; iglobal/uglobal
include 'lang.inc'              ; define language for localized strings
include 'crc.inc'               ; CRC32 calculation
include 'sha256.inc'            ; SHA-256 hash algorithm
include 'aes.inc'               ; AES crypto algorithm
; include main code for archives loading
include '7z.inc'                ; *.7z
include 'lzma.inc'              ; LZMA-decoder for *.7z
include 'ppmd.inc'              ; PPMD-decoder for *.7z
include '7zbranch.inc'          ; branch filters for *.7z
include '7zaes.inc'             ; AES cryptor for *.7z

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;; Interface for KFar ;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
virtual at 0
kfar_info_struc:
.lStructSize    dd      ?
.kfar_ver       dd      ?
.open           dd      ?
.read           dd      ?
.write          dd      ?
.seek           dd      ?
.flush          dd      ?
.filesize       dd      ?
.close          dd      ?
.pgalloc        dd      ?
.pgrealloc      dd      ?
.pgfree         dd      ?
.getfreemem     dd      ?
.pgalloc2       dd      ?
.pgrealloc2     dd      ?
.pgfree2        dd      ?
.menu           dd      ?
.menu_centered_in dd    ?
.DialogBox      dd      ?
.SayErr         dd      ?
.Message        dd      ?
.cur_console_size dd    ?
end virtual

; int __stdcall plugin_load(kfar_info* info);
; Initialization of plugin + Save used KFar functions.
plugin_load:
        mov     eax, [esp+4]
        mov     [kfar_info], eax
        push    [eax+kfar_info_struc.read]
        pop     [read]
        push    [eax+kfar_info_struc.seek]
        pop     [seek]
        push    [eax+kfar_info_struc.close]
        pop     [close]
        lea     esi, [eax+kfar_info_struc.DialogBox]
        mov     edi, DialogBox
        movsd
        movsd
        movsd
        movsd
        lea     esi, [eax+kfar_info_struc.pgalloc]
        mov     edi, pgalloc
        movsd
        movsd
        movsd
        movsd
        call    init_crc_table
        call    init_aes
        call    init_ppmd
        xor     eax, eax        ; success
        ret     4

; HANDLE __stdcall OpenFilePlugin(HANDLE basefile, const char* name,
;       const void* attr, const void* data, int datasize);
; This function is called when user presses Enter (or Ctrl+PgDn) on file.
; Plugin tests whether given file is of supported type
;   and if so, loads information and returns
;   handle to be used in subsequent calls to ReadFolder, SetFolder and so on.
OpenFilePlugin:
        mov     [bPasswordDefined], 0
        mov     esi, [esp+16]
        mov     ebp, [esp+4]
; test for 7z archive
        cmp     dword [esp+20], 20h     ; minimal size of 7z archive is 20h bytes
        jb      .no_7z
        cmp     word [esi], '7z'                ; signature, part 1
        jnz     .no_7z
        cmp     dword [esi+2], 0x1C27AFBC       ; signature, part 2
        jnz     .no_7z
        call    open_7z
        ret     20
.no_7z:
        xor     eax, eax
        ret     20

; Handle of plugin in kfar_arc is as follow:
virtual at 0
handle_common:
.type           dd      ?
.root.subfolders        dd      ?
.root.subfolders.end    dd      ?
.root.subfiles          dd      ?
.root.subfiles.end      dd      ?
.root.NumSubItems       dd      ?
; ... some plugin-specific data follows ...
end virtual

; and for each archive item there is one file info structure, which begins as follow:
virtual at 0
file_common:
.fullname       dd      ?       ; pointer to cp866 string
.name           dd      ?       ; name without path (end of .fullname)
.namelen        dd      ?       ; strlen(.name)
.bIsDirectory   db      ?
.bPseudoFolder  db      ?
                rb      2
.parent         dd      ?       ; pointer to parent directory record
.subfolders     dd      ?       ; head of L2-list of subfolders [for folders]
.subfolders.end dd      ?
.subfiles       dd      ?       ; head of L2-list of files [for folders]
.subfiles.end   dd      ?
.NumSubItems    dd      ?
.next           dd      ?       ; next item in list of subfolders/files
.prev           dd      ?       ; previous item in list of subfolders/files
end virtual

; void __stdcall ClosePlugin(HANDLE hPlugin);
; This function frees all resources allocated in OpenFilePlugin.
ClosePlugin:
        mov     eax, [esp+4]    ; get hPlugin
        mov     eax, [eax]      ; hPlugin is pointer to internal data structure
                                ; first dword is archive type (type_xxx constants)
        dec     eax             ; types start from 1
        jmp     dword [ClosePluginTable+eax*4]

; int __stdcall ReadFolder(HANDLE hPlugin,
;       unsigned dirinfo_start, unsigned dirinfo_size, void* dirdata);
ReadFolder:
        mov     eax, [esp+4]
        mov     eax, [eax]
        dec     eax
        jmp     dword [ReadFolderTable+eax*4]

; bool __stdcall SetFolder(HANDLE hPlugin,
;       const char* relative_path, const char* absolute_path);
SetFolder:
        mov     eax, [esp+4]
        mov     eax, [eax]
        dec     eax
        jmp     dword [SetFolderTable+eax*4]

; void __stdcall GetFiles(HANDLE hPlugin, int NumItems, void* items[],
;       void* addfile, void* adddir);
;       bool __stdcall addfile(const char* name, void* bdfe_info, HANDLE hFile);
;       bool __stdcall adddir(const char* name, void* bdfe_info);
GetFiles:
        mov     ebp, [esp+4]
        mov     eax, [ebp]
        dec     eax
        jmp     dword [GetFilesTable+eax*4]

; void __stdcall GetOpenPluginInfo(HANDLE hPlugin, OpenPluginInfo* info);
GetOpenPluginInfo:
        mov     eax, [esp+8]    ; get info ptr
        mov     byte [eax], 3   ; flags: add non-existing '..' entry automatically
                                ;        use GetFiles for copying
        ret     8

; int __stdcall getattr(HANDLE hPlugin, const char* filename, void* info);
mygetattr:
        call    lookup_file_name
        test    eax, eax
        jz      @f
        mov     edx, [ebp]
        dec     edx
        mov     edi, [esp+12]   ; info ptr
        call    dword [getattrTable+edx*4]
        xor     eax, eax
        ret     12
@@:
        mov     al, 5   ; ERROR_FILE_NOT_FOUND
        ret     12

; HANDLE __stdcall open(HANDLE hPlugin, const char* filename, int mode);
myopen:
        call    lookup_file_name
        test    eax, eax
        jz      @f
        mov     edx, [ebp]
        dec     edx
        mov     edi, [esp+12]   ; mode
        call    dword [openTable+edx*4]
@@:
        ret     12

; unsigned __stdcall read(HANDLE hFile, void* buf, unsigned size);
myread:
        mov     ebx, [esp+4]
        mov     eax, [ebx]
        jmp     dword [readTable+eax*4]

; void __stdcall setpos(HANDLE hFile, __int64 pos);
mysetpos:
        mov     ebx, [esp+4]
        mov     eax, [ebx]
        jmp     dword [setposTable+eax*4]

; void __stdcall close(HANDLE hFile);
myclose:
        mov     ebx, [esp+4]
        mov     eax, [ebx]
        jmp     dword [closeTable+eax*4]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;; Auxiliary procedures ;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; return.err and return.clear are labels to jmp if something is invalid
; the caller must previously define [_esp], [_ebp] and [error_proc], [clear_proc]
return.err:
        mov     esp, [_esp]
        mov     ebp, [_ebp]
        jmp     [error_proc]
return.clear:
        mov     esp, [_esp]
        mov     ebp, [_ebp]
        jmp     [clear_proc]

; data for following routine
iglobal
align 4
_24             dd      24
_60             dd      60
_10000000       dd      10000000
days400year     dd      365*400+100-4+1
days100year     dd      365*100+25-1
days4year       dd      365*4+1
days1year       dd      365
months          dd      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
months2         dd      31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
_400            dd      400
_100            dd      100
endg

; Convert QWORD FILETIME to BDFE format.
ntfs_datetime_to_bdfe:
; edx:eax = number of 100-nanosecond intervals since January 1, 1601, in UTC
        push    eax
        mov     eax, edx
        xor     edx, edx
        div     [_10000000]
        xchg    eax, [esp]
        div     [_10000000]
        pop     edx
; edx:eax = number of seconds since January 1, 1601
        push    eax
        mov     eax, edx
        xor     edx, edx
        div     [_60]
        xchg    eax, [esp]
        div     [_60]
        mov     [edi], dl
        pop     edx
; edx:eax = number of minutes
        div     [_60]
        mov     [edi+1], dl
; eax = number of hours (note that 2^64/(10^7*60*60) < 2^32)
        xor     edx, edx
        div     [_24]
        mov     [edi+2], dl
        mov     [edi+3], byte 0
; eax = number of days since January 1, 1601
        xor     edx, edx
        div     [days400year]
        imul    eax, 400
        add     eax, 1601
        mov     [edi+6], ax
        mov     eax, edx
        xor     edx, edx
        div     [days100year]
        cmp     al, 4
        jnz     @f
        dec     eax
        add     edx, [days100year]
@@:
        imul    eax, 100
        add     [edi+6], ax
        mov     eax, edx
        xor     edx, edx
        div     [days4year]
        shl     eax, 2
        add     [edi+6], ax
        mov     eax, edx
        xor     edx, edx
        div     [days1year]
        cmp     al, 4
        jnz     @f
        dec     eax
        add     edx, [days1year]
@@:
        add     [edi+6], ax
        push    esi edx
        mov     esi, months
        movzx   eax, word [edi+6]
        test    al, 3
        jnz     .noleap
        xor     edx, edx
        push    eax
        div     [_400]
        pop     eax
        test    edx, edx
        jz      .leap
        xor     edx, edx
        div     [_100]
        test    edx, edx
        jz      .noleap
.leap:
        mov     esi, months2
.noleap:
        pop     edx
        xor     eax, eax
        inc     eax
@@:
        sub     edx, [esi]
        jb      @f
        add     esi, 4
        inc     eax
        jmp     @b
@@:
        add     edx, [esi]
        pop     esi
        inc     edx
        mov     [edi+4], dl
        mov     [edi+5], al
        add     edi, 8
        ret

; By given array of files information, initialize links between them
; ("[folder] contains [item]" relations).
; Information structure must be compatible with 'file_common'.
; Size of information structure is in [esp+4].
init_file_links:
; in: edx->file infos, ebx = number of files, [esp+4] = size,
;     edi->{dd root.subfolders, dd root.subfolders.end,
;           dd root.subfiles, dd root.subfiles.end, dd root.NumItems}
        xor     eax, eax
        mov     [.free], eax
        push    edi
        stosd
        stosd
        stosd
        stosd
        stosd
        pop     edi
; Loop through all files
.mainloop:
        dec     ebx
        js      .mainloopdone
; Parse file name
; esi->current character in name
; dword [esp] = start of current component in file name
; ecx->{dd curdir.subfolders, dd curdir.subfolders.end,
;       dd curdir.subfiles, dd curdir.subfiles.end}
        mov     esi, [edx+file_common.fullname]
        mov     ecx, edi
.parseloop:
        push    esi
.parsename:
        lodsb
        test    al, al
        jz      @f
        cmp     al, '/'
        jnz     .parsename
@@:
; we have found next component of current name; look for it in current directory
        sub     esi, [esp]
        dec     esi     ; esi = strlen(component)
        cmp     esi, 259
        jbe     @f
        push    ContinueBtn
        push    1
        push    aNameTooLong_ptr
        push    1
        call    [SayErr]
        jmp     return.clear
@@:
        push    ecx
        mov     eax, [ecx]      ; eax->subfolders list
        mov     ecx, esi
.scansubfolders:
        test    eax, eax
        jz      .nofolder
        add     eax, [hOut]
        cmp     [eax+file_common.namelen], ecx
        jnz     .scancont
        mov     esi, [esp+4]
        push    ecx edi
        mov     edi, [eax+file_common.name]
        repz    cmpsb
        pop     edi ecx
        jz      .subfound
.scancont:
        mov     eax, [eax+file_common.next]
        jmp     .scansubfolders
.subfound:
; found subfolder, set it as current and continue parsing name
        add     [esp+4], ecx
        pop     ecx
        lea     ecx, [eax+file_common.subfolders]
        pop     esi
        lodsb
        test    al, al
        jnz     .parseloop
; that was the last component of the name, and we have found subfolder
; so found subfolder is a virtual subfolder and must be replaced with current
; name
        mov     eax, [ecx-file_common.subfolders+file_common.namelen]
        mov     [edx+file_common.namelen], eax
        sub     esi, eax
        dec     esi
        mov     [edx+file_common.name], esi
        sub     edx, [hOut]     ; convert pointer to relative
; replace item in L2-list
        mov     eax, [ecx-file_common.subfolders+file_common.prev]
        test    eax, eax
        jnz     .1
        mov     eax, [ecx-file_common.subfolders+file_common.parent]
        sub     eax, file_common.next - file_common.subfolders
        jnc     .1
        lea     eax, [edi-file_common.next]
        jmp     .2
.1:
        add     eax, [hOut]
.2:
        mov     [eax+file_common.next], edx
        mov     eax, [ecx-file_common.subfolders+file_common.next]
        test    eax, eax
        jnz     .3
        mov     eax, [ecx-file_common.subfolders+file_common.parent]
        sub     eax, file_common.prev - file_common.subfolders.end
        jnc     .3
        lea     eax, [edi-file_common.prev+4]
        jmp     .4
.3:
        add     eax, [hOut]
.4:
        mov     [eax+file_common.prev], edx
; correct parent links in childrens
        mov     eax, [ecx]
@@:
        test    eax, eax
        jz      @f
        add     eax, [hOut]
        mov     [eax+file_common.parent], edx
        mov     eax, [eax+file_common.next]
        jmp     @b
@@:
        mov     eax, [ecx+8]
@@:
        test    eax, eax
        jz      @f
        add     eax, [hOut]
        mov     [eax+file_common.parent], edx
        mov     eax, [eax+file_common.next]
        jmp     @b
@@:
        add     edx, [hOut]
; set children links
        mov     eax, [ecx]
        mov     [edx+file_common.subfolders], eax
        mov     eax, [ecx+4]
        mov     [edx+file_common.subfolders.end], eax
        mov     eax, [ecx+8]
        mov     [edx+file_common.subfiles], eax
        mov     eax, [ecx+12]
        mov     [edx+file_common.subfiles.end], eax
        mov     eax, [ecx+16]
        mov     [edx+file_common.NumSubItems], eax
; set prev/next links
        mov     eax, [ecx-file_common.subfolders+file_common.next]
        mov     [edx+file_common.next], eax
        mov     eax, [ecx-file_common.subfolders+file_common.prev]
        mov     [edx+file_common.prev], eax
; add old item to list of free items
uglobal
align 4
init_file_links.free    dd      ?
endg
        sub     ecx, file_common.subfolders
        mov     eax, [.free]
        mov     [ecx], eax
        sub     ecx, [hOut]
        mov     [.free], ecx
        jmp     .mainloopcont
.nofolder:
        mov     eax, edx
        mov     esi, [esp+4]
        cmp     byte [esi+ecx], 0
        jz      .newitem
; the current item is as 'dir1/item1' and 'dir1' has not been found
; allocate virtual subfolder 'dir1'
        mov     eax, [init_file_links.free]
        test    eax, eax
        jz      .realloc
        add     eax, [hOut]
        push    dword [eax]
        pop     [init_file_links.free]
        jmp     .allocated
.realloc:
; there is no free space, so reallocate [hOut] block
        mov     eax, [hOut]
        sub     [esp], eax      ; make pointers relative
        sub     edx, eax
        sub     edi, eax
        push    ecx
        mov     ecx, [hOut.allocated]
        add     ecx, [esp+12+4]
        mov     [hOut.allocated], ecx
        push    ecx
        and     ecx, 0xFFF
        cmp     ecx, [esp+16+4]
        pop     ecx
        ja      @f
        push    edx
        mov     edx, eax
        call    [pgrealloc]
        pop     edx
        test    eax, eax
        jnz     @f
        mov     ecx, [hOut]
        call    [pgfree]
        mov     esp, [_esp]
        or      eax, -1
        ret
@@:
        pop     ecx
        mov     [hOut], eax
        add     [esp], eax      ; make pointers absolute
        add     edx, eax
        add     edi, eax
        add     eax, [hOut.allocated]
        sub     eax, [esp+8+4]
.allocated:
; eax -> new item
        mov     [eax+file_common.bIsDirectory], 1
        mov     [eax+file_common.bPseudoFolder], 1
.newitem:
        mov     [eax+file_common.namelen], ecx
        pop     ecx
        pop     esi
; ecx = parent item, eax = current item
        mov     [eax+file_common.name], esi
        inc     dword [ecx+16]  ; new item in parent folder
        push    ecx
; add new item to end of L2-list
        and     [eax+file_common.next], 0
        cmp     [eax+file_common.bIsDirectory], 0
        jnz     @f
        add     ecx, 8
@@:
        push    eax
        sub     eax, [hOut]
        cmp     dword [ecx], 0
        jnz     @f
        mov     [ecx], eax
@@:
        xchg    eax, [ecx+4]
        xchg    eax, ecx
        pop     eax
        mov     [eax+file_common.prev], ecx
        jecxz   @f
        add     ecx, [hOut]
        sub     eax, [hOut]
        mov     [ecx+file_common.next], eax
        add     eax, [hOut]
@@:
        pop     ecx
; set parent link
        and     [eax+file_common.parent], 0
        cmp     ecx, edi
        jz      @f
        sub     ecx, file_common.subfolders
        sub     ecx, [hOut]
        mov     [eax+file_common.parent], ecx
@@:
; set current directory to current item
        lea     ecx, [eax+file_common.subfolders]
; if that was not last component, continue parse name
        add     esi, [eax+file_common.namelen]
        lodsb
        test    al, al
        jnz     .parseloop
.mainloopcont:
; continue main loop
        add     edx, [esp+4]
        jmp     .mainloop
.mainloopdone:
; Loop done.
        ret     4

; This subroutine is called by getattr and open.
; This subroutine looks for file name and returns NULL or pointer to file info record.
lookup_file_name:
        mov     ebp, [esp+8]    ; hPlugin
        mov     esi, [esp+12]   ; filename
        lea     edi, [ebp+handle_common.root.subfolders]
        xor     eax, eax
; KFar operates with absolute names, skip first '/'
        cmp     byte [esi], '/'
        jnz     .notfound
        inc     esi
.mainloop:
; get next component of name
        push    -1
        pop     ecx
@@:
        inc     ecx
        cmp     byte [esi+ecx], '/'
        jz      @f
        cmp     byte [esi+ecx], 0
        jnz     @b
@@:
; esi->component, ecx=length
; scan for required item in subfolders list
        push    -1
        mov     eax, [edi]      ; .subfolders
.scan1:
        test    eax, eax
        jz      .notfound1
        add     eax, ebp
        cmp     [eax+file_common.namelen], ecx
        jnz     .cont1
        push    ecx esi edi
        mov     edi, [eax+file_common.name]
        repz    cmpsb
        pop     edi esi ecx
        jz      .found1
.cont1:
        mov     eax, [eax+file_common.next]
        jmp     .scan1
.notfound1:
        pop     edx
; if this is last component in file name, scan in subfiles list
        cmp     byte [esi+ecx], al
        jnz     .notfound
        inc     edx
        jnz     .notfound
        mov     eax, [edi+8]    ; .subfiles
        push    edx
        jmp     .scan1
.found1:
        pop     edi
; item is found, go to next component
        lea     edi, [eax+file_common.subfolders]
        lea     esi, [esi+ecx+1]
        cmp     byte [esi-1], 0
        jnz     .mainloop
; this was the last component
.notfound:
        ret

; Memory streams handling.
; Archive handlers create memory stream for small files:
; size of which is not greater than (free RAM size)/4 and
; not greater than following constant...
;LIMIT_FOR_MEM_STREAM = 2*1024*1024
; ...if it is defined. Now the definition is commented:
; if user has many physical memory, why not to use it?

virtual at 0
mem_stream:
.type   dd      ?       ; type_mem_stream
.size   dd      ?
.pos    dd      ?
.buf:
end virtual

; unsigned __stdcall read(ebx = HANDLE hFile, void* buf, unsigned size);
read_mem_stream:
        mov     eax, [esp+12]
        mov     ecx, [ebx+mem_stream.size]
        sub     ecx, [ebx+mem_stream.pos]
        jnc     @f
        xor     ecx, ecx
@@:
        cmp     eax, ecx
        jb      @f
        mov     eax, ecx
@@:
        mov     ecx, eax
        lea     esi, [ebx+mem_stream.buf]
        add     esi, [ebx+mem_stream.pos]
        add     [ebx+mem_stream.pos], eax
        mov     edi, [esp+8]
        mov     edx, ecx
        shr     ecx, 2
        rep     movsd
        mov     ecx, edx
        and     ecx, 3
        rep     movsb
        ret     12

; void __stdcall setpos(ebx = HANDLE hFile, __int64 pos);
setpos_mem_stream:
        mov     eax, [esp+8]
        mov     [ebx+mem_stream.pos], eax
        ret     12

; void __stdcall close(ebx = HANDLE hFile);
close_mem_stream:
        mov     ecx, ebx
        call    [pgfree]
        ret     4

; Allocate handle for file
; esi -> handle table, ecx = size of handle
alloc_handle:
; Handle table is L2-list of allocated pages.
; Scan for free entry
        mov     edx, esi
@@:
        mov     edx, [edx]
        cmp     edx, esi
        jz      .alloc_new
        mov     eax, [edx+8]            ; head of L1-list of free entries
        test    eax, eax                ; has free entry?
        jz      @b
; we have found allocated page with free entry; allocate entry and return
        inc     dword [edx+12]          ; number of busy entries
        push    dword [eax]
        pop     dword [edx+8]
.ret:
        ret
.alloc_new:
; no free pages; get new page and initialize
        push    ecx
        mov     ecx, 0x1000
        call    [pgalloc]
        pop     ecx
        test    eax, eax
        jz      .ret
; insert new page to start of L2-list
        mov     edx, [esi]
        mov     [eax], edx
        mov     [esi], eax
        mov     [eax+4], esi
        mov     [edx+4], eax
        mov     dword [eax+12], 1       ; 1 allocated entry
; initialize list of free entries
        lea     edx, [eax+16]
        push    edx     ; save return value
        add     edx, ecx
        mov     [eax+8], edx
        add     eax, 0x1000
@@:
        mov     esi, edx
        add     edx, ecx
        mov     [esi], edx
        cmp     edx, eax
        jb      @b
        and     dword [esi], 0
        pop     eax
        ret

; Free handle allocated in previous procedure
; esi = handle
free_handle:
        mov     ecx, esi
        and     ecx, not 0xFFF  ; get page
; add entry to head of L1-list of free entries
        mov     eax, [ecx+8]
        mov     [esi], eax
        mov     [ecx+8], esi
        dec     dword [ecx+12]  ; decrement number of allocated entries
        jnz     .ret
; delete page from common L2-list
        mov     eax, [ecx]
        mov     edx, [ecx+4]
        mov     [eax+4], edx
        mov     [edx], eax
; free page
        call    [pgfree]
.ret:
        ret

; Ask user to enter password.
; Out: ZF set <=> user pressed Esc
;      'password_ansi', 'password_unicode', 'password_size' filled
query_password:
        cmp     [bPasswordDefined], 0
        jnz     .ret
        mov     edi, password_data
        mov     eax, password_maxlen
        stosd           ; maximum length
        xor     eax, eax
        stosd           ; start of visible part
        stosd           ; position of cursor
        stosb           ; initial state: empty string
        mov     eax, [cur_console_size]
        mov     eax, [eax]      ; get current console width
        sub     eax, 12
        mov     edi, password_dlg
        mov     [edi+password_dlg.width-password_dlg], eax
        dec     eax
        dec     eax
        mov     [edi+password_dlg.width1-password_dlg], eax
        mov     [edi+password_dlg.width2-password_dlg], eax
        push    edi
        call    [DialogBox]
        inc     eax
        jz      .ret
; convert ANSI-cp866 to UNICODE string; also calculate 'password_size'
        mov     esi, password_ansi
        mov     edi, password_unicode
        or      [password_size], -1
.cvt:
        inc     [password_size]
        lodsb
        mov     ah, 0
; 0x00-0x7F - trivial map
        cmp     al, 0x80
        jb      .symb
; 0x80-0xAF -> 0x410-0x43F
        cmp     al, 0xB0
        jae     @f
        add     ax, 0x410-0x80
        jmp     .symb
@@:
; 0xE0-0xEF -> 0x440-0x44F
        cmp     al, 0xE0
        jb      .unk
        cmp     al, 0xF0
        jae     @f
        add     ax, 0x440-0xE0
        jmp     .symb
@@:
; 0xF0 -> 0x401
; 0xF1 -> 0x451
        cmp     al, 'Ё'
        jz      .yo1
        cmp     al, 'ё'
        jz      .yo2
.unk:
        mov     al, '_'
        jmp     .symb
.yo1:
        mov     ax, 0x401
        jmp     .symb
.yo2:
        mov     ax, 0x451
.symb:
        stosw
        test    al, al
        jnz     .cvt
        inc     [bPasswordDefined]      ; clears ZF flag
.ret:
        ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;; Initialized data ;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; export table
align 4
EXPORTS:
        dd      aVersion,       1
        dd      aPluginLoad,    plugin_load
        dd      aOpenFilePlugin,OpenFilePlugin
        dd      aClosePlugin,   ClosePlugin
        dd      aReadFolder,    ReadFolder
        dd      aSetFolder,     SetFolder
        dd      aGetFiles,      GetFiles
        dd      aGetOpenPluginInfo, GetOpenPluginInfo
        dd      aGetattr,       mygetattr
        dd      aOpen,          myopen
        dd      aRead,          myread
        dd      aSetpos,        mysetpos
        dd      aClose,         myclose
        dd      0

; exported names
aVersion        db      'version',0
aPluginLoad     db      'plugin_load',0
aOpenFilePlugin db      'OpenFilePlugin',0
aClosePlugin    db      'ClosePlugin',0
aReadFolder     db      'ReadFolder',0
aSetFolder      db      'SetFolder',0
aGetFiles       db      'GetFiles',0
aGetOpenPluginInfo db   'GetOpenPluginInfo',0
aGetattr        db      'getattr',0
aOpen           db      'open',0
aRead           db      'read',0
aSetpos         db      'setpos',0
aClose          db      'close',0

; common strings
if lang eq ru
aContinue       db      'Продолжить',0
aCancel         db      'Отмена',0
aHeaderError    db      'Ошибка в заголовке архива',0
aReadError      db      'Ошибка чтения',0
aNoFreeRam      db      'Недостаточно свободной оперативной памяти',0
aEncodingProblem db     'Проблема с кодировкой',0
aEncodingProblem_str db 'Имена некоторых файлов в архиве содержат символы,',0
.2              db      'не представимые в кодировке cp866.',0
.3              db      'Эти символы будут заменены на подчёркивания.',0
aEnterPassword  db      'Введите пароль:',0
aEnterPasswordLen = $ - aEnterPassword - 1
aEnterPasswordTitle db  'Ввод пароля',0
aArchiveDataError db    'Ошибка в данных архива',0
aArchiveDataErrorPass db 'Ошибка в данных архива или неверный пароль',0
aChangePass     db      'Ввести пароль',0
aNameTooLong    db      'Слишком длинное имя',0
else
aContinue       db      'Continue',0
aCancel         db      'Cancel',0
aHeaderError    db      'Invalid archive header',0
aReadError      db      'Read error',0
aNoFreeRam      db      'There is not enough of free RAM',0
aEncodingProblem db     'Encoding problem',0
aEncodingProblem_str db 'The names of some files in the archive contain',0
.2              db      'characters which can not be represented in cp866.',0
.3              db      'Such characters will be replaced to underscores.',0
aEnterPassword  db      'Enter password:',0
aEnterPasswordLen = $ - aEnterPassword - 1
aEnterPasswordTitle db  'Get password',0
aArchiveDataError db    'Error in archive data',0
aArchiveDataErrorPass db 'Error in archive data or incorrect password',0
aChangePass     db      'Enter password',0
aNameTooLong    db      'Name is too long',0
end if

; kfar_arc supports [hmm... will support...] many archive types.
; OpenFilePlugin looks for supported archive signature and gives control
;   to concrete handler if found.
; Other functions just determine type of opened archive and jumps to corresponding handler.
type_mem_stream = 0     ; memory stream - for file handles (returned from 'open')
type_7z = 1

; archive functions (types start from type_7z)
align 4
ClosePluginTable:
        dd      close_7z
ReadFolderTable:
        dd      ReadFolder_7z
SetFolderTable:
        dd      SetFolder_7z
GetFilesTable:
        dd      GetFiles_7z
getattrTable:
        dd      getattr_7z
openTable:
        dd      open_file_7z

; file functions (types start from type_mem_stream)
readTable:
        dd      read_mem_stream
        dd      read_7z
setposTable:
        dd      setpos_mem_stream
        dd      setpos_7z
closeTable:
        dd      close_mem_stream
        dd      close_file_7z

; pointers for SayErr and Message
ContinueBtn     dd      aContinue
HeaderError_ptr dd      aHeaderError
aReadError_ptr  dd      aReadError
aNoFreeRam_ptr  dd      aNoFreeRam
aEncodingProblem_str_ptr:
                dd      aEncodingProblem_str
                dd      aEncodingProblem_str.2
                dd      aEncodingProblem_str.3
aNameTooLong_ptr dd     aNameTooLong
aArchiveDataError_ptr dd aArchiveDataError
aArchiveDataErrorPass_ptr dd aArchiveDataErrorPass
CancelPassBtn   dd      aCancel
                dd      aChangePass

; "enter password" dialog for KFar
password_dlg:
        dd      1       ; use standard dialog colors
        dd      -1      ; center window by x
        dd      -1      ; center window by y
.width  dd      ?       ; width (will be filled according to current console width)
        dd      2       ; height
        dd      4, 2    ; border size
        dd      aEnterPasswordTitle     ; title
        dd      ?       ; colors (will be set by KFar)
        dd      0, 0    ; reserved for DlgProc
        dd      2       ; 2 controls
; the string "enter password"
        dd      1       ; type: static
        dd      1,0     ; upper-left position
.width1 dd      ?,0     ; bottom-right position
        dd      aEnterPassword  ; data
        dd      0       ; flags
; editbox for password
        dd      3       ; type: edit
        dd      1,1     ; upper-left position
.width2 dd      ?,0     ; bottom-right position
        dd      password_data   ; data
        dd      2Ch     ; flags

IncludeIGlobals

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;; Uninitialized data ;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

section '.udata' data readable writable align 16
kfar_info       dd      ?
crc_table       rd      256
_esp            dd      ?
_ebp            dd      ?
bufsize         dd      ?
bufptr          dd      ?
bufend          dd      ?
buffer          rb      1024
inStream        dd      ?
hOut            dd      ?
.allocated      dd      ?

error_proc      dd      ?
clear_proc      dd      ?

; import from kfar
read            dd      ?
seek            dd      ?
close           dd      ?
pgalloc         dd      ?
pgrealloc       dd      ?
pgfree          dd      ?
getfreemem      dd      ?
DialogBox       dd      ?
SayErr          dd      ?
Message         dd      ?
cur_console_size dd     ?

; data for editbox in kfar dialog
password_maxlen = 512
password_data:
.maxlen         dd      ?
.pos            dd      ?
.start          dd      ?
password_ansi   rb      password_maxlen+1
bPasswordDefined db     ?

; converted password
password_unicode rw     password_maxlen+1
password_size   dd      ?

IncludeUGlobals

bWasWarning     db      ?
