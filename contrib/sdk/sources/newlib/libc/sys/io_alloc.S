
.section .text

.global ___io_alloc
.global ___io_free
.global ___io_tab

.align 8
___io_alloc:
           pushl $__io_mutex
           call ___mutex_lock
           popl %eax

           xorl %ecx, %ecx
           bsfl __io_map, %eax
           jnz .done

           movl $32, %ecx
           bsfl __io_map+4, %eax
           jnz .done

           xorl %eax, %eax
           mov %eax, __io_mutex
           notl %eax
           ret

.done:
           addl %ecx, %eax
           btrl %eax, __io_map
           movl $0, __io_mutex
           ret

.align 8
___io_free:
           movl 4(%esp), %eax
           bts  %eax, __io_map
           ret

.section .data

__io_map:         .long(0xFFFFFFFF)
                  .long(0xFFFFFFFF)

.section .bss

.align 16

___io_tab:        .space 64*24
__io_mutex:       .space 4

