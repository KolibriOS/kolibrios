#include <ddk.h>

#define U64_C(x)  x ## ULL
#define BIT_64(n) (U64_C(1) << (n))

struct e_msr {
	union {
		struct {
			u32 l;
			u32 h;
		};
		u64 q;
	};
};

static void _msr_read(u32 msr, struct e_msr *m)
{
	__asm__ __volatile__(
		"rdmsr"
		:"=a"(m->l), "=d"(m->h)
		:"c"(msr)
		:"memory"
	);
}
 
static void _msr_write(u32 msr, struct e_msr *m)
{
   	__asm__ __volatile__(
	   "wrmsr" 
   		::"a"(m->l), "d"(m->h), "c"(msr)
		:"memory"
	);
}

static int __flip_bit(u32 msr, u8 bit, bool set)
{
	struct e_msr m, m1;
	if (bit > 63)
		return EINVAL;
		
	_msr_read(msr, &m);

	m1 = m;

	if (set)
		m1.q |=  BIT_64(bit);
	else
		m1.q &= ~BIT_64(bit);

	if (m1.q == m.q)
		return 0;

	_msr_write(msr, &m1);
	return 1;
}


int e_msr_set_bit(u32 msr, u8 bit)
{
	return __flip_bit(msr, bit, true);
}