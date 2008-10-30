        .file   "export.asm"
	.intel_syntax


	.section .drectve
        .ascii " -export:CreateImage"
        .ascii " -export:LoadFile"

        .ascii " -export:Kmalloc"              #
        .ascii " -export:Kfree"                #

        .ascii " -export:UserAlloc"            # stdcall
        .ascii " -export:UserFree"             # stdcall

        .ascii " -export:MapIoMem"             # stdcall
        .ascii " -export:GetPgAddr"            # eax
        .ascii " -export:CreateObject"         #
        .ascii " -export:DestroyObject"        #
        .ascii " -export:CreateRingBuffer"     # stdcall
        .ascii " -export:CommitPages"          # eax, ebx, ecx

        .ascii " -export:RegService"           # stdcall
        .ascii " -export:UnmapPages"           # eax, ecx
        .ascii " -export:SysMsgBoardStr"       #
        .ascii " -export:SetScreen"            #


        .ascii " -export:PciApi"               #
        .ascii " -export:PciRead8"             # stdcall
        .ascii " -export:PciRead16"            # stdcall
        .ascii " -export:PciRead32"            # stdcall
        .ascii " -export:PciWrite8"            # stdcall
        .ascii " -export:PciWrite16"           # stdcall
        .ascii " -export:PciWrite32"           # stdcall

        .ascii " -export:SelectHwCursor"       # stdcall
        .ascii " -export:SetHwCursor"          # stdcall
        .ascii " -export:HwCursorRestore"      #
        .ascii " -export:HwCursorCreate"       #




