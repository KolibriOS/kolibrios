# memcpy() Author: Kees J. Bot 2 Jan 1994

# void *memcpy(void *s1, const void *s2, size_t n)
#       Copy a chunk of memory.
#       This routine need not handle overlap, so it does not handle overlap.
#       One could simply call __memmove, the cost of the overlap check is
#       negligible, but you are dealing with a programmer who believes that
#       if anything can go wrong, it should go wrong.

.intel_syntax

.globl _memcpy

.text

           .align  16
_memcpy:
           push    ebp
           mov     ebp, esp
           push    esi
           push    edi
           mov     edi, [ebp+8]    # String s1
           mov     esi, [ebp+12]   # String s2
           mov     ecx, [ebp+16]   # Length
# No overlap check here
           jmp     __memcpy        # Call the part of __memmove that copies up
