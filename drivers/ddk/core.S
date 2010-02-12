
    .file   "export.s"
    .intel_syntax

    .text

    .global _AllocPage
    .global _AllocPages

    .global _CreateRingBuffer
    .global _Delay
    .global _DestroyObject

    .global _FreeKernelSpace

    .global _GetDisplay
    .global _GetPgAddr
    .global _GetService

    .global _KernelAlloc
    .global _KernelFree

    .global _MapIoMem

    .global _PciApi
    .global _PciRead16
    .global _PciRead32
    .global _PciRead8
    .global _PciWrite16
    .global _PciWrite32
    .global _PciWrite8

    .global _RegService

    .global _SetScreen
    .global _SysMsgBoardStr


    .def  _AllocPage;         .scl  2;  .type 32; .endef
    .def  _AllocPages;        .scl  2;  .type 32; .endef

    .def  _CreateRingBuffer;  .scl  2;  .type 32; .endef

    .def  _Delay;             .scl  2;  .type 32; .endef
    .def  _DestroyObject;     .scl  2;  .type 32; .endef

    .def  _FreeKernelSpace;   .scl  2;  .type 32; .endef

    .def  _GetDisplay;        .scl  2;  .type 32; .endef
    .def  _GetPgAddr;         .scl  2;  .type 32; .endef
    .def  _GetService;        .scl  2;  .type 32; .endef

    .def  _KernelAlloc;       .scl  2;  .type 32; .endef
    .def  _KernelFree;        .scl  2;  .type 32; .endef

    .def  _MapIoMem;          .scl  2;  .type 32; .endef

    .def  _PciApi;            .scl  2;  .type 32; .endef
    .def  _PciRead16;         .scl  2;  .type 32; .endef
    .def  _PciRead32;         .scl  2;  .type 32; .endef
    .def  _PciRead8;          .scl  2;  .type 32; .endef
    .def  _PciWrite16;        .scl  2;  .type 32; .endef
    .def  _PciWrite32;        .scl  2;  .type 32; .endef
    .def  _PciWrite8;         .scl  2;  .type 32; .endef

    .def  _RegService;        .scl  2;  .type 32; .endef

    .def  _SetScreen;         .scl  2;  .type 32; .endef
    .def  _SysMsgBoardStr;    .scl  2;  .type 32; .endef


_AllocPage:
_AllocPages:

_CreateRingBuffer:

_Delay:

_DestroyObject:

_FreeKernelSpace:

_GetDisplay:
_GetPgAddr:
_GetService:

_KernelAlloc:
_KernelFree:

_MapIoMem:

_PciApi:
_PciRead16:
_PciRead32:
_PciRead8:
_PciWrite16:
_PciWrite32:
_PciWrite8:

_RegService:

_SetScreen:
_SysMsgBoardStr:
        ret

    .section .drectve

        .ascii " -export:AllocPage"            # gcc ABI
        .ascii " -export:AllocPages"           # gcc ABI

        .ascii " -export:CreateRingBuffer"     # stdcall

        .ascii " -export:Delay"                # stdcall

        .ascii " -export:DestroyObject"

        .ascii " -export:FreeKernelSpace"      # stdcall

        .ascii " -export:GetDisplay"           # stdcall
        .ascii " -export:GetPgAddr"            # stdcall
        .ascii " -export:GetService"           # stdcall

        .ascii " -export:KernelAlloc"          # stdcall
        .ascii " -export:KernelFree"           # stdcall

        .ascii " -export:MapIoMem"             # stdcall

        .ascii " -export:PciApi"               #
        .ascii " -export:PciRead16"            # stdcall
        .ascii " -export:PciRead32"            # stdcall
        .ascii " -export:PciRead8"             # stdcall
        .ascii " -export:PciWrite16"           # stdcall
        .ascii " -export:PciWrite32"           # stdcall
        .ascii " -export:PciWrite8"            # stdcall

        .ascii " -export:RegService"           # stdcall

        .ascii " -export:SetScreen"            # stdcall
        .ascii " -export:SysMsgBoardStr"       # stdcall


