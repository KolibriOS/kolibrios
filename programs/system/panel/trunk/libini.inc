;-----------------------------------------------------------------------------
Get_ini:
	stdcall dll.Init,[lib_init]	
	
	mov	ebx,ini_file_name
	mov	esi,path
	mov	edi,library_path
	call	copy_file_path
;------------------------------------------------------------------------------	
; Get panel variables
;------------------------------------------------------------------------------
	invoke	ini_get_int,library_path,aVariables,aPanelHeight,20
	cmp     eax,100
	ja      @f

	cmp     eax,10
	ja      .height
@@:
	mov     eax,20
.height:
	mov	[height],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aVariables,aPanelWidth,0
	cmp     eax,200
	ja      @f

	mov     eax,0
@@:
	mov	[width],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aVariables,aSoftenHeight,3
	cmp     eax,10
	ja      @f

	cmp     eax,1
	ja      .soften_height
@@:
	mov     eax,3
.soften_height:
	mov	[soften_height],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aVariables,aButtonTopOffset,3
	cmp     eax,10
	ja      @f

	cmp     eax,1
	ja      .button_top_offset
@@:
	mov     eax,3
.button_top_offset:
	mov	[button_top_offset],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aVariables,aButtonBottomOffset,3
	cmp     eax,10
	ja      @f

	cmp     eax,1
	ja      .button_bottom_offset
@@:
	mov     eax,3
.button_bottom_offset:
	mov	[button_bottom_offset],eax
;------------------------------------------------------------------------------
; Get panel flags
;------------------------------------------------------------------------------
	invoke	ini_get_int,library_path,aFlags,aSoftenUp,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[soften_up],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aFlags,aSoftenDown,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[soften_down],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aFlags,aMinimizeLeftButton,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[minimize_left],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aFlags,aMinimizeRightButton,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[minimize_right],eax
;--------------------------------------	
	invoke	ini_get_int,library_path,aFlags,aMenuButton,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[menu_enable],eax
;--------------------------------------	
	invoke	ini_get_int,library_path,aFlags,aRunApplButtons,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[run_appl],eax
;--------------------------------------	
	invoke	ini_get_int,library_path,aFlags,aCleanDesktopButton,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[clean_desktop_enable],eax
;--------------------------------------	
	invoke	ini_get_int,library_path,aFlags,aClock,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[clock_enable],eax
;--------------------------------------	
	invoke	ini_get_int,library_path,aFlags,aCpuUsage,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[cpu_usage_enable],eax
;--------------------------------------	
	invoke	ini_get_int,library_path,aFlags,aChangeLang,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[chlang_enable],eax
;--------------------------------------	
	invoke	ini_get_int,library_path,aFlags,aPageList,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[page_list_enable],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aFlags,aAttachment,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[place_attachment],eax
;--------------------------------------
	invoke	ini_get_int,library_path,aFlags,aButtonsStyle,1
	cmp     eax,2
	jb      @f

	mov     eax,1
@@:
	mov	[button_style],eax
;--------------------------------------
	invoke	ini_get_color,library_path,aColors,aMenuButton,0x44aa44
	mov	[MenuButton_color],eax
;--------------------------------------
	invoke	ini_get_color,library_path,aColors,aCleanDesktopButton,0x66cc
	mov	[CleanDesktopButton_color],eax
;--------------------------------------
	invoke	ini_get_color,library_path,aColors,aClock,0x66cc
	mov	[Clock_color],eax
;--------------------------------------
	invoke	ini_get_color,library_path,aColors,aCpuUsage,0x44aa44
	mov	[CpuUsage_color],eax
;--------------------------------------
	invoke	ini_get_color,library_path,aColors,aCpuUsageBckgr,0xdd2222
	mov	[CpuUsageBckgr_color],eax
;--------------------------------------
	invoke	ini_get_color,library_path,aColors,aChangeLang,0x66cc
	mov	[ChangeLang_color],eax
;--------------------------------------
	invoke	ini_get_color,library_path,aColors,aPageList,0xffffff
	mov	[PageList_color],eax
;--------------------------------------
;	invoke	ini_get_str,library_path,aConvertPlugins,aCP0,plugin_0_name,127,ConvertPlugins_no_key
	
	ret
;-----------------------------------------------------------------------------
copy_file_path:
    xor   eax,eax
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    mov   esi,edi
    dec   esi
@@:
    std
    lodsb
    cmp   al,'/'
    jnz   @b
    mov   edi,esi
    add   edi,2
    mov   esi,ebx
@@:
    cld
    lodsb
    stosb
    test  eax,eax
    jnz   @b
    ret
;-----------------------------------------------------------------------------