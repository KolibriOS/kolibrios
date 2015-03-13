#define NULL 0
typedef unsigned int size_t;


// sort
#define		THRESH		4		/* threshold for insertion */
#define		MTHRESH		6		/* threshold for median */

static  int		(*qcmp)(const void *, const void *);		/* the comparison routine */
static  int		qsz;			/* size of each record */
static  int		thresh;			/* THRESHold in chars */
static  int		mthresh;		/* MTHRESHold in chars */

/*
* qst:
* Do a quicksort
* First, find the median element, and put that one in the first place as the
* discriminator.  (This "median" is just the median of the first, last and
* middle elements).  (Using this median instead of the first element is a big
* win).  Then, the usual partitioning/swapping, followed by moving the
* discriminator into the right place.  Then, figure out the sizes of the two
* partions, do the smaller one recursively and the larger one via a repeat of
* this code.  Stopping when there are less than THRESH elements in a partition
* and cleaning up with an insertion sort (in our caller) is a huge win.
* All data swaps are done in-line, which is space-losing but time-saving.
* (And there are only three places where this is done).
*/

static void qst(char *base, char *max)
{
	char c, *i, *j, *jj;
	int ii;
	char *mid, *tmp;
	int lo, hi;

/*
* At the top here, lo is the number of characters of elements in the
* current partition.  (Which should be max - base).
* Find the median of the first, last, and middle element and make
* that the middle element.  Set j to largest of first and middle.
* If max is larger than that guy, then it's that guy, else compare
* max with loser of first and take larger.  Things are set up to
* prefer the middle, then the first in case of ties.
*/
	lo = max - base;		/* number of elements as chars */
	do	{
		mid = i = base + qsz * ((lo / qsz) >> 1);
		if (lo >= mthresh)
		{
			j = (qcmp((jj = base), i) > 0 ? jj : i);
			if (qcmp(j, (tmp = max - qsz)) > 0)
			{
/* switch to first loser */
				j = (j == jj ? i : jj);
				if (qcmp(j, tmp) < 0)
					j = tmp;
			}
			if (j != i)
			{
				ii = qsz;
				do	{
					c = *i;
					*i++ = *j;
					*j++ = c;
				} while (--ii);
			}
		}
/*
* Semi-standard quicksort partitioning/swapping
*/
		for (i = base, j = max - qsz; ; )
		{
			while (i < mid && qcmp(i, mid) <= 0)
				i += qsz;
			while (j > mid)
			{
				if (qcmp(mid, j) <= 0)
				{
					j -= qsz;
					continue;
				}
				tmp = i + qsz;		/* value of i after swap */
				if (i == mid)
				{
/* j <-> mid, new mid is j */
					mid = jj = j;
				}
				else
				{
/* i <-> j */
					jj = j;
					j -= qsz;
				}
				goto swap;
			}
			if (i == mid)
			{
				break;
			}
			else
			{
/* i <-> mid, new mid is i */
				jj = mid;
				tmp = mid = i;		/* value of i after swap */
				j -= qsz;
			}
			swap:
			ii = qsz;
			do	{
				c = *i;
				*i++ = *jj;
				*jj++ = c;
			} while (--ii);
			i = tmp;
		}
/*
* Look at sizes of the two partitions, do the smaller
* one first by recursion, then do the larger one by
* making sure lo is its size, base and max are update
* correctly, and branching back.  But only repeat
* (recursively or by branching) if the partition is
* of at least size THRESH.
*/
		i = (j = mid) + qsz;
		if ((lo = j - base) <= (hi = max - i))
		{
			if (lo >= thresh)
				qst(base, j);
			base = i;
			lo = hi;
		}
		else
		{
			if (hi >= thresh)
				qst(i, max);
			max = j;
		}
	} while (lo >= thresh);
}

/*
* qsort:
* First, set up some global parameters for qst to share.  Then, quicksort
* with qst(), and then a cleanup insertion sort ourselves.  Sound simple?
* It's not...
*/

void qsort_g(void *base0, size_t n, size_t size, int (*compar)(const void *, const void *))
{
	char *base = (char *)base0;
	char c, *i, *j, *lo, *hi;
	char *min, *max;

	if (n <= 1)
		return;
	qsz = size;
	qcmp = compar;
	thresh = qsz * THRESH;
	mthresh = qsz * MTHRESH;
	max = base + n * qsz;
	if (n >= THRESH)
	{
		qst(base, max);
		hi = base + thresh;
	}
	else
	{
		hi = max;
	}
/*
* First put smallest element, which must be in the first THRESH, in
* the first position as a sentinel.  This is done just by searching
* the first THRESH elements (or the first n if n < THRESH), finding
* the min, and swapping it into the first position.
*/
	for (j = lo = base; (lo += qsz) < hi; )
		if (qcmp(j, lo) > 0)
			j = lo;
	if (j != base)
	{
/* swap j into place */
		for (i = base, hi = base + qsz; i < hi; )
		{
			c = *j;
			*j++ = *i;
			*i++ = c;
		}
	}
/*
* With our sentinel in place, we now run the following hyper-fast
* insertion sort.  For each remaining element, min, from [1] to [n-1],
* set hi to the index of the element AFTER which this one goes.
* Then, do the standard insertion sort shift on a character at a time
* basis for each element in the frob.
*/
	for (min = base; (hi = min += qsz) < max; )
	{
		while (qcmp(hi -= qsz, min) > 0)
/* void */;
			if ((hi += qsz) != min) {
				for (lo = min + qsz; --lo >= min; )
			{
				c = *lo;
				for (i = j = lo; (j -= qsz) >= hi; i = j)
					*i = *j;
				*i = c;
			}
		}
	}
}

#define STBTT_sort(data,num_items,item_size,compare_func)   qsort_g(data,num_items,item_size,compare_func)

asm ("_floor: \n\t"
"pushl	%ebp\n\t"
"movl	%esp,%ebp\n\t"
"subl	$8,%esp\n\t"      
"fstcw	-4(%ebp)\n\t"
"fwait\n\t"
"movw	-4(%ebp),%ax\n\t"
"andw	$0xf3ff,%ax\n\t"
"orw	$0x0400,%ax\n\t"
"movw	%ax,-2(%ebp)\n\t"
"fldcw	-2(%ebp)\n\t"
"fldl	8(%ebp)\n\t"
"frndint\n\t"
"fldcw	-4(%ebp)\n\t"
"movl	%ebp,%esp\n\t"
"popl	%ebp\n\t"
"ret");


int i_floor (float x) {
	int z;
	z=x;
	if (z+1>x) {return z;} else {return (z+1);}
}

int i_ceil (float x) {
	int z;
	z=x;
	if (z>x) {return z;} else {return (z+1);}
}


double sqrt (double x)
{
	if (x < 0.0F )
	{
		return -1;
	}
	else
	{
		double res;
		asm ("fsqrt" : "=t" (res) : "0" (x));
		return res;
	}
}

#define STBTT_ifloor(x)   ((int) i_floor(x))
#define STBTT_iceil(x)    ((int) i_ceil(x))

static inline void *zmalloc(size_t size) { 
	void *val;
	__asm__ __volatile__( "int $0x40":"=a"(val):"a"(68),"b"(12),"c"(size));
	return val; 
}

static inline void zfree(void *p)
{
	size_t foo;
	asm volatile ("int $0x40":"=a"(foo):"a"(68), "b"(13), "c"(p));
	return;
}

#define STBTT_malloc(x,u)  zmalloc(x)
#define STBTT_free(x,u)    zfree(x)

#define assert_g(ignore)((void) 0)

#define STBTT_assert(x)    assert_g(x)

int strlen_g(const char* string)
{
	int i;
	i=0;
	while (*string++) i++;
	return i;
}

#define STBTT_strlen(x)    strlen_g(x)

void*  zmemset(void *mem, int c, unsigned size)
{
	unsigned i;

	for ( i = 0; i < size; i++ )
		*((char *)mem+i) = (char) c;

	return 0;	
}

void* zmemcpy(void *dst, const void *src, unsigned size)
{

	unsigned i;

	for ( i = 0; i < size; i++)
		*(char *)(dst+i) = *(char *)(src+i);

	return 0;
}

#define STBTT_memcpy       zmemcpy
#define STBTT_memset       zmemset