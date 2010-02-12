# strncmp()  Author: Kees J. Bot 1 Jan 1994

# int strncmp(const char *s1, const char *s2, size_t ecx)
#     Compare two strings.
#

.intel_syntax

.globl __strncmp

.text
           .align  16
__strncmp:
           push    ebp
           mov     ebp, esp

           push    esi
           push    edi

           test    ecx, ecx         # Max length is zero?
           je      done

           mov     esi, [ebp+8]     # esi = string s1
           mov     edi, [ebp+12]    # edi = string s2
           cld
compare:
           cmpsb                    # Compare two bytes
           jne     done

           cmpb    [esi-1], 0       # End of string?
           je      done

           dec     ecx              # Length limit reached?
           jne     compare
done:
           seta    al               # al = (s1 > s2)
           setb    ah               # ah = (s1 < s2)
           subb    al, ah
           movsx  eax, al           # eax = (s1 > s2) - (s1 < s2), i.e. -1, 0, 1

           pop     edi
           pop     esi
           pop     ebp
           ret
