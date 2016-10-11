#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/math64.h>

s64 div_s64_rem(s64 dividend, s32 divisor, s32 *remainder)
{
        u64 quotient;

        if (dividend < 0) {
                quotient = div_u64_rem(-dividend, abs(divisor), (u32 *)remainder);
                *remainder = -*remainder;
                if (divisor > 0)
                        quotient = -quotient;
        } else {
                quotient = div_u64_rem(dividend, abs(divisor), (u32 *)remainder);
                if (divisor < 0)
                        quotient = -quotient;
        }
        return quotient;
}

