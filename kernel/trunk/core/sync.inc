;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; Synhronization for MenuetOS.                                 ;;
;; Author: Halyavin Andrey, halyavin@land.ru                    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


if ~defined sync_inc
sync_inc_fix:
sync_inc fix sync_inc_fix

;simplest mutex.
macro SimpleMutex name
{
;  iglobal
    name dd 0
    name#.type = 1
;  endg
}
macro WaitSimpleMutex name
{
  local start_wait,ok
start_wait=$
        cli
        cmp     [name], dword 0
        jz      ok
        sti
        call    change_task
        jmp     start_wait
ok=$
        push    eax
        mov     eax, dword [TASK_BASE+second_base_address]
        mov     eax, [eax+TASKDATA.pid]
        mov     [name], eax
        pop     eax
        sti
}
macro ReleaseSimpleMutex name
{
        mov     [name], dword 0
}
macro TryWaitSimpleMutex name  ;result in eax and in flags
{
  local ok,try_end
        cmp     [name], dword 0
        jz      ok
        xor     eax, eax
        jmp     try_end
ok=$
        xor     eax, eax
        inc     eax
try_end=$
}
macro SimpleCriticalSection name
{
;  iglobal
    name  dd 0
          dd 0
    name#.type=2
;  endg
}
macro WaitSimpleCriticalSection name
{
  local start_wait,first_wait,inc_counter,end_wait
        push    eax
        mov     eax, [TASK_BASE+second_base_address]
        mov     eax, [eax+TASKDATA.pid]
start_wait=$
        cli
        cmp     [name], dword 0
        jz      first_wait
        cmp     [name], eax
        jz      inc_counter
        sti
        call    change_task
        jmp     start_wait
first_wait=$
        mov     [name], eax
        mov     [name+4], dword 1
        jmp     end_wait
inc_counter=$
        inc     dword [name+4]
end_wait=$
        sti
        pop     eax
}
macro ReleaseSimpleCriticalSection name
{
  local release_end
        dec     dword [name+4]
        jnz     release_end
        mov     [name], dword 0
release_end=$
}
macro TryWaitSimpleCriticalSection name ;result in eax and in flags
{
  local ok,try_end
        mov     eax, [CURRENT_TASK+second_base_address]
        mov     eax, [eax+TASKDATA.pid]
        cmp     [name], eax
        jz      ok
        cmp     [name], 0
        jz      ok
        xor     eax, eax
        jmp     try_end
ok=$
        xor     eax, eax
        inc     eax
try_end=$
}
_cli equ call MEM_HeapLock
_sti equ call MEM_HeapUnLock
end if

