;---------------------------------------------------------------------
Get_ini:

	stdcall dll.Init,[lib_init]	
	
	mov		ebx,ini_file_name
    mov		esi,path
    mov		edi,plugin_pach
    call	copy_file_path

	invoke	ini_get_int,plugin_pach,aVariables,aSortMode,2
	cmp     eax,0
	jb      .error_aSortMode
	cmp     eax,13
	jb      @f
.error_aSortMode:
	mov     eax,2
@@:
	mov		[sort_type],eax
	invoke	ini_get_int,plugin_pach,aVariables,aArrowsOffset,10	
	cmp     eax,1
	jb      .error_aArrowsOffset
	cmp     eax,100
	jb      @f
.error_aArrowsOffset:
	mov     eax,10
@@:
	mov		[arrows_offset],eax
	invoke	ini_get_int,plugin_pach,aVariables,aSlideShowDelay,300
	cmp     eax,1
	jb      .error_aSlideShowDelay
	cmp     eax,99990
	jb      @f
.error_aSlideShowDelay:
	mov     eax,300
@@:
	mov		[slide_show_delay],eax
	invoke	ini_get_int,plugin_pach,aVariables,aShowFileName,0
	cmp     eax,0
	jb      .error_aShowFileName
	cmp     eax,1
	jb      @f
.error_aShowFileName:
	mov     eax,1
@@:
	shl     eax,1
	mov		[check1+32],eax
	invoke	ini_get_int,plugin_pach,aVariables,aFilteringFlag,0
	cmp     eax,0
	jb      .error_aFilteringFlag
	cmp     eax,1
	jb      @f
.error_aFilteringFlag:
	mov     eax,1
@@:
	mov		[filtering_flag],eax
	shl     eax,1
	mov		[check2+32],eax
	
;	get convert plugins names
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP0,plugin_0_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP1,plugin_1_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP2,plugin_2_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP3,plugin_3_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP4,plugin_4_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP5,plugin_5_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP6,plugin_6_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP7,plugin_7_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP8,plugin_8_name,127,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aConvertPlugins,aCP9,plugin_9_name,127,ConvertPlugins_no_key
	
	invoke ini_get_str,plugin_pach,aApplicationsPaths,aHelp,Help_path,1023,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aApplicationsPaths,aEditor,Editor_path,1023,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aApplicationsPaths,aOpenDialog,OpenDialog_path,1023,ConvertPlugins_no_key
	invoke ini_get_str,plugin_pach,aOther,akey_help,key_help_name,127,ConvertPlugins_no_key
	ret
;---------------------------------------------------------------------	
Set_ini:
	mov		ebx,ini_file_name
    mov		esi,path
    mov		edi,plugin_pach
    call	copy_file_path

	invoke	ini_set_int,plugin_pach,aVariables,aSortMode,[sort_type]
	invoke	ini_set_int,plugin_pach,aVariables,aArrowsOffset,[arrows_offset]
	invoke	ini_set_int,plugin_pach,aVariables,aSlideShowDelay,[slide_show_delay]
	mov  eax,[check1+32]
	shr  eax,1
	mov  [check1+32],eax
	invoke	ini_set_int,plugin_pach,aVariables,aShowFileName,[check1+32]
	invoke	ini_set_int,plugin_pach,aVariables,aFilteringFlag,[filtering_flag]	
	ret
;---------------------------------------------------------------------
;libini_alloc:
;	push    ecx
;	mov     ecx, [esp+8]
;	mcall	68,12
;	pop     ecx
;	ret     4
;---------------------------------------------------------------------
;libini_free:
;	push    ecx
;	mov     ecx, [esp+8]
;	mcall	68,13
;	pop     ecx
;	ret     4
;---------------------------------------------------------------------
;libini_realloc:
;	push    ecx edx
;	mov     edx, [esp+8+4]
;	mov     ecx, [esp+8+8]
;	mcall	68,20
;	pop     edx ecx
;	ret     8
;---------------------------------------------------------------------