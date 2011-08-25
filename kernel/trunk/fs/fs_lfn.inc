;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2009. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


image_of_eax EQU esp+32
image_of_ebx EQU esp+20

; System function 70 - files with long names (LFN)
; diamond, 2006

iglobal
; in this table names must be in lowercase
rootdirs:
	db	2,'rd'
	dd	fs_OnRamdisk
	dd	fs_NextRamdisk
	db	7,'ramdisk'
	dd	fs_OnRamdisk
	dd	fs_NextRamdisk
	db	2,'fd'
	dd	fs_OnFloppy
	dd	fs_NextFloppy
	db	10,'floppydisk'
	dd	fs_OnFloppy
	dd	fs_NextFloppy
	db	3,'hd0'
	dd	fs_OnHd0
	dd	fs_NextHd0
	db	3,'hd1'
	dd	fs_OnHd1
	dd	fs_NextHd1
	db	3,'hd2'
	dd	fs_OnHd2
	dd	fs_NextHd2
	db	3,'hd3'
	dd	fs_OnHd3
	dd	fs_NextHd3
;**********************************************
	db	3,'cd0'
	dd	fs_OnCd0
	dd	fs_NextCd
	db	3,'cd1'
	dd	fs_OnCd1
	dd	fs_NextCd
	db	3,'cd2'
	dd	fs_OnCd2
	dd	fs_NextCd
	db	3,'cd3'
	dd	fs_OnCd3
	dd	fs_NextCd
;***********************************************
	db	0


virtual_root_query:
	dd	fs_HasRamdisk
	db	'rd',0
	dd	fs_HasFloppy
	db	'fd',0
	dd	fs_HasHd0
	db	'hd0',0
	dd	fs_HasHd1
	db	'hd1',0
	dd	fs_HasHd2
	db	'hd2',0
	dd	fs_HasHd3
	db	'hd3',0
;**********************************************
	dd	fs_HasCd0
	db	'cd0',0
	dd	fs_HasCd1
	db	'cd1',0
	dd	fs_HasCd2
	db	'cd2',0
	dd	fs_HasCd3
	db	'cd3',0
;**********************************************
	dd	0

fs_additional_handlers:
        dd      biosdisk_handler, biosdisk_enum_root
        dd      dyndisk_handler, dyndisk_enum_root
; add new handlers here
        dd      0

endg
file_system_lfn:
; in: ebx->fileinfo block
; operation codes:
; 0 : read file
; 1 : read folder
; 2 : create/rewrite file
; 3 : write/append to file
; 4 : set end of file
; 5 : get file/directory attributes structure
; 6 : set file/directory attributes structure
; 7 : start application
; 8 : delete file
; 9 : create directory

; parse file name
	lea	esi, [ebx+20]
	lodsb
	test	al, al
	jnz	@f
	mov	esi, [esi]
	lodsb
@@:
	cmp	al, '/'
	jz	.notcurdir
	dec	esi
	mov	ebp, esi
	test	al, al
	jnz	@f
	xor	ebp, ebp
@@:
	mov	esi, [current_slot]
	mov	esi, [esi+APPDATA.cur_dir]
	jmp	.parse_normal
.notcurdir:
	cmp	byte [esi], 0
	jz	.rootdir
	call	process_replace_file_name
.parse_normal:
	cmp dword [ebx], 7
	jne @F
	mov edx, [ebx+4]
	mov ebx, [ebx+8]
	call fs_execute  ; esi+ebp, ebx, edx
	mov [image_of_eax], eax
	ret
@@:
	mov	edi, rootdirs-8
	xor	ecx, ecx
	push	esi
.scan1:
	pop	esi
	add	edi, ecx
	scasd
	scasd
	mov	cl, byte [edi]
	test	cl, cl
	jz	.notfound_try
	inc	edi
	push	esi
@@:
	lodsb
	or	al, 20h
	scasb
	loopz	@b
	jnz	.scan1
	lodsb
	cmp	al, '/'
	jz	.found1
	test	al, al
	jnz	.scan1
	pop	eax
; directory /xxx
.maindir:
	mov	esi, [edi+4]
.maindir_noesi:
	cmp	dword [ebx], 1
	jnz	.access_denied
	xor	eax, eax
	mov	ebp, [ebx+12]			;количество блоков для считывания
	mov	edx, [ebx+16]			;куда записывать рузельтат
    ;    add     edx, std_application_base_address
	push	dword [ebx+4]	; first block
	mov	ebx, [ebx+8]	; flags
; ebx=flags, [esp]=first block, ebp=number of blocks, edx=return area, esi='Next' handler
	mov	edi, edx
	push    ecx
	mov	ecx, 32/4
	rep	stosd
	pop     ecx
	mov	byte [edx], 1	; version
.maindir_loop:
	call	esi
	jc	.maindir_done
	inc	dword [edx+8]
	dec	dword [esp]
	jns	.maindir_loop
	dec	ebp
	js	.maindir_loop
	inc	dword [edx+4]
	mov	dword [edi], 0x10	; attributes: folder
	mov	dword [edi+4], 1	; name type: UNICODE
	push	eax
	xor	eax, eax
	add	edi, 8
	push    ecx
	mov	ecx, 40/4-2
	rep	stosd
	pop     ecx
	pop	eax
	push	eax edx
; convert number in eax to decimal UNICODE string
	push	edi
	push    ecx
	push	-'0'
	mov	ecx, 10
@@:
	xor	edx, edx
	div	ecx
	push	edx
	test	eax, eax
	jnz	@b
@@:
	pop	eax
	add	al, '0'
	stosb
	test	bl, 1		; UNICODE name?
	jz	.ansi2
	mov	byte [edi], 0
	inc	edi
.ansi2:
	test	al, al
	jnz	@b
	mov	byte [edi-1], 0
	pop     ecx
	pop	edi
; UNICODE name length is 520 bytes, ANSI - 264
	add	edi, 520
	test	bl, 1
	jnz	@f
	sub	edi, 520-264
@@:
	pop	edx eax
	jmp	.maindir_loop
.maindir_done:
	pop	eax
	mov	ebx, [edx+4]
	xor	eax, eax
	dec	ebp
	js	@f
	mov	al, ERROR_END_OF_FILE
@@:
	mov	[image_of_eax], eax
	mov	[image_of_ebx], ebx
	ret
; directory /
.rootdir:
	cmp	dword [ebx], 1	; read folder?
	jz	.readroot
.access_denied:
	mov	dword [image_of_eax], 10      ; access denied
	ret

.readroot:
; virtual root folder - special handler
	mov	esi, virtual_root_query
	mov	ebp, [ebx+12]
	mov	edx, [ebx+16]
    ;    add     edx, std_application_base_address
	push	dword [ebx+4]	; first block
	mov	ebx, [ebx+8]	; flags
	xor	eax, eax
; eax=0, [esp]=first block, ebx=flags, ebp=number of blocks, edx=return area
	mov	edi, edx
	mov	ecx, 32/4
	rep	stosd
	mov	byte [edx], 1	; version
.readroot_loop:
	cmp	dword [esi], eax
	jz	.readroot_done_static
	call	dword [esi]
	add	esi, 4
	test	eax, eax
	jnz	@f
.readroot_next:
	or	ecx, -1
	xchg	esi, edi
	repnz	scasb
	xchg	esi, edi
	jmp	.readroot_loop
@@:
	xor	eax, eax
	inc	dword [edx+8]
	dec	dword [esp]
	jns	.readroot_next
	dec	ebp
	js	.readroot_next
	inc	dword [edx+4]
	mov	dword [edi], 0x10	; attributes: folder
	mov	dword [edi+4], ebx	; name type: UNICODE
	add	edi, 8
	mov	ecx, 40/4-2
	rep	stosd
	push	edi
@@:
	lodsb
	stosb
	test	bl, 1
	jz	.ansi
	mov	byte [edi], 0
	inc	edi
.ansi:
	test	eax, eax
	jnz	@b
	pop	edi
	add	edi, 520
	test	bl, 1
	jnz	.readroot_loop
	sub	edi, 520-264
	jmp	.readroot_loop
.readroot_done_static:
        mov     esi, fs_additional_handlers-8
        sub     esp, 16
.readroot_ah_loop:
        add     esi, 8
        cmp     dword [esi], 0
        jz      .readroot_done
        xor     eax, eax
.readroot_ah_loop2:
        push    edi
        lea     edi, [esp+4]
        call    dword [esi+4]
        pop     edi
        test    eax, eax
        jz      .readroot_ah_loop
        inc     dword [edx+8]
        dec     dword [esp+16]
        jns     .readroot_ah_loop2
        dec     ebp
        js      .readroot_ah_loop2
        push    eax
        xor     eax, eax
        inc     dword [edx+4]
        mov     dword [edi], 0x10       ; attributes: folder
        mov     dword [edi+4], ebx
        add     edi, 8
        mov     ecx, 40/4-2
        rep     stosd
        push    esi edi
        lea     esi, [esp+12]
@@:
        lodsb
        stosb
        test    bl, 1
        jz      .ansi3
        mov     byte [edi], 0
        inc     edi
.ansi3:
        test    al, al
        jnz     @b
        pop     edi esi eax
        add     edi, 520
        test    bl, 1
        jnz     .readroot_ah_loop2
        sub     edi, 520-264
        jmp     .readroot_ah_loop2
.readroot_done:
        add     esp, 16
	pop	eax
	mov	ebx, [edx+4]
	xor	eax, eax
	dec	ebp
	js	@f
	mov	al, ERROR_END_OF_FILE
@@:
	mov	[image_of_eax], eax
	mov	[image_of_ebx], ebx
	ret
.notfound_try:
        mov     edi, fs_additional_handlers
@@:
        cmp     dword [edi], 0
        jz      .notfound
        call    dword [edi]
        scasd
        scasd
        jmp     @b
.notfound:
	mov	dword [image_of_eax], ERROR_FILE_NOT_FOUND
	and	dword [image_of_ebx], 0
	ret

.notfounda:
        cmp     edi, esp
        jnz     .notfound
        call    dword [edi+4]
        add     esp, 16
        jmp     .notfound

.found1:
	pop	eax
	cmp	byte [esi], 0
	jz	.maindir
.found2:
; read partition number
	xor	ecx, ecx
	xor	eax, eax
@@:
	lodsb
	cmp	al, '/'
	jz	.done1
	test	al, al
	jz	.done1
	sub	al, '0'
	cmp	al, 9
	ja	.notfounda
	lea	ecx, [ecx*5]
	lea	ecx, [ecx*2+eax]
	jmp	@b
.done1:
	jecxz	.notfounda
	test	al, al
	jnz	@f
	dec	esi
@@:
	cmp	byte [esi], 0
	jnz	@f
	test	ebp, ebp
	jz	@f
	mov	esi, ebp
	xor	ebp, ebp
@@:
; now [edi] contains handler address, ecx - partition number,
; esi points to ASCIIZ string - rest of name
	jmp	dword [edi]

; handlers for devices
; in: ecx = 0 => query virtual directory /xxx
; in: ecx = partition number
;     esi -> relative (for device) name
;     ebx -> fileinfo
;     ebp = 0 or pointer to rest of name from folder addressed by esi
; out: [image_of_eax]=image of eax, [image_of_ebx]=image of ebx

fs_OnRamdisk:
	cmp	ecx, 1
	jnz	file_system_lfn.notfound
	mov	eax, [ebx]
	cmp	eax, fs_NumRamdiskServices
	jae	.not_impl
	mov	ecx, [ebx+12]
	mov	edx, [ebx+16]
   ;     add     edx, std_application_base_address
	add	ebx, 4
	call	dword [fs_RamdiskServices + eax*4]
	mov	[image_of_eax], eax
	mov	[image_of_ebx], ebx
	ret
.not_impl:
	mov	dword [image_of_eax], 2       ; not implemented
	ret

fs_NotImplemented:
	mov	eax, 2
	ret

fs_RamdiskServices:
	dd	fs_RamdiskRead
	dd	fs_RamdiskReadFolder
	dd	fs_RamdiskRewrite
	dd	fs_RamdiskWrite
	dd	fs_RamdiskSetFileEnd
	dd	fs_RamdiskGetFileInfo
	dd	fs_RamdiskSetFileInfo
	dd	0
	dd	fs_RamdiskDelete
	dd	fs_RamdiskCreateFolder
fs_NumRamdiskServices = ($ - fs_RamdiskServices)/4

fs_OnFloppy:
	cmp	ecx, 2
	ja	file_system_lfn.notfound
	mov	eax, [ebx]
	cmp	eax, fs_NumFloppyServices
	jae	fs_OnRamdisk.not_impl
	call	reserve_flp
	mov	[flp_number], cl
	mov	ecx, [ebx+12]
	mov	edx, [ebx+16]
   ;     add     edx, std_application_base_address
	add	ebx, 4
	call	dword [fs_FloppyServices + eax*4]
	and	[flp_status], 0
	mov	[image_of_eax], eax
	mov	[image_of_ebx], ebx
	ret

fs_FloppyServices:
	dd	fs_FloppyRead
	dd	fs_FloppyReadFolder
	dd	fs_FloppyRewrite
	dd	fs_FloppyWrite
	dd	fs_FloppySetFileEnd
	dd	fs_FloppyGetFileInfo
	dd	fs_FloppySetFileInfo
	dd	0
	dd	fs_FloppyDelete
	dd	fs_FloppyCreateFolder
fs_NumFloppyServices = ($ - fs_FloppyServices)/4

fs_OnHd0:
	call	reserve_hd1
	mov	[hdbase], 0x1F0
	mov	[hdid], 0
	push	1
	jmp	fs_OnHd
fs_OnHd1:
	call	reserve_hd1
	mov	[hdbase], 0x1F0
	mov	[hdid], 0x10
	push	2
	jmp	fs_OnHd
fs_OnHd2:
	call	reserve_hd1
	mov	[hdbase], 0x170
	mov	[hdid], 0
	push	3
	jmp	fs_OnHd
fs_OnHd3:
	call	reserve_hd1
	mov	[hdbase], 0x170
	mov	[hdid], 0x10
	push	4
fs_OnHd:
	call	reserve_hd_channel
	pop	eax
	mov	[hdpos], eax
	cmp	ecx, 0x100
	jae	fs_OnHdAndBd.nf
	cmp	cl, [DRIVE_DATA+1+eax]
fs_OnHdAndBd:
	jbe	@f
.nf:
	call	free_hd_channel
	and	[hd1_status], 0
	mov	dword [image_of_eax], 5       ; not found
	ret
@@:
	mov	[known_part], ecx ;	mov	[fat32part], ecx
	push	ebx esi
	call	choice_necessity_partition_1
	pop	esi ebx
	mov	ecx, [ebx+12]
	mov	edx, [ebx+16]
    ;    add     edx, std_application_base_address
	mov	eax, [ebx]
	cmp	eax, fs_NumHdServices
	jae	.not_impl
	add	ebx, 4
	call	dword [fs_HdServices + eax*4]
	call	free_hd_channel
	and	[hd1_status], 0
	mov	[image_of_eax], eax
	mov	[image_of_ebx], ebx
	ret
.not_impl:
	call	free_hd_channel
	and	[hd1_status], 0
	mov	dword [image_of_eax], 2       ; not implemented
	ret

fs_HdServices:
	dd	fs_HdRead
	dd	fs_HdReadFolder
	dd	fs_HdRewrite
	dd	fs_HdWrite
	dd	fs_HdSetFileEnd
	dd	fs_HdGetFileInfo
	dd	fs_HdSetFileInfo
	dd	0
	dd	fs_HdDelete
	dd	fs_HdCreateFolder
fs_NumHdServices = ($ - fs_HdServices)/4

;*******************************************************
fs_OnCd0:
	call	reserve_cd
	mov  [ChannelNumber],1
	mov  [DiskNumber],0
	push	6
	push	1
	jmp	fs_OnCd
fs_OnCd1:
	call	reserve_cd
	mov  [ChannelNumber],1
	mov  [DiskNumber],1
	push	4
	push	2
	jmp	fs_OnCd
fs_OnCd2:
	call	reserve_cd
	mov  [ChannelNumber],2
	mov  [DiskNumber],0
	push	2
	push	3
	jmp	fs_OnCd
fs_OnCd3:
	call	reserve_cd
	mov  [ChannelNumber],2
	mov  [DiskNumber],1
	push	0
	push	4
fs_OnCd:
	call	reserve_cd_channel
	pop	eax
	mov	[cdpos], eax
	pop	eax
	cmp	ecx, 0x100
	jae	.nf
	push	ecx ebx
	mov	cl,al
	mov	bl,[DRIVE_DATA+1]
	shr	bl,cl
	test	bl,2
	pop	ebx ecx

	jnz	@f
.nf:
	call	free_cd_channel
	and    [cd_status], 0
	mov	dword [image_of_eax], 5       ; not found
	ret
@@:
	mov	ecx, [ebx+12]
	mov	edx, [ebx+16]
    ;    add     edx, std_application_base_address
	mov	eax, [ebx]
	cmp	eax,fs_NumCdServices
	jae	 .not_impl
	add	ebx, 4
	call	dword [fs_CdServices + eax*4]
	call	free_cd_channel
	and	[cd_status], 0
	mov	[image_of_eax], eax
	mov	[image_of_ebx], ebx
	ret
.not_impl:
	call	free_cd_channel
	and	[cd_status], 0
	mov	dword [image_of_eax], 2       ; not implemented
	ret

fs_CdServices:
	dd	fs_CdRead
	dd	fs_CdReadFolder
	dd	fs_NotImplemented
	dd	fs_NotImplemented
	dd	fs_NotImplemented
	dd	fs_CdGetFileInfo
	dd	fs_NotImplemented
	dd	0
	dd	fs_NotImplemented
	dd	fs_NotImplemented
fs_NumCdServices = ($ - fs_CdServices)/4

;*******************************************************

fs_HasRamdisk:
	mov	al, 1	; we always have ramdisk
	ret

fs_HasFloppy:
	cmp	byte [DRIVE_DATA], 0
	setnz	al
	ret

fs_HasHd0:
	mov	al, [DRIVE_DATA+1]
	and	al, 11000000b
	cmp	al, 01000000b
	setz	al
	ret
fs_HasHd1:
	mov	al, [DRIVE_DATA+1]
	and	al, 00110000b
	cmp	al, 00010000b
	setz	al
	ret
fs_HasHd2:
	mov	al, [DRIVE_DATA+1]
	and	al, 00001100b
	cmp	al, 00000100b
	setz	al
	ret
fs_HasHd3:
	mov	al, [DRIVE_DATA+1]
	and	al, 00000011b
	cmp	al, 00000001b
	setz	al
	ret

;*******************************************************
fs_HasCd0:
	mov	al, [DRIVE_DATA+1]
	and	al, 11000000b
	cmp	al, 10000000b
	setz	al
	ret
fs_HasCd1:
	mov	al, [DRIVE_DATA+1]
	and	al, 00110000b
	cmp	al, 00100000b
	setz	al
	ret
fs_HasCd2:
	mov	al, [DRIVE_DATA+1]
	and	al, 00001100b
	cmp	al, 00001000b
	setz	al
	ret
fs_HasCd3:
	mov	al, [DRIVE_DATA+1]
	and	al, 00000011b
	cmp	al, 00000010b
	setz	al
	ret
;*******************************************************

; fs_NextXXX functions:
; in: eax = partition number, from which start to scan
; out: CF=1 => no more partitions
;      CF=0 => eax=next partition number

fs_NextRamdisk:
; we always have /rd/1
	test	eax, eax
	stc
	jnz	@f
	mov	al, 1
	clc
@@:
	ret

fs_NextFloppy:
; we have /fd/1 iff (([DRIVE_DATA] and 0xF0) != 0) and /fd/2 iff (([DRIVE_DATA] and 0x0F) != 0)
	test	byte [DRIVE_DATA], 0xF0
	jz	.no1
	test	eax, eax
	jnz	.no1
	inc	eax
	ret	; CF cleared
.no1:
	test	byte [DRIVE_DATA], 0x0F
	jz	.no2
	cmp	al, 2
	jae	.no2
	mov	al, 2
	clc
	ret
.no2:
	stc
	ret

; on hdx, we have partitions from 1 to [0x40002+x]
fs_NextHd0:
	push	0
	jmp	fs_NextHd
fs_NextHd1:
	push	1
	jmp	fs_NextHd
fs_NextHd2:
	push	2
	jmp	fs_NextHd
fs_NextHd3:
	push	3
fs_NextHd:
	pop	ecx
	movzx	ecx, byte [DRIVE_DATA+2+ecx]
	cmp	eax, ecx
	jae	fs_NextFloppy.no2
	inc	eax
	clc
	ret

;*******************************************************
fs_NextCd:
; we always have /cdX/1
	test	eax, eax
	stc
	jnz	@f
	mov	al, 1
	clc
@@:
	ret
;*******************************************************

; Additional FS handlers.
; This handler gets the control each time when fn 70 is called
; with unknown item of root subdirectory.
; in: esi -> name
;     ebp = 0 or rest of name relative to esi
; out: if the handler processes path, he must not return in file_system_lfn,
;      but instead pop return address and return directly to the caller
;      otherwise simply return

; here we test for /bd<N>/... - BIOS disks
biosdisk_handler:
        cmp     [NumBiosDisks], 0
        jz      .ret
        mov     al, [esi]
        or      al, 20h
        cmp     al, 'b'
        jnz     .ret
        mov     al, [esi+1]
        or      al, 20h
        cmp     al, 'd'
        jnz     .ret
        push    esi
        inc     esi
        inc     esi
        cmp     byte [esi], '0'
        jb      .ret2
        cmp     byte [esi], '9'
        ja      .ret2
        xor     edx, edx
@@:
        lodsb
        test    al, al
        jz      .ok
        cmp     al, '/'
        jz      .ok
        sub     al, '0'
        cmp     al, 9
        ja      .ret2
        lea     edx, [edx*5]
        lea     edx, [edx*2+eax]
        jmp     @b
.ret2:
        pop     esi
.ret:
        ret
.ok:
        cmp     al, '/'
        jz      @f
        dec     esi
@@:
        add     dl, 80h
        xor     ecx, ecx
@@:
        cmp     dl, [BiosDisksData+ecx*4]
        jz      .ok2
        inc     ecx
        cmp     ecx, [NumBiosDisks]
        jb      @b
        jmp     .ret2
.ok2:
        add     esp, 8
        test    al, al
        jnz     @f
        mov     esi, fs_BdNext
        jmp     file_system_lfn.maindir_noesi
@@:
        push    ecx
        push    ecx
        push    biosdisk_cleanup
        push    fs_OnBd
        mov     edi, esp
        jmp     file_system_lfn.found2

fs_BdNext:
        cmp     eax, [BiosDiskPartitions+ecx*4]
	inc	eax
	cmc
biosdisk_cleanup:
	ret

fs_OnBd:
        pop     edx edx edx edx
; edx = disk number, ecx = partition number
; esi+ebp = name
	call	reserve_hd1
	add     edx, 0x80
	mov     [hdpos], edx
	cmp     ecx, [BiosDiskPartitions+(edx-0x80)*4]
	jmp     fs_OnHdAndBd

; This handler is called when virtual root is enumerated
; and must return all items which can be handled by this.
; It is called several times, first time with eax=0
; in: eax = 0 for first call, previously returned value for subsequent calls
; out: eax = 0 => no more items
;      eax != 0 => buffer pointed to by edi contains name of item

; here we enumerate existing BIOS disks /bd<N>
biosdisk_enum_root:
        cmp     eax, [NumBiosDisks]
        jae     .end
        push    eax
        movzx   eax, byte [BiosDisksData+eax*4]
        sub     al, 80h
        push    eax
        mov     al, 'b'
        stosb
        mov     al, 'd'
        stosb
        pop     eax
        cmp     al, 10
        jae     .big
        add     al, '0'
        stosb
        mov     byte [edi], 0
        pop     eax
        inc     eax
        ret
.end:
        xor     eax, eax
        ret
.big:
        push    ecx edx
        push    -'0'
        mov     ecx, 10
@@:
        xor     edx, edx
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
        xchg    eax, edx
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
        pop     edx ecx
        pop     eax
        inc     eax
        ret

process_replace_file_name:
	mov	ebp, [full_file_name_table]
	mov	edi, [full_file_name_table.size]
	dec	edi
	shl	edi, 7
	add	edi, ebp
.loop:
	cmp	edi, ebp
	jb	.notfound
	push	esi edi
@@:
	cmp	byte [edi], 0
	jz	.dest_done
	lodsb
	test	al, al
	jz	.cont
	or	al, 20h
	scasb
	jz	@b
	jmp	.cont
.dest_done:
	cmp	byte [esi], 0
	jz	.found
	cmp	byte [esi], '/'
	jnz	.cont
	inc	esi
	jmp	.found
.cont:
	pop	edi esi
	sub	edi, 128
	jmp	.loop
.found:
	pop	edi eax
	mov	ebp, esi
	cmp	byte [esi], 0
	lea	esi, [edi+64]
	jnz	.ret
.notfound:
	xor	ebp, ebp
.ret:
	ret

sys_current_directory:
;	mov	esi, [current_slot]
;	mov	esi, [esi+APPDATA.cur_dir]
;	mov	edx, esi

;get length string of appdata.cur_dir
	mov	eax, [current_slot]
	mov	edi, [eax+APPDATA.cur_dir]

	dec	ebx
	jz	.set
	dec	ebx
	jz	.get
	ret
.get:
; sysfunction 30.2: [for app] eax=30,ebx=2,ecx->buffer,edx=len
; for our code: ebx->buffer,ecx=len
max_cur_dir	equ	0x1000

	mov	ebx,edi

	push	ecx
	push	edi

	xor	eax,eax
	mov	ecx,max_cur_dir

	repne	scasb		;find zerro at and string
	jnz     .error		; no zero in cur_dir: internal error, should not happen

	sub	edi,ebx		;lenght for copy
	inc	edi
        mov	[esp+32+8],edi	;return in eax

	cmp	edx, edi
	jbe     @f
	mov     edx, edi
@@:
;source string
	pop	esi
;destination string
	pop	edi
	cmp	edx, 1
	jbe	.ret

	mov	al,'/'		;start string with '/'
	stosb
	mov	ecx,edx
	rep	movsb           ;copy string
.ret:	ret

.error: add	esp,8
	or	dword [esp+32],-1	;error not found zerro at string ->[eax+APPDATA.cur_dir]
	ret
.set:
; sysfunction 30.1: [for app] eax=30,ebx=1,ecx->string
; for our code: ebx->string to set
; use generic resolver with APPDATA.cur_dir as destination
	push	max_cur_dir	;0x1000
	push	edi	;destination
	mov	ebx,ecx
	call	get_full_file_name
	ret

; in: ebx = file name, [esp+4] = destination, [esp+8] = sizeof destination
; destroys all registers except ebp,esp
get_full_file_name:
	push	ebp
	mov	esi, [current_slot]
	mov	esi, [esi+APPDATA.cur_dir]
	mov	edx, esi
@@:
	inc	esi
	cmp	byte [esi-1], 0
	jnz	@b
	dec	esi
	cmp	byte [ebx], '/'
	jz	.set_absolute
; string gives relative path
	mov	edi, [esp+8]	; destination
.relative:
	cmp	byte [ebx], 0
	jz	.set_ok
	cmp	word [ebx], '.'
	jz	.set_ok
	cmp	word [ebx], './'
	jnz	@f
	add	ebx, 2
	jmp	.relative
@@:
	cmp	word [ebx], '..'
	jnz	.doset_relative
	cmp	byte [ebx+2], 0
	jz	@f
	cmp	byte [ebx+2], '/'
	jnz	.doset_relative
@@:
	dec	esi
	cmp	byte [esi], '/'
	jnz	@b
	add	ebx, 3
	jmp	.relative
.set_ok:
	cmp	edx, edi	; is destination equal to APPDATA.cur_dir?
	jz	.set_ok.cur_dir
	sub	esi, edx
	cmp	esi, [esp+12]
	jb	.set_ok.copy
.fail:
	mov	byte [edi], 0
	xor	eax, eax	; fail
	pop	ebp
	ret	8
.set_ok.copy:
	mov	ecx, esi
	mov	esi, edx
	rep	movsb
	mov	byte [edi], 0
.ret.ok:
	mov	al, 1	; ok
	pop	ebp
	ret	8
.set_ok.cur_dir:
	mov	byte [esi], 0
	jmp	.ret.ok
.doset_relative:
	cmp	edx, edi
	jz	.doset_relative.cur_dir
	sub	esi, edx
	cmp	esi, [esp+12]
	jae	.fail
	mov	ecx, esi
	mov	esi, edx
	mov	edx, edi
	rep	movsb
	jmp	.doset_relative.copy
.doset_relative.cur_dir:
	mov	edi, esi
.doset_relative.copy:
	add	edx, [esp+12]
	mov	byte [edi], '/'
	inc	edi
	cmp	edi, edx
	jae	.overflow
@@:
	mov	al, [ebx]
	inc	ebx
	stosb
	test	al, al
	jz	.ret.ok
	cmp	edi, edx
	jb	@b
.overflow:
	dec	edi
	jmp	.fail
.set_absolute:
	lea	esi, [ebx+1]
	call	process_replace_file_name
	mov	edi, [esp+8]
	mov	edx, [esp+12]
	add	edx, edi
.set_copy:
	lodsb
	stosb
	test	al, al
	jz	.set_part2
.set_copy_cont:
	cmp	edi, edx
	jb	.set_copy
	jmp	.overflow
.set_part2:
	mov	esi, ebp
	xor	ebp, ebp
	test	esi, esi
	jz	.ret.ok
	mov	byte [edi-1], '/'
	jmp	.set_copy_cont
