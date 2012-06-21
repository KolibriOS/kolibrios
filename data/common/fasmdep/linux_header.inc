; Header for Linux program
        format ELF executable 3
        entry start
; for system calls
include 'unistd.inc'
macro __mov a,b
{
if b eq
else if ~(b eqtype 1)
        mov     a, b
else if b = 0
        xor     a, a
else if (b < 0x80) & (b >= -0x80)
        push    b
        pop     a
else
        mov     a, b
end if
}
macro kercall a,b,c,d,e,f,g
{
        __mov   eax, a
        __mov   ebx, b
        __mov   ecx, c
        __mov   edx, d
        __mov   esi, e
        __mov   edi, f
        __mov   ebp, g
        int     0x80
}
macro stdcall func,[arg]
{
reverse
        pushd   arg
common
        call    func
}
PROT_READ       = 0x1           ; page can be read
PROT_WRITE      = 0x2           ; page can be written
PROT_EXEC       = 0x4           ; page can be executed
PROT_SEM        = 0x8           ; page may be used for atomic ops
PROT_NONE       = 0x0           ; page can not be accessed
PROT_GROWSDOWN  = 0x01000000    ; mprotect flag: extend change to start of growsdown vma
PROT_GROWSUP    = 0x02000000    ; mprotect flag: extend change to end of growsup vma

MAP_SHARED      = 0x01          ; Share changes
MAP_PRIVATE     = 0x02          ; Changes are private
MAP_TYPE        = 0x0f          ; Mask for type of mapping
MAP_FIXED       = 0x10          ; Interpret addr exactly
MAP_ANONYMOUS   = 0x20          ; don't use a file

O_ACCMODE       = 00000003
O_RDONLY        = 00000000
O_WRONLY        = 00000001
O_RDWR          = 00000002
O_CREAT         = 00000100      ; not fcntl
O_EXCL          = 00000200      ; not fcntl
O_NOCTTY        = 00000400      ; not fcntl
O_TRUNC         = 00001000      ; not fcntl
O_APPEND        = 00002000
O_NONBLOCK      = 00004000
O_DSYNC         = 00010000      ; used to be O_SYNC, see below
FASYNC          = 00020000      ; fcntl, for BSD compatibility
O_DIRECT        = 00040000      ; direct disk access hint
O_LARGEFILE     = 00100000
O_DIRECTORY     = 00200000      ; must be a directory
O_NOFOLLOW      = 00400000      ; don't follow links
O_NOATIME       = 01000000
O_CLOEXEC       = 02000000      ; set close_on_exec
__O_SYNC        = 04000000
O_SYNC          = (__O_SYNC + O_DSYNC)
O_NDELAY        = O_NONBLOCK
segment readable executable
