# strchr() Author: Kees J. Bot 1 Jan 1994

# char *strchr(const char *s, int c)
# Look for a character in a string.

.intel_syntax

.globl _strchr

.text
           .align  16
_strchr:
           push    ebp
           mov     ebp, esp
           push    edi
           cld
           mov     edi, [ebp+8]    # edi = string
           mov     edx, 16         # Look at small chunks of the string
next:
           shl     edx, 1          # Chunks become bigger each time
           mov     ecx, edx
           xorb    al, al          # Look for the zero at the end
           repne scasb

           pushf                   # Remember the flags
           sub     ecx, edx
           neg     ecx             # Some or all of the chunk
           sub     edi, ecx        # Step back
           movb    al, [ebp+12]    # The character to look for
           repne scasb
           je      found

           popf                    # Did we find the end of string earlier?

           jne     next            # No, try again

           xor     eax, eax        # Return NULL
           pop     edi
           pop     ebp
           ret
found:
           pop     eax             # Get rid of those flags
           lea     eax, [edi-1]    # Address of byte found
           pop     edi
           pop     ebp
           ret
