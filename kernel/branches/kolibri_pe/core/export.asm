        .file   "export.asm"
	.intel_syntax


	.section .drectve
#       .ascii " -export:CreateImage"          # cdecl
        .ascii " -export:LoadFile"             # stdcall

        .ascii " -export:Kmalloc"              # eax               FIXME
        .ascii " -export:Kfree"                # eax               FIXME

        .ascii " -export:UserAlloc"            # stdcall
        .ascii " -export:UserFree"             # stdcall

        .ascii " -export:GetPgAddr"            # eax               FIXME
        .ascii " -export:MapIoMem"             # stdcall
        .ascii " -export:CreateRingBuffer"     # stdcall
        .ascii " -export:CommitPages"          # eax, ebx, ecx     FIXME
        .ascii " -export:UnmapPages"           # eax, ecx          FIXME
        .ascii " -export:CreateObject"         # eax, ebx          FIXME
        .ascii " -export:DestroyObject"        # eax

        .ascii " -export:RegService"           # stdcall
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




