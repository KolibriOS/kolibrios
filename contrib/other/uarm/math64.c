#include "math64.h"

#ifndef COMPILER_SUPPORTS_LONG_LONG



UInt64 u64_from_halves(UInt32 hi, UInt32 lo){
	
	UInt64 r;
	
	r.lo = lo;
	r.hi = hi;
	
	return r;	
}


UInt64 u64_32_to_64(UInt32 v){
	
	UInt64 r;
	
	r.hi = 0;
	r.lo = v;
	
	return r;	
}

UInt32 u64_64_to_32(UInt64 v){
	
	return v.lo;	
}

UInt32 u64_get_hi(UInt64 v){
	
	return v.hi;	
}

UInt64 u64_add(UInt64 a, UInt64 b){
	
	UInt64 r;
	
	r.lo = a.lo + b.lo;
	r.hi = a.hi + b.hi;
	if(r.lo < a.lo) r.hi++;
	
	return r;
}

UInt64 u64_add32(UInt64 a, UInt32 b){
	
	UInt64 r;
	
	r.lo = a.lo + b;
	r.hi = a.hi;
	if(r.lo < a.lo) r.hi++;
	
	return r;	
}

UInt64 u64_umul3232(UInt32 a, UInt32 b){
	
	UInt64 r;
	UInt32 ah, al, bh, bl;
	
	ah = a >> 16;
	al = a & 0xFFFFUL;
	bh = b >> 16;
	bl = b & 0xFFFFUL;
	
	r = u64_shl(u64_32_to_64(ah * bh), 16);
	r = u64_add32(r, ah * bl);
	r = u64_add32(r, al * bh);
	r = u64_add32(u64_shl(r, 16), al * bl);
	
	return r;
}

UInt64 u64_smul3232(Int32 a, Int32 b){
	
	Boolean negative = false;
	UInt64 r;
	
	if(a < 0){
		a = -a;
		negative = !negative;
	}
	
	if(b < 0){
		b = -b;
		negative = !negative;
	}
	
	r = u64_umul3232(a, b);
	
	if(negative){
		
		r.hi = ~r.hi;			//negate r
		r.lo = ~r.lo;
		r = u64_inc(r);
	}
	
	return r;
}

UInt64 u64_shr(UInt64 v, unsigned bits){
	
	UInt64 a = v;
	
	while(bits >= 32){
		a.lo = a.hi;
		a.hi = 0;
		bits -= 32;
	}
	
	a.lo = (a.lo >> bits) + (a.hi << (32 - bits));
	a.hi >>= bits;
	
	return a;
}

UInt64 u64_shl(UInt64 v, unsigned bits){
	
	UInt64 a = v;
	
	while(bits >= 32){
		a.hi = a.lo;
		a.lo = 0;
		bits -= 32;
	}
	
	a.hi = (a.hi << bits) + (a.lo >> (32 - bits));
	a.lo <<= bits;
	
	return a;
}

UInt64 u64_xtnd32(UInt64 v){
	
	UInt64 a = v;
	
	if(a.lo & 0x80000000UL) a.hi = 0xFFFFFFFFUL;
	
	return a;
}

Boolean u64_isZero(UInt64 a){
	
	return a.lo == 0 && a.hi == 0;
}

UInt64 u64_inc(UInt64 v){
	
	UInt64 a = v;
	
	if(!++a.lo) a.hi++;
	
	return a;
}

UInt64 u64_and(UInt64 a, UInt64 b){
	
	UInt64 r;
	
	r.lo = a.lo & b.lo;
	r.hi = a.hi & b.hi;
	
	return r;	
}

UInt64 u64_zero(void){
	
	UInt64 r;
	
	r.lo = 0;
	r.hi = 0;
	
	return r;	
}


UInt64 u64_sub(UInt64 a, UInt64 b){
	
	UInt64 bn;
	
	bn.lo = ~b.lo;
	bn.hi = ~b.hi;
	
	bn = u64_inc(bn);
	
	return u64_add(a, bn);
}





#endif