format PE GUI 4.0 at 400000h
section '.text' code readable executable
entry start

include 'config_resource.inc'

start:
        call    [GetProcessHeap]
        mov     [hHeap], eax
        xor     ebx, ebx
; SHBrowseForFolder requires CoInitialize (according to docs)
        push    2
        push    ebx
        call    [CoInitializeEx]
        mov     edi, full_ini_name
        push    1000
        push    edi
        push    ebx
        call    [GetModuleFileNameA]
@@:
        inc     edi
        cmp     byte [edi-1], 0
        jnz     @b
@@:
        dec     edi
        cmp     byte [edi-1], '\'
        jnz     @b
        mov	[pathzeroptr], edi
        mov     esi, IniFileName
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        push    ebx             ; hTemplateFile
        push    80h             ; dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL
        push    1               ; dwCreationDisposition = CREATE_NEW
        push    ebx             ; lpSecurityAttributes
        push    ebx             ; dwShareMode
        push    40000000h       ; dwDesiredAccess = GENERIC_WRITE
        push    full_ini_name   ; lpFileName
        call    [CreateFileA]
        mov     esi, eax
        inc     eax
        jz      .ini_exists
        push    eax
        mov     eax, esp
        push    ebx             ; lpOverlapped
        push    eax             ; lpNumberOfBytesWritten
        push    ini_size        ; nNumberOfBytesToWrite
        push    ini             ; lpBuffer
        push    esi             ; hFile
        call    [WriteFile]
        pop     eax
        push    esi
        call    [CloseHandle]
.ini_exists:
        push    ebx
        push    DlgProc
        push    ebx
        push    IDD_DIALOG1
        push    400000h
        call    [DialogBoxParamA]
        push    eax
        call    [CoUninitialize]
        call    [ExitProcess]

DlgProc:
        push    ebx
        mov     ebx, [esp+8]
        mov     eax, [esp+12]
        cmp     eax, 10h        ; WM_CLOSE
        jz      .close
        sub     eax, 110h       ; WM_INITDIALOG
        jz      .init
        dec     eax             ; WM_COMMAND
        jz      .command
        pop     ebx
        xor     eax, eax
        ret     16
.close:
        push    0
        push    ebx
        call    [EndDialog]
.true:
        pop     ebx
        xor     eax, eax
        inc     eax
        ret     16
.command:
        cmp     word [esp+16], 2        ; IDCANCEL
        jz      .close
        cmp     word [esp+16], IDC_HD_DEV
        jnz     .no_hd_dev
        cmp     word [esp+18], 1        ; LBN_SELCHANGE
        jnz     .true
.lastdel:
        call    OnSelectHdDev
        jmp     .true
.no_hd_dev:
        cmp     word [esp+16], IDC_HD_PART
        jnz     .no_hd_part
        cmp     word [esp+18], 1
        jnz     .true
        cmp     [cur_hd_dev], -1
        jz      .true
        call    OnSelectHdPart
        jmp     .true
.no_hd_part:
        mov     eax, IDC_SKIN
        mov     [filter], skn_filter
        cmp     word [esp+16], IDC_SKIN_BROWSE
        jz      @f
        mov     eax, IDC_FONT1
        mov     [filter], mt_filter
        cmp     word [esp+16], IDC_FONT1_BROWSE
        jz      @f
        mov     eax, IDC_FONT2
        cmp     word [esp+16], IDC_FONT2_BROWSE
        jnz     .nofilebrowse
@@:
        push    eax
        push    512
        push    cur_ini_param
        push    eax
        push    ebx
        call    [GetDlgItemTextA]
        push    ofn
        call    [GetOpenFileNameA]
        test    eax, eax
        pop     eax
        jz      .true
        push    cur_ini_param
        push    eax
        push    ebx
        call    [SetDlgItemTextA]
        jmp     .true
.nofilebrowse:
        mov     eax, IDC_RAMDISK
        cmp     word [esp+16], IDC_RAMDISK_BROWSE
        jz      @f
        mov     eax, IDC_PARTITION
        cmp     word [esp+16], IDC_PARTITION_BROWSE
        jnz     .nodirbrowse
@@:
        push    eax
        push    512
        push    cur_ini_param
        push    eax
        push    ebx
        call    [GetDlgItemTextA]
        push    bi
        call    [SHBrowseForFolder]
        test    eax, eax
        pop     ecx
        jz      .true
        push    ecx eax
        push    cur_ini_param
        push    eax
        call    [SHGetPathFromIDList]
        test    eax, eax
        pop     eax ecx
        jz      .nodir
        push    eax
        push    cur_ini_param
        push    ecx
        push    ebx
        call    [SetDlgItemTextA]
        pop     eax
.nodir:
        push    eax
        push    eax
        push    esp
        call    [SHGetMalloc]
        pop     eax
        pop     edx
        push    eax
        push    edx
        push    eax
        mov     eax, [eax]
        call    dword [eax+20]
        pop     eax
        push    eax
        mov     eax, [eax]
        call    dword [eax+8]
        jmp     .true
.nodirbrowse:
        cmp     word [esp+16], IDC_ADD_PARTITION
        jnz     .noadd
        cmp     [cur_hd_dev], -1
        jz      .true
        push    eax
        call    set_cur_part
        pop     eax
        mov     eax, [cur_hd_dev]
        mov     ecx, [hdns+eax*4]
        inc     cl
        jnz     @f
        push    10h
        push    0
        push    aTooManyPartitions
        push    ebx
        call    [MessageBoxA]
        jmp     .true
@@:
        mov     [hdns+eax*4], ecx
        push    eax
        shl     ecx, 9
        mov     edx, [hdpart+eax*4]
        test    edx, edx
        jz      .alloc
        push    ecx
        push    [hdpart+eax*4]
        push    0
        push    [hHeap]
        call    [HeapReAlloc]
        jmp     @f
.alloc:
        push    ecx
        push    0
        push    [hHeap]
        call    [HeapAlloc]
@@:
        pop     ecx
        mov     [hdpart+ecx*4], eax
        call    OnSelectHdDev
        mov     eax, [cur_hd_dev]
        mov     ecx, [hdns+eax*4]
        dec     ecx
        mov     eax, [hdpart+eax*4]
        push    0
        push    ecx
        shl     ecx, 9
        mov     byte [eax+ecx], 0
        mov     byte [eax+ecx+511], 0
        push    186h            ; LB_SETCURSEL
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
        call    OnSelectHdPart
        jmp     .true
.noadd:
        cmp     word [esp+16], IDC_DEL_PARTITION
        jnz     .nodel
        mov     eax, [cur_hd_dev]
        cmp     eax, -1
        jz      .true
        mov     ecx, [hdns+eax*4]
        dec     ecx
        js      .true
        mov     [hdns+eax*4], ecx
        push    eax ecx
        push    0
        push    ecx
        push    182h            ; LB_DELETESTRING
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
        pop     ecx eax
        push    eax
        shl     ecx, 9
        push    ecx
        push    [hdpart+eax*4]
        push    0
        push    [hHeap]
        call    [HeapReAlloc]
        pop     ecx
        mov     [hdpart+ecx*4], eax
        mov     eax, [hdns+ecx*4]
        test    eax, eax
        jz      .lastdel
        cmp     [cur_hd_part], eax
        jnz     @f
        or      [cur_hd_part], -1
        push    0
        push    eax
        push    186h
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
@@:
        call    OnSelectHdPart
        jmp     .true
.nodel:
        cmp     word [esp+16], IDC_NOTEMPTY_EXT
        jnz     .noassoc
        push    0
        push    0
        push    0F0h            ; BM_GETCHECK
        push    IDC_NOTEMPTY_EXT
        push    ebx
        call    [SendDlgItemMessageA]
        push    eax
        mov     ecx, IDC_EXTENSIONS
        call    enable_part_items.1
        pop     eax
        jmp     .true
.noassoc:
        cmp     word [esp+16], 1        ; IDOK
        jnz     .true
        call    set_cur_part
; check input parameters
	push	esi
	mov	esi, [pathzeroptr]
	mov	byte [esi-1], 0
	push	full_ini_name
	call	[SetCurrentDirectoryA]
	mov	byte [esi-1], '\'
	pop	esi
        mov     ecx, IDC_RAMDISK
        call    check_file_exists
        mov     ecx, IDC_SKIN
        call    check_file_exists
        mov     ecx, IDC_FONT1
        call    check_file_exists
        mov     ecx, IDC_FONT2
        call    check_file_exists
; ramdisk
        push    512
        push    cur_ini_param
        push    IDC_RAMDISK
        push    ebx
        call    [GetDlgItemTextA]
        mov     ecx, cur_ini_param
        call    add_trailing_slash
        push    full_ini_name
        push    cur_ini_param
        push    ramdisk_keyname
        push    aDisk
        call    [WritePrivateProfileStringA]
; hard disks - delete old values
        push    esi
        xor     esi, esi
        mov     [aHdNn+2], '0'
.readhd_del:
        push    full_ini_name   ; lpFileName
        push    0               ; nDefault
        push    aHdNn           ; lpKeyName
        push    aDisk           ; lpAppName
        call    [GetPrivateProfileIntA]
        push    eax
        push    full_ini_name
        push    0
        push    aHdNn
        push    aDisk
        call    [WritePrivateProfileStringA]
        pop     eax
        inc     [aHdNn+2]
        test    eax, eax
        jz      .3
        push    eax
        push    1
.2:
        push    dword [esp]
        push    esi
        push    aHdNM
        push    cur_ini_name
        call    [wsprintfA]
        add     esp, 10h
        push    full_ini_name
        push    0
        push    cur_ini_name
        push    aDisk
        call    [WritePrivateProfileStringA]
        pop     ecx
        inc     ecx
        cmp     ecx, [esp]
        ja      @f
        push    ecx
        jmp     .2
@@:
        pop     eax
.3:
        inc     esi
        cmp     esi, 4
        jb      .readhd_del
; hard disks - write new values
        xor     esi, esi
        mov     [aHdNn+2], '0'
.writehd:
        mov     eax, [hdns+esi*4]
        test    eax, eax
        jz      .writehd_next
        push    edi
        push    -'0'
@@:
        xor     edx, edx
        mov     ecx, 10
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
@@:
        mov     edi, cur_ini_param
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
        pop     edi
        push    full_ini_name
        push    cur_ini_param
        push    aHdNn
        push    aDisk
        call    [WritePrivateProfileStringA]
        push    1
.4:
        push    dword [esp]
        push    esi
        push    aHdNM
        push    cur_ini_name
        call    [wsprintfA]
        add     esp, 10h
        pop     eax
        push    eax
        dec     eax
        shl     eax, 9
        push    esi edi
        mov     esi, [hdpart+esi*4]
        add     esi, eax
        mov     edi, cur_ini_param
        push    esi
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     esi
        cmp     byte [esi+511], 0
        jz      @f
        mov     byte [edi-1], ','
        mov     dword [edi], 'read'
        mov     dword [edi+4], 'only'
        mov     byte [edi+8], 0
@@:
        pop     edi esi
        push    full_ini_name
        push    cur_ini_param
        push    cur_ini_name
        push    aDisk
        call    [WritePrivateProfileStringA]
        pop     ecx
        inc     ecx
        cmp     ecx, [hdns+esi*4]
        ja      .writehd_next
        push    ecx
        jmp     .4
.writehd_next:
        inc     [aHdNn+2]
        inc     esi
        cmp     esi, 4
        jb      .writehd
; skin file
        push    512
        push    cur_ini_param
        push    IDC_SKIN
        push    ebx
        call    [GetDlgItemTextA]
        push    full_ini_name
        push    cur_ini_param
        push    aSkin
        push    aMain
        call    [WritePrivateProfileStringA]
; system fonts
        push    512
        push    cur_ini_param
        push    IDC_FONT1
        push    ebx
        call    [GetDlgItemTextA]
        push    full_ini_name
        push    cur_ini_param
        push    aFont1
        push    aMain
        call    [WritePrivateProfileStringA]
        push    512
        push    cur_ini_param
        push    IDC_FONT2
        push    ebx
        call    [GetDlgItemTextA]
        push    full_ini_name
        push    cur_ini_param
        push    aFont2
        push    aMain
        call    [WritePrivateProfileStringA]
; associations
        push    0
        push    0
        push    0F0h
        push    IDC_EMPTY_EXT
        push    ebx
        call    [SendDlgItemMessageA]
        cmp     eax, [bWasEmptyAssoc]
        jz      .nomodify_empty_assoc
        test    eax, eax
        jz      .del_empty_assoc
        mov     esi, null
        call    add_assoc
        jmp     .nomodify_empty_assoc
.del_empty_assoc:
        mov     esi, null
        call    del_assoc
.nomodify_empty_assoc:
        push    edi
        mov     esi, cur_ext
        mov     byte [esi], 0
        push    0
        push    0
        push    0F0h
        push    IDC_NOTEMPTY_EXT
        push    ebx
        call    [SendDlgItemMessageA]
        test    eax, eax
        jz      @f
        push    1024
        push    esi
        push    IDC_EXTENSIONS
        push    ebx
        call    [GetDlgItemTextA]
@@:
.scan1loop:
        mov     edi, assoc_ext
        call    find_ext
        jz      @f
        call    add_assoc
@@:
        lodsb
        call    is_ext_delim
        jnz     @b
        test    al, al
        jz      @f
        cmp     byte [esi], 0
        jnz     .scan1loop
@@:
        mov     esi, assoc_ext
        cmp     byte [esi], 0
        jz      .scan2done
.scan2loop:
        mov     edi, cur_ext
        call    find_ext
        jz      @f
        call    del_assoc
@@:
        lodsb
        call    is_ext_delim
        jnz     @b
        test    al, al
        jz      @f
        cmp     byte [esi], 0
        jnz     .scan2loop
@@:
.scan2done:
        pop     edi
.nomodify_notempty_assoc:
        pop     esi
        jmp     .close

.init:
; window icons
        push    0
        push    16
        push    16
        push    1
        push    IDI_ICON1
        push    400000h
        call    [LoadImageA]
        push    eax
        push    0
        push    80h
        push    ebx
        call    [SendMessageA]
        push    0
        push    32
        push    32
        push    1
        push    IDI_ICON1
        push    400000h
        call    [LoadImageA]
        push    eax
        push    1
        push    80h
        push    ebx
        call    [SendMessageA]
; ramdisk
        push    full_ini_name   ; lpFileName
        push    512             ; nSize
        push    cur_ini_param   ; lpReturnedString
        push    null            ; lpDefault
        push    ramdisk_keyname ; lpKeyName
        push    aDisk           ; lpAppName
        call    [GetPrivateProfileStringA]
        push    cur_ini_param
        push    IDC_RAMDISK
        push    ebx
        call    [SetDlgItemTextA]
; hard disks
        push    4
        pop     ecx
@@:
        push    ecx
        push    aHdN
        push    0
        push    180h            ; LB_ADDSTRING
        push    IDC_HD_DEV
        push    ebx
        call    [SendDlgItemMessageA]
        inc     [aHdN+3]
        pop     ecx
        loop    @b
        push    esi
        xor     esi, esi
.readhd:
        push    full_ini_name   ; lpFileName
        push    0               ; nDefault
        push    aHdNn           ; lpKeyName
        push    aDisk           ; lpAppName
        call    [GetPrivateProfileIntA]
        inc     [aHdNn+2]
        mov     [hdns+esi*4], eax
        and     [hdpart+esi*4], 0
        push    edi
        test    eax, eax
        jz      .nopart
        shl     eax, 9
        push    eax
        push    0
        push    [hHeap]
        call    [HeapAlloc]
        mov     edi, eax
        mov     [hdpart+esi*4], eax
        push    1
.1:
        push    dword [esp]
        push    esi
        push    aHdNM
        push    cur_ini_name
        call    [wsprintfA]
        add     esp, 10h
        mov     byte [edi+511], 0
        push    full_ini_name
        push    511
        push    edi
        push    null
        push    cur_ini_name
        push    aDisk
        call    [GetPrivateProfileStringA]
        mov     eax, edi
@@:
        inc     eax
        cmp     byte [eax], 0
        jnz     @b
        sub     eax, 9
        cmp     eax, edi
        jbe     @f
        cmp     byte [eax], ','
        jnz     @f
        cmp     dword [eax+1], 'read'
        jnz     @f
        cmp     dword [eax+5], 'only'
        jnz     @f
        mov     byte [eax], 0
        mov     byte [edi+511], 1
@@:
        add     edi, 512
        pop     ecx
        inc     ecx
        cmp     ecx, [hdns+esi*4]
        ja      .nopart
        push    ecx
        jmp     .1
.nopart:
        pop     edi
        inc     esi
        cmp     esi, 4
        jb      .readhd
        pop     esi
        push    0
        push    0
        push    186h            ; LB_SETCURSEL
        push    IDC_HD_DEV
        push    ebx
        call    [SendDlgItemMessageA]
        call    OnSelectHdDev
; skin
        push    full_ini_name   ; lpFileName
        push    512             ; nSize
        push    cur_ini_param   ; lpReturnedString
        push    null            ; lpDefault
        push    aSkin           ; lpKeyName
        push    aMain           ; lpAppName
        call    [GetPrivateProfileStringA]
        push    cur_ini_param
        push    IDC_SKIN
        push    ebx
        call    [SetDlgItemTextA]
; fonts
        push    full_ini_name   ; lpFileName
        push    512             ; nSize
        push    cur_ini_param   ; lpReturnedString
        push    null            ; lpDefault
        push    aFont1          ; lpKeyName
        push    aMain           ; lpAppName
        call    [GetPrivateProfileStringA]
        push    cur_ini_param
        push    IDC_FONT1
        push    ebx
        call    [SetDlgItemTextA]
        push    full_ini_name   ; lpFileName
        push    512             ; nSize
        push    cur_ini_param   ; lpReturnedString
        push    null            ; lpDefault
        push    aFont2          ; lpKeyName
        push    aMain           ; lpAppName
        call    [GetPrivateProfileStringA]
        push    cur_ini_param
        push    IDC_FONT2
        push    ebx
        call    [SetDlgItemTextA]
        push    emptyExt
        call    check_association
        mov     [bWasEmptyAssoc], eax
        push    0
        push    eax
        push    0F1h    ; BM_SETCHECK
        push    IDC_EMPTY_EXT
        push    ebx
        call    [SendDlgItemMessageA]
        push    esi edi
        xor     esi, esi
        mov     edi, assoc_ext
.enum:
        push    1024
        mov     eax, esp
        push    0
        push    0
        push    0
        push    0
        push    eax
        push    cur_ext
        push    esi
        push    80000000h
        call    [RegEnumKeyExA]
        test    eax, eax
        pop     eax
        jnz     .enum_done
        cmp     byte [cur_ext], '.'
        jnz     .next
        cmp     byte [cur_ext+1], 0
        jz      .next
        push    cur_ext
        call    check_association
        jz      .next
        push    esi
        mov     esi, cur_ext+1
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     byte [edi-1], ';'
        pop     esi
.next:
        inc     esi
        jmp     .enum
.enum_done:
        mov     byte [edi], 0
        pop     edi esi
        xor     eax, eax
        cmp     byte [assoc_ext], 0
        setnz   al
        push    0
        push    eax
        push    0F1h
        push    IDC_NOTEMPTY_EXT
        push    ebx
        call    [SendDlgItemMessageA]
        cmp     byte [assoc_ext], 0
        jz      .no_custom
        push    assoc_ext
        push    IDC_EXTENSIONS
        push    ebx
        call    [SetDlgItemTextA]
        jmp     .assoc_done
.no_custom:
        push    def_custom_assoc
        push    IDC_EXTENSIONS
        push    ebx
        call    [SetDlgItemTextA]
        push    0
        mov     ecx, IDC_EXTENSIONS
        call    enable_part_items.1
        pop     eax
.assoc_done:
        pop     ebx
        xor     eax, eax
        ret     16

set_cur_part:
        cmp     [cur_hd_dev], -1
        jz      .ret
        cmp     [cur_hd_part], -1
        jnz     @f
.ret:
        ret
@@:
        push    eax ecx
        mov     ecx, [cur_hd_part]
        mov     eax, [cur_hd_dev]
        shl     ecx, 9
        add     ecx, [hdpart+eax*4]
        push    ecx
        push    510
        push    ecx
        push    IDC_PARTITION
        push    ebx
        call    [GetDlgItemTextA]
        push    dword [esp]
        call    [GetFileAttributesA]
        cmp     eax, -1
        jz      .err
        test    al, 10h
        jz      .err
        push    0
        push    0
        push    0F0h    ; BM_GETCHECK
        push    IDC_CHECK1
        push    ebx
        call    [SendDlgItemMessageA]
        pop     ecx
        mov     [ecx+511], al
        call    add_trailing_slash
        pop     ecx eax
        ret
.err:
        push    10h
        push    0
        push    aInvalidPath
        push    ebx
        call    [MessageBoxA]
        pop     ecx
        pop     ecx eax
        pop     eax eax
        push    0
        push    [cur_hd_dev]
        push    186h
        push    IDC_HD_DEV
        push    ebx
        call    [SendDlgItemMessageA]
        push    0
        push    [cur_hd_part]
        push    186h
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
        call    focus_on_path
        jmp     DlgProc.true

focus_on_path:
        push    IDC_PARTITION
        push    ebx
        call    [GetDlgItem]
        push    eax
        call    [SetFocus]
        ret

add_trailing_slash:
        inc     ecx
        cmp     byte [ecx], 0
        jnz     add_trailing_slash
        cmp     byte [ecx-1], '\'
        jz      @f
        mov     word [ecx], '\'
@@:
        ret

enable_part_items:
        push    eax
        mov     ecx, IDC_PATH_STRING
        call    .1
        mov     ecx, IDC_PARTITION
        call    .1
        mov     ecx, IDC_PARTITION_BROWSE
        call    .1
        mov     ecx, IDC_CHECK1
        call    .1
        pop     eax
        ret

.1:
        push    ecx
        push    ebx
        call    [GetDlgItem]
        push    dword [esp+4]
        push    eax
        call    [EnableWindow]
        ret

OnSelectHdDev:
        push    0
        push    0
        push    188h            ; LB_GETCURSEL
        push    IDC_HD_DEV
        push    ebx
        call    [SendDlgItemMessageA]
        cmp     eax, 4
        jae     .ret
        call    set_cur_part
        mov     [cur_hd_dev], eax
        or      [cur_hd_part], -1
        push    eax
        xor     eax, eax
        call    enable_part_items
        push    null
        push    IDC_PARTITION
        push    ebx
        call    [SetDlgItemTextA]
        push    0
        push    0
        push    0F1h            ; BM_SETCHECK
        push    IDC_CHECK1
        push    ebx
        call    [SendDlgItemMessageA]
        push    0
        push    0
        push    184h            ; LB_RESETCONTENT
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
        pop     eax
        cmp     dword [hdns+eax*4], 0
        jnz     .haspart
        push    aNone
        push    0
        push    180h            ; LB_ADDSTRING
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
        push    0
.done:
        mov     ecx, IDC_HD_PART
        call    enable_part_items.1
        mov     ecx, IDC_DEL_PARTITION
        call    enable_part_items.1
        pop     eax
.ret:
        ret
.haspart:
        push    esi
        mov     esi, eax
        push    1
@@:
        push    dword [esp]
        push    esi
        push    aHdNM2
        push    cur_ini_name
        call    [wsprintfA]
        add     esp, 10h
        push    cur_ini_name
        push    0
        push    180h
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
        pop     ecx
        inc     ecx
        cmp     ecx, [hdns+esi*4]
        ja      @f
        push    ecx
        jmp     @b
@@:
        pop     esi
        push    1
        jmp     .done

OnSelectHdPart:
        push    0
        push    0
        push    188h
        push    IDC_HD_PART
        push    ebx
        call    [SendDlgItemMessageA]
        mov     ecx, [cur_hd_dev]
        cmp     eax, [hdns+ecx*4]
        jae     .ret
        call    set_cur_part
        mov     [cur_hd_part], eax
        mov     ecx, [hdpart+ecx*4]
        shl     eax, 9
        add     ecx, eax
        push    ecx
        movzx   eax, byte [ecx+511]
        push    0
        push    eax
        push    0F1h    ; BM_SETCHECK
        push    IDC_CHECK1
        push    ebx
        call    [SendDlgItemMessageA]
        push    IDC_PARTITION
        push    ebx
        call    [SetDlgItemTextA]
        xor     eax, eax
        inc     eax
        call    enable_part_items
        call    focus_on_path
.ret:
        ret

check_association:
        push    hkey
        push    1       ; KEY_QUERY_VALUE
        push    0
        push    dword [esp+16]
        push    80000000h       ; HKEY_CLASSES_ROOT
        call    [RegOpenKeyExA]
        test    eax, eax
        jnz     .not
        push    1000
        push    esp
        push    cur_ini_param
        push    0
        push    0
        push    0
        push    [hkey]
        call    [RegQueryValueExA]
        mov     [esp], eax
        push    [hkey]
        call    [RegCloseKey]
        pop     eax
        test    eax, eax
        jnz     .not
        push    esi edi
        mov     edi, cur_ini_param
@@:
        inc     edi
        cmp     byte [edi-1], 0
        jnz     @b
        dec     edi
        mov     esi, aShellOpenCommand
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     edi esi
        push    hkey
        push    1
        push    0
        push    cur_ini_param
        push    80000000h
        call    [RegOpenKeyExA]
        test    eax, eax
        jnz     .not
        push    1024
        push    esp
        push    cur_ini_param
        push    0
        push    0
        push    0
        push    [hkey]
        call    [RegQueryValueExA]
        mov     [esp], eax
        push    [hkey]
        call    [RegCloseKey]
        pop     eax
        test    eax, eax
        jnz     .not
        push    esi edi
        mov     edi, cur_ini_param
.scan:
        cmp     byte [edi], 0
        jz      .scanno
        mov     esi, ExeFileName
        mov     ecx, ExeFileNameLen
        push    edi
        repz    cmpsb
        pop     edi
        jz      .scanok
        inc     edi
        jmp     .scan
.scanno:
        pop     edi esi
.not:
        xor     eax, eax
        ret     4
.scanok:
        pop     edi esi
        xor     eax, eax
        inc     eax
        ret     4

add_assoc:
        call    form_ext_name
        push    ebx
        xor     ebx, ebx
        push    ebx
        push    hkey
        push    ebx
        push    2       ; KEY_SET_VALUE
        push    ebx
        push    ebx
        push    ebx
        push    edi
        push    80000000h
        call    [RegCreateKeyExA]
        test    eax, eax
        jnz     .ret
        push    aKolibriExecutable.size
        push    aKolibriExecutable
        push    1       ; REG_SZ
        push    ebx
        push    ebx
        push    [hkey]
        call    [RegSetValueExA]
        push    eax
        push    [hkey]
        call    [RegCloseKey]
        pop     eax
        test    eax, eax
        jnz     .ret
        push    ebx
        push    hkey
        push    ebx
        push    6       ; KEY_SET_VALUE + KEY_CREATE_SUB_KEY
        push    ebx
        push    ebx
        push    ebx
        push    aKolibriExecutable
        push    80000000h
        call    [RegCreateKeyExA]
        test    eax, eax
        jnz     .ret
        push    edi
        mov     edi, cur_ini_param
        mov     al, '"'
        stosb
        call    get_exe_name
        mov     al, '"'
        stosb
        mov     eax, ' "%1'
        stosd
        mov     al, '"'
        stosb
        mov     al, 0
        stosb
        push    [hkey]
        push    ebx
        push    hkey
        push    ebx
        push    2       ; KEY_SET_VALUE
        push    ebx
        push    ebx
        push    ebx
        push    aShellOpenCommand2
        push    [hkey]
        call    [RegCreateKeyExA]
        test    eax, eax
        jnz     .ret2
        mov     eax, cur_ini_param
        sub     edi, eax
        push    edi
        push    eax
        push    1
        push    ebx
        push    ebx
        push    [hkey]
        call    [RegSetValueExA]
        push    [hkey]
        call    [RegCloseKey]
        mov     edi, cur_ini_param
        call    get_exe_name
        mov     al, 0
        stosb
        pop     eax
        push    eax
        push    ebx
        push    hkey
        push    ebx
        push    2
        push    ebx
        push    ebx
        push    ebx
        push    aDefaultIcon
        push    eax
        call    [RegCreateKeyExA]
        test    eax, eax
        jnz     .ret2
        mov     eax, cur_ini_param
        sub     edi, eax
        push    edi
        push    eax
        push    1
        push    ebx
        push    ebx
        push    [hkey]
        call    [RegSetValueExA]
        push    [hkey]
        call    [RegCloseKey]
.ret2:
        call    [RegCloseKey]
        pop     edi
.ret:
        pop     ebx
        ret

del_assoc:
        call    form_ext_name
        push    edi
        push    80000000h
        call    [RegDeleteKeyA]
        ret

form_ext_name:
        push    esi
        mov     edi, cur_ini_param
        push    edi
        mov     al, '.'
        stosb
@@:
        lodsb
        stosb
        call    is_ext_delim
        jnz     @b
        mov     byte [edi-1], 0
        pop     edi esi
        ret

get_exe_name:
        push    esi
        mov     esi, full_ini_name
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
@@:
        dec     edi
        cmp     byte [edi-1], '\'
        jnz     @b
        mov     esi, ExeFileName
        mov     ecx, ExeFileNameLen
        rep     movsb
        pop     esi
        ret

check_file_exists:
        push    ecx
        push    512
        push    cur_ini_param
        push    ecx
        push    ebx
        call    [GetDlgItemTextA]
        push    cur_ini_param
        call    [GetFileAttributesA]
        inc     eax
        jz      .err
        pop     ecx
        ret
.err:
        push    10h
        push    0
        push    aInvalidFile
        push    ebx
        call    [MessageBoxA]
        push    ebx
        call    [GetDlgItem]
        push    eax
        call    [SetFocus]
        pop     eax
        jmp     DlgProc.true

find_ext:
        push    esi
@@:
        lodsb
        scasb
        jnz     @f
        call    is_ext_delim
        jnz     @b
.ret:
        pop     esi
        ret
@@:
        call    is_ext_delim
        jnz     .next
        mov     al, byte [edi-1]
        call    is_ext_delim
        jz      .ret
.next:
        pop     esi
        dec     edi
@@:
        inc     edi
        mov     al, byte [edi-1]
        call    is_ext_delim
        jnz     @b
        test    al, al
        jz      @f
        cmp     byte [edi], 0
        jnz     find_ext
@@:
        inc     edi
        ret

is_ext_delim:
        test    al, al
        jz      @f
        cmp     al, ','
        jz      @f
        cmp     al, ';'
@@:     ret

align 4
data import
macro thunk a {
a#_thunk:dw 0
db `a,0}
        dd      0,0,0, rva kernel32_name, rva kernel32_thunks
        dd      0,0,0, rva user32_name, rva user32_thunks
        dd      0,0,0, rva advapi32_name, rva advapi32_thunks
        dd      0,0,0, rva shell32_name, rva shell32_thunks
        dd      0,0,0, rva ole32_name, rva ole32_thunks
        dd      0,0,0, rva comdlg32_name, rva comdlg32_thunks
        dd      0,0,0,0,0
kernel32_name   db      'kernel32.dll',0
user32_name     db      'user32.dll',0
advapi32_name   db      'advapi32.dll',0
comdlg32_name   db      'comdlg32.dll',0
shell32_name    db      'shell32.dll',0
ole32_name      db      'ole32.dll',0
kernel32_thunks:
GetFileAttributesA      dd      rva GetFileAttributesA_thunk
CreateFileA             dd      rva CreateFileA_thunk
WriteFile               dd      rva WriteFile_thunk
CloseHandle             dd      rva CloseHandle_thunk
SetCurrentDirectoryA	dd	rva SetCurrentDirectoryA_thunk
GetPrivateProfileIntA   dd      rva GetPrivateProfileIntA_thunk
GetPrivateProfileStringA dd     rva GetPrivateProfileStringA_thunk
WritePrivateProfileStringA dd   rva WritePrivateProfileStringA_thunk
GetModuleFileNameA      dd      rva GetModuleFileNameA_thunk
GetProcessHeap          dd      rva GetProcessHeap_thunk
HeapAlloc               dd      rva HeapAlloc_thunk
HeapReAlloc             dd      rva HeapReAlloc_thunk
ExitProcess             dd      rva ExitProcess_thunk
                        dw      0
thunk GetFileAttributesA
thunk CreateFileA
thunk WriteFile
thunk CloseHandle
thunk SetCurrentDirectoryA
thunk GetPrivateProfileIntA
thunk GetPrivateProfileStringA
thunk WritePrivateProfileStringA
thunk GetModuleFileNameA
thunk GetProcessHeap
thunk HeapAlloc
thunk HeapReAlloc
thunk ExitProcess
user32_thunks:
DialogBoxParamA         dd      rva DialogBoxParamA_thunk
EndDialog               dd      rva EndDialog_thunk
GetDlgItem              dd      rva GetDlgItem_thunk
GetDlgItemTextA         dd      rva GetDlgItemTextA_thunk
SetDlgItemTextA         dd      rva SetDlgItemTextA_thunk
EnableWindow            dd      rva EnableWindow_thunk
SendDlgItemMessageA     dd      rva SendDlgItemMessageA_thunk
wsprintfA               dd      rva wsprintfA_thunk
MessageBoxA             dd      rva MessageBoxA_thunk
SetFocus                dd      rva SetFocus_thunk
LoadImageA              dd      rva LoadImageA_thunk
SendMessageA            dd      rva SendMessageA_thunk
                        dw      0
thunk DialogBoxParamA
thunk EndDialog
thunk GetDlgItem
thunk GetDlgItemTextA
thunk SetDlgItemTextA
thunk EnableWindow
thunk SendDlgItemMessageA
thunk wsprintfA
thunk MessageBoxA
thunk SetFocus
thunk LoadImageA
thunk SendMessageA
advapi32_thunks:
RegOpenKeyExA           dd      rva RegOpenKeyExA_thunk
RegQueryValueExA        dd      rva RegQueryValueExA_thunk
RegCloseKey             dd      rva RegCloseKey_thunk
RegEnumKeyExA           dd      rva RegEnumKeyExA_thunk
RegDeleteKeyA           dd      rva RegDeleteKeyA_thunk
RegCreateKeyExA         dd      rva RegCreateKeyExA_thunk
RegSetValueExA          dd      rva RegSetValueExA_thunk
                        dw      0
thunk RegOpenKeyExA
thunk RegQueryValueExA
thunk RegCloseKey
thunk RegEnumKeyExA
thunk RegDeleteKeyA
thunk RegCreateKeyExA
thunk RegSetValueExA
comdlg32_thunks:
GetOpenFileNameA        dd      rva GetOpenFileNameA_thunk
                        dw      0
thunk GetOpenFileNameA
ole32_thunks:
CoInitializeEx          dd      rva CoInitializeEx_thunk
CoUninitialize          dd      rva CoUninitialize_thunk
                        dw      0
thunk CoInitializeEx
thunk CoUninitialize
shell32_thunks:
SHGetMalloc             dd      rva SHGetMalloc_thunk
SHBrowseForFolder       dd      rva SHBrowseForFolder_thunk
SHGetPathFromIDList     dd      rva SHGetPathFromIDList_thunk
                        dw      0
thunk SHGetMalloc
thunk SHBrowseForFolder
thunk SHGetPathFromIDList
end data

align 10h
data resource from 'config.res'
end data

section '.data' data readable writable

IniFileName     db      'KlbrInWin.ini',0
ExeFileName     db      'KlbrInWin.exe'
ExeFileNameLen = $ - ExeFileName
null            db      0
aDisk           db      'Disk',0
aMain           db      'Main',0
ramdisk_keyname db      'RamDisk',0
aSkin           db      'Skin',0
aFont1          db      'Font1',0
aFont2          db      'Font2',0
aHdN            db      '/hd0',0
aHdNn           db      'hd0n',0
aHdNM           db      'hd%d_%d',0
aHdNM2          db      '/hd%d/%d',0
aNone           db      '(none)',0
emptyExt        db      '.',0
def_custom_assoc db     'kex;',0
aShellOpenCommand db    '\'
aShellOpenCommand2 db   'shell\open\command',0
aKolibriExecutable db   'KolibriExecutable',0
.size = $ - aKolibriExecutable
aDefaultIcon    db      'DefaultIcon',0
ini:
        file    'config.ini'
ini_size = $ - ini

align 4
ofn:
        dd      76      ; lStructSize
        dd      0       ; hWndOwner
        dd      0       ; hInstance
filter  dd      0       ; lpstrFilter
        dd      0       ; lpstrCustomFilter
        dd      0       ; nMaxCustFilter
        dd      0       ; nFilterIndex
        dd      cur_ini_param   ; lpstrFile
        dd      512     ; nMaxFile
        dd      0       ; lpstrFileTitle
        dd      0       ; nMaxFileTitle
        dd      0       ; lpstrInitialDir
;        dd      ofn_title       ; lpstrTitle
        dd      0
        dd      81804h  ; flags
        dw      0       ; nFileOffset
        dw      0       ; nFileExtension
        dd      0       ; lpstrDefExt
        dd      0       ; lCustData
        dd      0       ; lpfnHook
        dd      0       ; lpTemplateName

bi:
        dd      0       ; hwndOwner
        dd      0       ; pidlRoot
        dd      cur_ini_param   ; pszDisplayName
        dd      bi_title; lpszTitle
        dd      1       ; BIF_RETURNONLYFSDIRS
        dd      0       ; lpfn
        dd      0       ; lParam
        dd      0       ; iImage

cur_hd_dev      dd      -1
cur_hd_part     dd      -1

bWasEmptyAssoc  dd      0

skn_filter      db      'Skin files (*.skn)',0,'*.skn',0,'All files (*.*',0,'*.*',0,0
mt_filter       db      'Files *.mt',0,'*.mt',0,'All files (*.*)',0,'*.*',0,0

bi_title        db      'Select folder for root of emulated file system:',0

aInvalidPath    db      'Entered path is invalid or does not specify a directory',0
aInvalidFile    db      'The mentioned file does not exist',0
aTooManyPartitions db   'Too many partitions! (maximum 255 per device is allowed)',0

align 4
hHeap           dd      ?
hkey            dd      ?
pathzeroptr	dd	?
full_ini_name   rb      1024
cur_ini_param   rb      1024
cur_ini_name    rb      32
hdns            rd      4
hdpart          rd      4
cur_ext         rb      1024
assoc_ext       rb      1024
