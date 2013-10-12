ORG 0
BITS 32
; ---------------------------------------------------------------------------- ;
STACK_SIZE     equ 256
; ---------------------------------------------------------------------------- ;
MENUET01       db 'MENUET01'
version        dd 1
program.start  dd start_
program.end    dd end_
program.memory dd end_ + STACK_SIZE
program.stack  dd end_ + STACK_SIZE
program.params dd 0
program.path   dd 0
; ---------------------------------------------------------------------------- ;
Partition:
.full_space    dd 0
.free_space    dd 0
; ---------------------------------------------------------------------------- ;
FS_Info:
.cluster_size  dd 0
.all_clusters  dd 0
.free_clusters dd 0
; ---------------------------------------------------------------------------- ;
sz_caption       db "RDInfo",0
sz_all_clusters  db "All clusters:",0
sz_free_clusters db "Free clusters:",0
sz_cluster_size  db "Cluster size:",0
sz_full_space    db "Full space(kb):",0
sz_free_space    db "Free space(kb):",0
; ---------------------------------------------------------------------------- ;
%define buffer [esp + 8]
%define disk   [esp + 4]
get_file_system_info:
        mov    edx, esp
        sub    edx, 24
        mov    [edx], dword 15
        mov    eax, disk
        mov    [edx + 20], eax
        mov    eax, 58
        lea    ebx, [edx]
        int    64
        mov    esi, eax
        mov    edx, [edx]
        mov    eax, buffer
        mov    [eax], edx
        mov    [eax + 4], ebx
        mov    [eax + 8], ecx
        mov    eax, esi
        ret    8
; ---------------------------------------------------------------------------- ;
start_:
; set.event:
        mov    eax, 40
        mov    ebx, 5 ; redraw + button
        int    64
on_redraw:
; redraw.start
        mov    eax, 12
        mov    ebx, 1
        int    64
; draw.window
        xor    eax, eax
        mov    ebx, 200
        mov    ecx, 100
        mov    edx, 0x34CCDDEE
        mov    edi, sz_caption
        int    64
; get.info
        push   dword FS_Info
        push   dword "/rd" ; ramdisk
        call   get_file_system_info

        mov    eax, [FS_Info.all_clusters]
        mul    dword [FS_Info.cluster_size]
        shr    eax, 10
        mov    [Partition.full_space], eax

        mov    eax, [FS_Info.free_clusters]
        mul    dword [FS_Info.cluster_size]
        shr    eax, 10
        mov    [Partition.free_space], eax
; draw.info
        mov    eax, 4
        mov    ecx, 0xC0000000
        mov    edi, 0x00CCDDEE

        mov    ebx, (10 << 16) | 10
        mov    edx, sz_all_clusters
        int    64
        mov    ebx, (10 << 16) | 20
        mov    edx, sz_free_clusters
        int    64
        mov    ebx, (10 << 16) | 30
        mov    edx, sz_cluster_size
        int    64
        mov    ebx, (10 << 16) | 40
        mov    edx, sz_full_space
        int    64
        mov    ebx, (10 << 16) | 50
        mov    edx, sz_free_space
        int    64

        mov    eax, 47
        mov    ebx, (10 << 16) | 0x80000000
        mov    esi, ecx

        mov    ecx, [FS_Info.all_clusters]
        mov    edx, (110 << 16) | 10
        int    64
        mov    ecx, [FS_Info.free_clusters]
        mov    edx, (110 << 16) | 20
        int    64
        mov    ecx, [FS_Info.cluster_size]
        mov    edx, (110 << 16) | 30
        int    64
        mov    ecx, [Partition.full_space]
        mov    edx, (110 << 16) | 40
        int    64
        mov    ecx, [Partition.free_space]
        mov    edx, (110 << 16) | 50
        int    64
; redraw.finish
        mov    eax, 12
        mov    ebx, 2
        int    64
; wait.event
        mov    eax, 10
        int    64
        dec    eax
        jz     on_redraw
; program.terminate:
        or     eax, -1
        int    64
; ---------------------------------------------------------------------------- ;
end_: