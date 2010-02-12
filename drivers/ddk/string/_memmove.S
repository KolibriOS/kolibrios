# _memmove() Author: Kees J. Bot 2 Jan 1994

# void *_memmove(void *s1, const void *s2, size_t n)
#       Copy a chunk of memory.  Handle overlap.

.intel_syntax

.globl __memmove, __memcpy

.text

           .align  16
__memmove:
           push    ebp
           mov     ebp, esp

           push    esi
           push    edi

           mov     edi, [ebp+8]     # String s1
           mov     esi, [ebp+12]    # String s2
           mov     ecx, [ebp+16]    # Length

           mov     eax, edi
           sub     eax, esi
           cmp     eax, ecx
           jb      downwards        # if (s2 - s1) < n then copy downwards
__memcpy:
           cld                      # Clear direction bit: upwards
           cmp     ecx, 16
           jb      upbyte           # Don't bother being smart with short arrays

           mov     eax, esi
           or      eax, edi
           testb   al, 1
           jnz     upbyte           # Bit 0 set, use byte copy

           testb   al, 2

           jnz     upword           # Bit 1 set, use word copy
uplword:
           shrd    eax, ecx, 2      # Save low 2 bits of ecx in eax
           shr     ecx, 2
           rep movsd                # Copy longwords.
           shld    ecx, eax, 2      # Restore excess count
upword:
           shr     ecx, 1
           rep movsw                # Copy words
           adc     ecx, ecx         # One more byte?
upbyte:
           rep movsb                # Copy bytes
done:
           mov     eax, [ebp+8]     # Absolutely noone cares about this value
           pop     edi
           pop     esi
           pop     ebp
           ret

# Handle bad overlap by copying downwards, don't bother to do word copies.

downwards:
           std                     # Set direction bit: downwards
           lea     esi, [esi+ecx-1]
           lea     edi, [edi+ecx-1]
           rep movsb               # Copy bytes
           cld
           jmp     done
