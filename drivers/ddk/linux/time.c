#include <jiffies.h>



#define HZ_TO_MSEC_MUL32 0xA0000000
#define HZ_TO_MSEC_ADJ32 0x0
#define HZ_TO_MSEC_SHR32 28
#define HZ_TO_MSEC_MUL64 0xA000000000000000
#define HZ_TO_MSEC_ADJ64 0x0
#define HZ_TO_MSEC_SHR64 60
#define MSEC_TO_HZ_MUL32 0xCCCCCCCD
#define MSEC_TO_HZ_ADJ32 0x733333333
#define MSEC_TO_HZ_SHR32 35
#define MSEC_TO_HZ_MUL64 0xCCCCCCCCCCCCCCCD
#define MSEC_TO_HZ_ADJ64 0x73333333333333333
#define MSEC_TO_HZ_SHR64 67
#define HZ_TO_MSEC_NUM 10
#define HZ_TO_MSEC_DEN 1
#define MSEC_TO_HZ_NUM 1
#define MSEC_TO_HZ_DEN 10

#define HZ_TO_USEC_MUL32 0x9C400000
#define HZ_TO_USEC_ADJ32 0x0
#define HZ_TO_USEC_SHR32 18
#define HZ_TO_USEC_MUL64 0x9C40000000000000
#define HZ_TO_USEC_ADJ64 0x0
#define HZ_TO_USEC_SHR64 50
#define USEC_TO_HZ_MUL32 0xD1B71759
#define USEC_TO_HZ_ADJ32 0x1FFF2E48E8A7
#define USEC_TO_HZ_SHR32 45
#define USEC_TO_HZ_MUL64 0xD1B71758E219652C
#define USEC_TO_HZ_ADJ64 0x1FFF2E48E8A71DE69AD4
#define USEC_TO_HZ_SHR64 77
#define HZ_TO_USEC_NUM 10000
#define HZ_TO_USEC_DEN 1
#define USEC_TO_HZ_NUM 1
#define USEC_TO_HZ_DEN 10000


#define MSEC_PER_SEC    1000L
#define USEC_PER_MSEC   1000L
#define NSEC_PER_USEC   1000L
#define NSEC_PER_MSEC   1000000L
#define USEC_PER_SEC    1000000L
#define NSEC_PER_SEC    1000000000L
#define FSEC_PER_SEC    1000000000000000LL


unsigned int jiffies_to_msecs(const unsigned long j)
{
#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
        return (MSEC_PER_SEC / HZ) * j;
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
        return (j + (HZ / MSEC_PER_SEC) - 1)/(HZ / MSEC_PER_SEC);
#else
# if BITS_PER_LONG == 32
        return (HZ_TO_MSEC_MUL32 * j) >> HZ_TO_MSEC_SHR32;
# else
        return (j * HZ_TO_MSEC_NUM) / HZ_TO_MSEC_DEN;
# endif
#endif
}

unsigned int jiffies_to_usecs(const unsigned long j)
{
#if HZ <= USEC_PER_SEC && !(USEC_PER_SEC % HZ)
        return (USEC_PER_SEC / HZ) * j;
#elif HZ > USEC_PER_SEC && !(HZ % USEC_PER_SEC)
        return (j + (HZ / USEC_PER_SEC) - 1)/(HZ / USEC_PER_SEC);
#else
# if BITS_PER_LONG == 32
        return (HZ_TO_USEC_MUL32 * j) >> HZ_TO_USEC_SHR32;
# else
        return (j * HZ_TO_USEC_NUM) / HZ_TO_USEC_DEN;
# endif
#endif
}


/*
 * When we convert to jiffies then we interpret incoming values
 * the following way:
 *
 * - negative values mean 'infinite timeout' (MAX_JIFFY_OFFSET)
 *
 * - 'too large' values [that would result in larger than
 *   MAX_JIFFY_OFFSET values] mean 'infinite timeout' too.
 *
 * - all other values are converted to jiffies by either multiplying
 *   the input value by a factor or dividing it with a factor
 *
 * We must also be careful about 32-bit overflows.
 */
unsigned long msecs_to_jiffies(const unsigned int m)
{
    /*
     * Negative value, means infinite timeout:
     */
    if ((int)m < 0)
        return MAX_JIFFY_OFFSET;

#if HZ <= MSEC_PER_SEC && !(MSEC_PER_SEC % HZ)
    /*
     * HZ is equal to or smaller than 1000, and 1000 is a nice
     * round multiple of HZ, divide with the factor between them,
     * but round upwards:
     */
    return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
#elif HZ > MSEC_PER_SEC && !(HZ % MSEC_PER_SEC)
    /*
     * HZ is larger than 1000, and HZ is a nice round multiple of
     * 1000 - simply multiply with the factor between them.
     *
     * But first make sure the multiplication result cannot
     * overflow:
     */
    if (m > jiffies_to_msecs(MAX_JIFFY_OFFSET))
        return MAX_JIFFY_OFFSET;

    return m * (HZ / MSEC_PER_SEC);
#else
    /*
     * Generic case - multiply, round and divide. But first
     * check that if we are doing a net multiplication, that
     * we wouldn't overflow:
     */
    if (HZ > MSEC_PER_SEC && m > jiffies_to_msecs(MAX_JIFFY_OFFSET))
        return MAX_JIFFY_OFFSET;

    return (MSEC_TO_HZ_MUL32 * m + MSEC_TO_HZ_ADJ32)
        >> MSEC_TO_HZ_SHR32;
#endif
}

unsigned long usecs_to_jiffies(const unsigned int u)
{
    if (u > jiffies_to_usecs(MAX_JIFFY_OFFSET))
        return MAX_JIFFY_OFFSET;
#if HZ <= USEC_PER_SEC && !(USEC_PER_SEC % HZ)
    return (u + (USEC_PER_SEC / HZ) - 1) / (USEC_PER_SEC / HZ);
#elif HZ > USEC_PER_SEC && !(HZ % USEC_PER_SEC)
    return u * (HZ / USEC_PER_SEC);
#else
    return (USEC_TO_HZ_MUL32 * u + USEC_TO_HZ_ADJ32)
        >> USEC_TO_HZ_SHR32;
#endif
}

unsigned long
timespec_to_jiffies(const struct timespec *value)
{
    unsigned long sec = value->tv_sec;
    long nsec = value->tv_nsec + TICK_NSEC - 1;

    if (sec >= MAX_SEC_IN_JIFFIES){
            sec = MAX_SEC_IN_JIFFIES;
            nsec = 0;
    }
    return (((u64)sec * SEC_CONVERSION) +
            (((u64)nsec * NSEC_CONVERSION) >>
             (NSEC_JIFFIE_SC - SEC_JIFFIE_SC))) >> SEC_JIFFIE_SC;

}

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

struct timespec ns_to_timespec(const s64 nsec)
{
        struct timespec ts;
        s32 rem;

        if (!nsec)
                return (struct timespec) {0, 0};

        ts.tv_sec = div_s64_rem(nsec, NSEC_PER_SEC, &rem);
        if (unlikely(rem < 0)) {
                ts.tv_sec--;
                rem += NSEC_PER_SEC;
        }
        ts.tv_nsec = rem;

        return ts;
}

