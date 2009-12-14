
        .global _destroy_cursor
        .global _fini_cursor

.section .text

_destroy_cursor:
        pushl   %eax
        call _fini_cursor
        addl $4, %esp
        ret


