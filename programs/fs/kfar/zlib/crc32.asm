; crc32.asm -- compute the CRC-32 of a data stream
; Copyright (C) 1995-2006, 2010, 2011, 2012 Mark Adler
; For conditions of distribution and use, see copyright notice in zlib.inc

; Thanks to Rodney Brown <rbrown64@csc.com.au> for his contribution of faster
; CRC methods: exclusive-oring 32 bits of data at a time, and pre-computing
; tables for updating the shift register in one step with three exclusive-ors
; instead of four steps with four exclusive-ors.  This results in about a
; factor of two increase in speed on a Power PC G4 (PPC7455) using gcc -O3.


;  Note on the use of DYNAMIC_CRC_TABLE: there is no mutex or semaphore
;  protection on the static variables used to control the first-use generation
;  of the crc tables.  Therefore, if you #define DYNAMIC_CRC_TABLE, you should
;  first call get_crc_table() to initialize the tables before allowing more than
;  one thread to use crc32().

; Definitions for doing the crc four data bytes at a time.

if DYNAMIC_CRC_TABLE eq 1

align 4
crc_table_empty dd 1

;  Generate tables for a byte-wise 32-bit CRC calculation on the polynomial:
;  x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

;  Polynomials over GF(2) are represented in binary, one bit per coefficient,
;  with the lowest powers in the most significant bit.  Then adding polynomials
;  is just exclusive-or, and multiplying a polynomial by x is a right shift by
;  one.  If we call the above polynomial p, and represent a byte as the
;  polynomial q, also with the lowest power in the most significant bit (so the
;  byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is (q*x^32) mod p,
;  where a mod b means the remainder after dividing a by b.

;  This calculation is done using the shift-register method of multiplying and
;  taking the remainder.  The register is initialized to zero, and for each
;  incoming bit, x^32 is added mod p to the register if the bit is a one (where
;  x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
;  x (which is shifting right by one and adding x^32 mod p if the bit shifted
;  out is a one).  We start with the highest power (least significant bit) of
;  q and repeat for all eight bits of q.

;  The first table is simply the CRC of all possible eight bit values.  This is
;  all the information needed to generate CRCs on data a byte at a time for all
;  combinations of CRC register values and incoming bytes.  The remaining tables
;  allow for word-at-a-time CRC calculation for both big-endian and little-
;  endian machines, where a word is four bytes.

;void ()
align 4
proc make_crc_table uses ecx edx edi
zlib_debug 'make_crc_table'

	; generate a crc for every 8-bit value
	xor edx, edx
	mov edi, crc_table
.1:
	mov ecx, 8
	mov eax, edx
.2:
	shr eax, 1
	jnc @f
	xor eax, 0xEDB88320
@@:
	loop .2
	stosd
	inc dl
	jnz .1

	mov dword[crc_table_empty],0
	ret
endp

else ;!DYNAMIC_CRC_TABLE
; ========================================================================
; Tables of CRC-32s of all single-byte values, made by make_crc_table().

;include 'crc32.inc'
end if ;DYNAMIC_CRC_TABLE

; =========================================================================
; This function can be used by asm versions of crc32()

;const z_crc_t* ()
align 4
proc get_crc_table
if DYNAMIC_CRC_TABLE eq 1
	cmp dword[crc_table_empty],0
	je @f ;if (..)
		call make_crc_table
	@@:
end if
	mov eax,crc_table
	ret
endp

; =========================================================================
;unsigned long (unsigned long crc, unsigned char *buf, uInt len)
align 4
proc calc_crc32 uses ecx esi, p1crc:dword, buf:dword, len:dword
	xor eax,eax
	mov esi,[buf]

	cmp esi,Z_NULL
	je .end_f ;if (..==0) return 0

if DYNAMIC_CRC_TABLE eq 1
	cmp dword[crc_table_empty],0
	je @f ;if (..)
		call make_crc_table
	@@:
end if

	mov eax,[p1crc]
	mov ecx,[len]
	push edx
	call crc_continue
	pop edx
.end_f:
	ret
endp

GF2_DIM equ 32 ;dimension of GF(2) vectors (length of CRC)

; =========================================================================
;unsigned long (unsigned long *mat, unsigned long vec)
align 4
proc gf2_matrix_times, mat:dword, vec:dword
;    unsigned long sum;

;    sum = 0;
;    while (vec) {
;        if (vec & 1)
;            sum ^= *mat;
;        vec >>= 1;
;        mat++;
;    }
;    return sum;
	ret
endp

; =========================================================================
;local void (unsigned long *square, unsigned long *mat)
align 4
proc gf2_matrix_square, square:dword, mat:dword
;    int n;

;    for (n = 0; n < GF2_DIM; n++)
;        square[n] = gf2_matrix_times(mat, mat[n]);
	ret
endp

; =========================================================================
;uLong (uLong crc1, uLong crc2, z_off64_t len2)
align 4
proc crc32_combine_, crc1:dword, crc2:dword, len2:dword
;    int n;
;    unsigned long row;
;    unsigned long even[GF2_DIM];    /* even-power-of-two zeros operator */
;    unsigned long odd[GF2_DIM];     /* odd-power-of-two zeros operator */

	; degenerate case (also disallow negative lengths)
;    if (len2 <= 0)
;        return crc1;

	; put operator for one zero bit in odd
;    odd[0] = 0xedb88320UL;          /* CRC-32 polynomial */
;    row = 1;
;    for (n = 1; n < GF2_DIM; n++) {
;        odd[n] = row;
;        row <<= 1;
;    }

	; put operator for two zero bits in even
;    gf2_matrix_square(even, odd);

	; put operator for four zero bits in odd
;    gf2_matrix_square(odd, even);

	; apply len2 zeros to crc1 (first square will put the operator for one
	; zero byte, eight zero bits, in even)
;    do {
		; apply zeros operator for this bit of len2
;        gf2_matrix_square(even, odd);
;        if (len2 & 1)
;            crc1 = gf2_matrix_times(even, crc1);
;        len2 >>= 1;

	; if no more bits set, then done
;        if (len2 == 0)
;            break;

	; another iteration of the loop with odd and even swapped
;        gf2_matrix_square(odd, even);
;        if (len2 & 1)
;            crc1 = gf2_matrix_times(odd, crc1);
;        len2 >>= 1;

	; if no more bits set, then done
;    } while (len2 != 0);

	; return combined crc
;    crc1 ^= crc2;
;    return crc1;
	ret
endp

; =========================================================================
;uLong (uLong crc1, uLong crc2, z_off_t len2)
align 4
proc crc32_combine, crc1:dword, crc2:dword, len2:dword
	stdcall crc32_combine_, [crc1], [crc2], [len2]
	ret
endp

;uLong (uLong crc1, uLong crc2, z_off64_t len2)
align 4
proc crc32_combine64, crc1:dword, crc2:dword, len2:dword
	stdcall crc32_combine_, [crc1], [crc2], [len2]
	ret
endp
