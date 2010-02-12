#       strcat()                                        Author: Kees J. Bot
#                                                               1 Jan 1994
# char *strcat(char *s1, const char *s2)
#       Append string s2 to s1.
#

        .intel_syntax

.global _strcat

        .text
        .align  16
_strcat:
        mov     edx, -1         # Unlimited length
        jmp     __strncat       # Common code
