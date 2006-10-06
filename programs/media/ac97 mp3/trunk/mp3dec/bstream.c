/* max bits required for any lookup - change if htable changes */
/* quad required 10 bit w/signs  must have (MAXBITS+2) >= 10   */
#define MAXBITS 9

static unsigned int bitbuf;
static int bits;
static unsigned char *bs_ptr;
static unsigned char *bs_ptr0;
static unsigned char *bs_ptr_end;	// optional for overrun test

void bitget_init(unsigned char *buf)
{
	bs_ptr0 = bs_ptr = buf;
	bits = 0;
	bitbuf = 0;
}

int bitget(int n)
{
	unsigned int x;

	if (bits < n)
	{				/* refill bit buf if necessary */
		while (bits <= 24)
		{
			bitbuf = (bitbuf << 8) | *bs_ptr++;
			bits += 8;
		}
	}
	bits -= n;
	x = bitbuf >> bits;
	bitbuf -= x << bits;
	return x;
}

void bitget_skip(int n)
{
   unsigned int k;

   if (bits < n)
   {
      n -= bits;
      k = n >> 3;
/*--- bytes = n/8 --*/
      bs_ptr += k;
      n -= k << 3;
      bitbuf = *bs_ptr++;
      bits = 8;
   }
   bits -= n;
   bitbuf -= (bitbuf >> bits) << bits;
}

void bitget_init_end(unsigned char *buf_end)
{
   bs_ptr_end = buf_end;
}

/*------------- check overrun -------------*/
int bitget_overrun()
{
	return bs_ptr > bs_ptr_end;
}
/*------------- get n bits from bitstream -------------*/
int bitget_bits_used()
{
   unsigned int n;			/* compute bits used from last init call */

   n = ((bs_ptr - bs_ptr0) << 3) - bits;
   return n;
}
/*------------- check for n bits in bitbuf -------------*/
void bitget_check(int n)
{
   if (bits < n)
   {
      while (bits <= 24)
      {
	 bitbuf = (bitbuf << 8) | *bs_ptr++;
	 bits += 8;
      }
   }
}

#if 0
/*------------- get 1 bit from bitstream -------------*/
int bitget_1bit()
{
   unsigned int x;

   if (bits <= 0)
   {				/* refill bit buf if necessary */
      while (bits <= 24)
      {
	 bitbuf = (bitbuf << 8) | *bs_ptr++;
	 bits += 8;
      }
   }
   bits--;
   x = bitbuf >> bits;
   bitbuf -= x << bits;
   return x;
}

/*------------- get 1 bit from bitstream NO CHECK -------------*/
int bitget_1bit()
{
   unsigned int x;

   bits--;
   x = bitbuf >> bits;
   bitbuf -= x << bits;
   return x;
}
#endif
/* only huffman */

/*----- get n bits  - checks for n+2 avail bits (linbits+sign) -----*/
int bitget_lb(int n)
{
   unsigned int x;

   if (bits < (n + 2))
   {				/* refill bit buf if necessary */
      while (bits <= 24)
      {
	 bitbuf = (bitbuf << 8) | *bs_ptr++;
	 bits += 8;
      }
   }
   bits -= n;
   x = bitbuf >> bits;
   bitbuf -= x << bits;
   return x;
}

/*------------- get n bits but DO NOT remove from bitstream --*/
int bitget2(int n)
{
   unsigned int x;

   if (bits < (MAXBITS + 2))
   {				/* refill bit buf if necessary */
      while (bits <= 24)
      {
	 bitbuf = (bitbuf << 8) | *bs_ptr++;
	 bits += 8;
      }
   }
   x = bitbuf >> (bits - n);
   return x;
}

/*------------- remove n bits from bitstream ---------*/
void bitget_purge(int n)
{
   bits -= n;
   bitbuf -= (bitbuf >> bits) << bits;
}

void mac_bitget_check(int n)
{
	if( bits < n ) {
		while( bits <= 24 ) {
			bitbuf = (bitbuf << 8) | *bs_ptr++;
			bits += 8;
		}
	}
}

int mac_bitget(int n)
{
	unsigned int code;

	bits -= n;
	code  = bitbuf >> bits;
	bitbuf -= code << bits;
    return code;
}

int mac_bitget2(int n)
{
	return (bitbuf >> (bits-n));
}

int mac_bitget_1bit()
{
	unsigned int code;

	bits--;
	code  = bitbuf >> bits;
	bitbuf -= code << bits;
	return code;
}

void mac_bitget_purge(int n)
{
	bits -= n;
    bitbuf -= (bitbuf >> bits) << bits;
}

/*
#define mac_bitget(n) ( bits -= n,           \
         code  = bitbuf >> bits,     \
         bitbuf -= code << bits,     \
         code )

#define mac_bitget_1bit() ( bits--,                           \
         code  = bitbuf >> bits,    \
         bitbuf -= code << bits,  \
         code )
*/