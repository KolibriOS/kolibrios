

.section .text

.global _tls_alloc
.global __tls_map

.align 4
_tls_alloc:

           pushl $tls_mutex
           call ___mutex_lock
           popl %eax

           movl tls_map_start, %edx
.align 4
.test:
           bsfl (%edx), %eax
           jnz .done

           add $4, %edx
           cmpl $128+__tls_map, %edx
           jb .test

           xorl %eax, %eax
           mov %eax, tls_mutex
           ret

.done:
           btrl %eax, (%edx)
           movl %edx, tls_map_start
           movl $0, tls_mutex

           subl $__tls_map, %edx
           leal (%eax, %edx, 8), %eax
           shll $2, %eax
           ret

.section .data

tls_mutex:         .long(0)
tls_map_start:     .long(__tls_map)

.section .bss

.align 16

__tls_map:         .space 128
