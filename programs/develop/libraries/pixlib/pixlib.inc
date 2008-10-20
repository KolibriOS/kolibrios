
REQ_DLL_VER      =  1
DLL_ENTRY        =  1

HS_HORIZONTAL    =  0
HS_VERTICAL      =  1
HS_FDIAGONAL     =  2
HS_BDIAGONAL     =  3
HS_CROSS         =  4
HS_DIAGCROSS     =  5

SCR_PIXMAP       = -1

PX_MEM_SYSTEM    =  0
PX_MEM_LOCAL     =  1
PX_MEM_GART      =  2

PX_MEM_MASK      =  3

ARGB32           = ((32 shl 24) or (2 shl 16) or 0x8888)

macro CreateHatch hatch, bkcolor, fcolor
{
           pushd fcolor
           pushd bkcolor
           pushd hatch
           call [imp_CreateHatch]
           add esp, 3*4
}

macro DestroyBrush brush
{
           pushd brush
           call [imp_DestroyBrush]
           add esp, 4
}

macro CreatePixmap width, height, format, flags
{
           pushd flags
           pushd format
           pushd height
           pushd width
           call [imp_CreatePixmap]
           add esp, 4*4
}

macro DestroyPixmap pixmap
{
          pushd pixmap
          call [imp_DestroyPixmap]
          add esp, 4
};

macro LockPixmap pixmap
{
           pushd pixmap
           call [imp_LockPixmap]
           add esp, 4
}

macro UnlockPixmap pixmap
{
           pushd pixmap
           call [imp_UnlockPixmap]
           add esp, 4
}

macro ClearPixmap pixmap, color
{
           pushd color
           pushd pixmap
           call [imp_ClearPixmap]
           add esp, 2*4
}

macro Line pixmap, x0,y0,x1,y1,color
{
           pushd color
           pushd y1
           pushd x1
           pushd y0
           pushd x0
           pushd pixmap
           call [imp_Line]
           add esp, 6*4
};

macro DrawRect pixmap,x,y,w,h,color,border
{
           pushd border
           pushd color
           pushd h
           pushd w
           pushd y
           pushd x
           pushd pixmap
           call [imp_DrawRect]
           add esp, 7*4
}

macro FillRect pixmap,x,y,w,h,brush,border
{
           pushd border
           pushd brush
           pushd h
           pushd w
           pushd y
           pushd x
           pushd pixmap
           call [imp_FillRect]
           add esp, 7*4
}

macro Blit dstpix, dstx, dsty, srcpix, srcx, srcy, w, h
{
           pushd h
           pushd w
           pushd srcy
           pushd srcx
           pushd srcpix
           pushd dsty
           pushd dstx
           pushd dstpix
           call [imp_Blit]
           add esp, 8*4
}

macro TransparentBlit dstpix, dstx, dsty, srcpix, srcx, srcy, w, h, key
{
           pushd key
           pushd h
           pushd w
           pushd srcy
           pushd srcx
           pushd srcpix
           pushd dsty
           pushd dstx
           pushd dstpix
           call [imp_TransparentBlit]
           add esp, 9*4
}

szPxlib              db  '/rd/1/lib/pixlib.obj',0

szStart              db  'START',0
szVersion            db  'version',0

szCreatePixmap       db  'CreatePixmap',0
szDestroyPixmap      db  'DestroyPixmap',0
szLockPixmap         db  'LockPixmap',0
szUnlockPixmap       db  'UnlockPixmap',0
szGetPixmapPitch     db  'GetPixmapPitch',0

szCreateHatch        db  'CreateHatch',0
szCreateMonoBrush    db  'CreateMonoBrush',0
szDestroyBrush       db  'DestroyBrush',0

szClearPixmap        db  'ClearPixmap',0
szLine               db  'Line',0
szDrawRect           db  'DrawRect',0
szFillRect           db  'FillRect',0
szBlit               db  'Blit',0
szTransparentBlit    db  'TransparentBlit',0

align 4

px_import:

imp_start            dd  szStart
imp_ver              dd  szVersion

imp_CreatePixmap     dd  szCreatePixmap
imp_DestroyPixmap    dd  szDestroyPixmap
imp_LockPixmap       dd  szLockPixmap
imp_UnlockPixmap     dd  szUnlockPixmap
imp_GetPixmapPitch   dd  szGetPixmapPitch

imp_CreateHatch      dd  szCreateHatch
imp_CreateMonoBrush  dd  szCreateMonoBrush
imp_DestroyBrush     dd  szDestroyBrush

imp_ClearPixmap      dd  szClearPixmap
imp_Line             dd  szLine
imp_DrawRect         dd  szDrawRect
imp_FillRect         dd  szFillRect
imp_Blit             dd  szBlit
imp_TransparentBlit  dd  szTransparentBlit

                     dd  0


;szBlockClip          db  'BlockClip',0
;szLineClip           db  'LineClip',0
;imp_BlockClip        dd  szBlockClip
;imp_LineClip         dd  szLineClip


align 4

load_pxlib:
           mov  eax, 68
           mov  ebx, 19
           mov  ecx, szPxlib
           int  0x40
           test eax, eax
           jz   .fail

           mov  edx, eax
           mov  esi, px_import
.import_loop:
           lodsd
           test eax, eax
           jz   .import_done
           push edx
.import_find:
           mov ebx, [edx]
           test ebx, ebx
           jz .fail         ;import_not_found

           push eax
@@:
           mov cl, [eax]
           cmp cl, [ebx]
           jnz .import_find_next

           test cl, cl
           jz .import_found

           inc eax
           inc ebx
           jmp @b
.import_find_next:
           pop eax
           add edx, 8
           jmp .import_find
.import_found:
           pop eax
           mov eax, [edx+4]
           mov [esi-4], eax
           pop edx
           jmp .import_loop
.import_done:

           cmp word [imp_ver], REQ_DLL_VER
           jb .fail
           cmp word [imp_ver+2], REQ_DLL_VER
           ja .fail

           push DLL_ENTRY
           call [imp_start]
           test eax, eax
           jz .fail

           ret
.fail:
           xor eax, eax
           ret


