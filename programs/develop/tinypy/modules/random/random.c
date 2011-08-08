#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include <math.h>
#ifndef M_E
 #define M_E     2.7182818284590452354
#endif
#ifndef M_PI
 #define M_PI    3.14159265358979323846
#endif

/*
 * following random generators are mutual exclusive.
 */
#define __USE_MERSENNE_TWISTER_RANDOM_GENERATOR
/*#define __USE_WICHMANN_HILL_RANDOM_GENERATOR*/
/*#define __USE_POSIX_RANDOM_GENERATOR*/


#if defined(__USE_MERSENNE_TWISTER_RANDOM_GENERATOR)

/***********************************************************
 * following code is borrowed from Python's _randommodule.c
 ***********************************************************/
/* Random objects */

/* ------------------------------------------------------------------
   The code in this module was based on a download from:
      http://www.math.keio.ac.jp/~matumoto/MT2002/emt19937ar.html

   It was modified in 2002 by Raymond Hettinger as follows:

    * the principal computational lines untouched except for tabbing.

    * renamed genrand_res53() to random_random() and wrapped
      in python calling/return code.

    * genrand_int32() and the helper functions, init_genrand()
      and init_by_array(), were declared static, wrapped in
      Python calling/return code.  also, their global data
      references were replaced with structure references.

    * unused functions from the original were deleted.
      new, original C python code was added to implement the
      Random() interface.

   The following are the verbatim comments from the original code:

   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
    products derived from this software without specific prior written
    permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.keio.ac.jp/matumoto/emt.html
   email: matumoto@math.keio.ac.jp
*/

/* Period parameters -- These are all magic.  Don't change. */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

typedef struct {
    unsigned long state[N];
    int index;
    int has_seed;
} RandomObject;

/* generates a random number on [0,0xffffffff]-interval */
static unsigned long
genrand_int32(RandomObject *self)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */
    unsigned long *mt;

    mt = self->state;
    if (self->index >= N) { /* generate N words at one time */
        int kk;

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        self->index = 0;
    }

    y = mt[self->index++];
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680UL;
    y ^= (y << 15) & 0xefc60000UL;
    y ^= (y >> 18);
    return y;
}

/* initializes mt[N] with a seed */
static void
init_genrand(RandomObject *self, unsigned long s)
{
    int mti;
    unsigned long *mt;

    mt = self->state;
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                */
        /* 2002/01/09 modified by Makoto Matsumoto         */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
    self->index = mti;
    return;
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
static void
init_by_array(RandomObject *self, unsigned long init_key[], unsigned long key_length)
{
    unsigned int i, j, k;   /* was signed in the original code. RDH 12/16/2002 */
    unsigned long *mt;

    mt = self->state;
    init_genrand(self, 19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
             + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
             - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
    return;
}

/*----------------------end of Mersenne Twister Algorithm----------------*/

/*************************************************************************
 * following are tinypy related stuffs.
 *************************************************************************/

static RandomObject _gRandom;       /* random object */

static tp_obj random_seed(TP)
{
    tp_obj arg = TP_DEFAULT(tp_None);

    if (arg.type == TP_NONE) {
        time_t now;

        (void)time(&now);
        init_genrand(&_gRandom, (unsigned long)now);
        _gRandom.has_seed = 1;
    } else if (arg.type == TP_NUMBER) {
        init_genrand(&_gRandom, (unsigned long)arg.number.val);
        _gRandom.has_seed = 1;
    } else if (arg.type == TP_STRING) {
        unsigned long seed;
        
        seed = (unsigned long)tp_hash(tp, arg);
        init_genrand(&_gRandom, seed);
        _gRandom.has_seed = 1;
    } else {
        tp_raise(tp_None,tp_printf(tp, "%s", "invalid argument for seed()"));
    }
    
    return (tp_None);
}

static tp_obj random_getstate(TP)
{
    tp_obj state_list = tp_list(tp);
    int i;

    for (i = 0; i < N; i++) {
        _tp_list_append(tp, state_list.list.val, tp_number(_gRandom.state[i]));
    }
    _tp_list_append(tp, state_list.list.val, tp_number(_gRandom.index));

    return (state_list);
}

/*
 * @state_list  must contain exactly N+1(625) integer.
 */
static tp_obj random_setstate(TP)
{
    tp_obj state_list = TP_OBJ();
    tp_obj state_elem;
    tp_obj len;
    int i;

    len = tp_len(tp, state_list);
    if (len.number.val != N+1) {
        tp_raise(tp_None,tp_printf(tp, "%s: state vector's size invalid(should be %d)", 
                __func__, N+1));
    }

    for (i = 0; i < N; i++) {
        state_elem = tp_get(tp, state_list, tp_number(i));
        _gRandom.state[i] = (unsigned long)state_elem.number.val;
    }
    state_elem = tp_get(tp, state_list, tp_number(i));
    _gRandom.index = (int)state_elem.number.val;

    return (tp_None);
}

/*
 * Jumpahead should be a fast way advance the generator n-steps ahead, but
 * lacking a formula for that, the next best is to use n and the existing
 * state to create a new state far away from the original.
 *
 * The generator uses constant spaced additive feedback, so shuffling the
 * state elements ought to produce a state which would not be encountered
 * (in the near term) by calls to random().  Shuffling is normally
 * implemented by swapping the ith element with another element ranging
 * from 0 to i inclusive.  That allows the element to have the possibility
 * of not being moved.  Since the goal is to produce a new, different
 * state, the swap element is ranged from 0 to i-1 inclusive.  This assures
 * that each element gets moved at least once.
 *
 * To make sure that consecutive calls to jumpahead(n) produce different
 * states (even in the rare case of involutory shuffles), i+1 is added to
 * each element at position i.  Successive calls are then guaranteed to
 * have changing (growing) values as well as shuffled positions.
 *
 * Finally, the self->index value is set to N so that the generator itself
 * kicks in on the next call to random().  This assures that all results
 * have been through the generator and do not just reflect alterations to
 * the underlying state.
 */
static tp_obj random_jumpahead(TP)
{
    long n = (long)TP_NUM();
    long i, j;
    unsigned long *mt;
    unsigned long tmp;

    mt = _gRandom.state;

    for (i = N-1; i > 1; i--) {
        j = n % i;
        if (j == -1L) {
            tp_raise(tp_None,tp_printf(tp, "error: %s: j = %ld", __func__, j));
        }
        tmp   = mt[i];
        mt[i] = mt[j];
        mt[j] = tmp;
    }

    for (i = 0; i < N; i++)
        mt[i] += i + 1;

    _gRandom.index = N;

    return (tp_None);
}

/* random_random is the function named genrand_res53 in the original code;
 * generates a random number on [0,1) with 53-bit resolution; note that
 * 9007199254740992 == 2**53; I assume they're spelling "/2**53" as
 * multiply-by-reciprocal in the (likely vain) hope that the compiler will
 * optimize the division away at compile-time.  67108864 is 2**26.  In
 * effect, a contains 27 random bits shifted left 26, and b fills in the
 * lower 26 bits of the 53-bit numerator.
 * The orginal code credited Isaku Wada for this algorithm, 2002/01/09.
 */
static tp_obj random_random(TP)
{
    RandomObject *self = &_gRandom;
    unsigned long a, b;

    if (! self->has_seed)
        random_seed(tp);

    a = genrand_int32(self)>>5;
    b = genrand_int32(self)>>6;
    
    return tp_number((a*67108864.0+b)*(1.0/9007199254740992.0));
}


#elif defined(__USE_WICHMANN_HILL_RANDOM_GENERATOR)

/************************************************* 
 * divmod(x, y)
 * a helper function borrowed from Python
 *************************************************/
/* Compute Python divmod(x, y), returning the quotient and storing the
 * remainder into *r.  The quotient is the floor of x/y, and that's
 * the real point of this.  C will probably truncate instead (C99
 * requires truncation; C89 left it implementation-defined).
 * Simplification:  we *require* that y > 0 here.  That's appropriate
 * for all the uses made of it.  This simplifies the code and makes
 * the overflow case impossible (divmod(LONG_MIN, -1) is the only
 * overflow case).
 */
#include <assert.h>

static int
divmod(int x, int y, int *r)
{
    int quo;

    assert(y > 0);
    quo = x / y;
    *r = x - quo * y;
    if (*r < 0) {
        --quo;
        *r += y;
    }
    assert(0 <= *r && *r < y);
    return quo;
}


typedef struct WH_RandomObject {
    struct seed {
        unsigned long   x;
        unsigned long   y;
        unsigned long   z;
    } seed;
    int has_seed;
} WH_RandomObject;

static WH_RandomObject _gWhRandom;

static tp_obj random_seed(TP)
{
    long a;
    int x, y, z;
    tp_obj arg = TP_DEFAULT(tp_None);

    if (arg.type == TP_NONE) {
        time_t now;

        (void)time(&now);
        a = (long)now * 256;
    } else if (arg.type == TP_NUMBER) {
        a = (long)arg.number.val;
    } else {
        tp_raise(tp_None,tp_printf(tp, "%s", "invalid argument for seed()"));
    }
    
    a = divmod(a, 30268, &x);
    a = divmod(a, 30306, &y);
    a = divmod(a, 30322, &z);
    _gWhRandom.seed.x = (int)x + 1;
    _gWhRandom.seed.y = (int)y + 1;
    _gWhRandom.seed.z = (int)z + 1;
    _gWhRandom.has_seed = 1;

    return (tp_None);
}

/*
 * following comments are from Python's random.py
 *---------------------------------------
 * Wichman-Hill random number generator.
 *
 * Wichmann, B. A. & Hill, I. D. (1982)
 * Algorithm AS 183:
 * An efficient and portable pseudo-random number generator
 * Applied Statistics 31 (1982) 188-190
 *
 * see also:
 *        Correction to Algorithm AS 183
 *        Applied Statistics 33 (1984) 123
 *
 *        McLeod, A. I. (1985)
 *        A remark on Algorithm AS 183
 *        Applied Statistics 34 (1985),198-200
 * This part is thread-unsafe:
 * BEGIN CRITICAL SECTION
 */
static tp_obj random_random(TP)
{
    long x, y, z;
    double r;

    if (! _gWhRandom.has_seed)
        random_seed(tp);

    x = _gWhRandom.seed.x;
    y = _gWhRandom.seed.y;
    z = _gWhRandom.seed.z;

    x = (171 * x) % 30269;
    y = (172 * y) % 30307;
    z = (170 * z) % 30323;

    _gWhRandom.seed.x = x;
    _gWhRandom.seed.y = y;
    _gWhRandom.seed.z = z;

    /*
     * Note:  on a platform using IEEE-754 double arithmetic, this can
     * never return 0.0 (asserted by Tim; proof too long for a comment).
     */
    errno = 0;
    r = fmod(((double)x/30269.0+(double)y/30307.0+(double)z/30323.0), 1.0);
    if (errno == EDOM)
        tp_raise(tp_None,tp_printf(tp, "%s", "fmod(): denominator can't be zero"));

    return tp_number(r);
}

/*
 * for compatibility
 */
static tp_obj random_setstate(TP)
{
    return (tp_None);
}

/*
 * for compatibility
 */
static tp_obj random_getstate(TP)
{
    return (tp_None);
}

/*
 * FIXME: risk of overflow.
 * following comments are from Python's random.py
 * --------------------------------------------
 * Act as if n calls to random() were made, but quickly.
 *
 * n is an int, greater than or equal to 0.
 *
 * Example use:  If you have 2 threads and know that each will
 * consume no more than a million random numbers, create two Random
 * objects r1 and r2, then do
 *      r2.setstate(r1.getstate())
 *      r2.jumpahead(1000000)
 * Then r1 and r2 will use guaranteed-disjoint segments of the full
 * period.
 */
static tp_obj random_jumpahead(TP)
{
    int n = (int)TP_NUM();
    long x, y, z;

    if (n < 0)
        tp_raise(tp_None,tp_printf(tp, "%s: n = %d invalid, should >= 0", __func__, n));

    x = _gWhRandom.seed.x;
    y = _gWhRandom.seed.y;
    z = _gWhRandom.seed.z;

    x = (int)(x * ((long)pow(171, n) % 30269)) % 30269;
    y = (int)(y * ((long)pow(172, n) % 30307)) % 30307;
    z = (int)(z * ((long)pow(170, n) % 30323)) % 30323;

    _gWhRandom.seed.x = x;
    _gWhRandom.seed.y = y;
    _gWhRandom.seed.z = z;

    return (tp_None);
}


#elif defined(__USE_POSIX_RANDOM_GENERATOR)

/*
 * judge whether seeded
 */
static int has_seed = 0;

static tp_obj random_seed(TP)
{
    tp_obj arg = TP_DEFAULT(tp_None);

    if (arg.type == TP_NONE) {
        time_t now;

        (void)time(&now);
        srandom((unsigned int)now);
        has_seed = 1;
    } else if (arg.type == TP_NUMBER) {
        srandom((unsigned long)arg.number.val);
        has_seed = 1;
    } else {
        tp_raise(tp_None,tp_printf(tp, "%s", "invalid argument for seed()"));
    }
    
    return (tp_None);
}

/*
 * random()
 *
 * generate successive pseudo random number ranging from [0.0, 1.0).
 * usually RAND_MAX is huge number, thus the periods of the success-
 * ive random number is very long, about 16*((2**31)-1).
 * NOTE: if seed() not called before random(), random() will
 * automatically call seed() with current time.
 */
tp_obj random_random(TP)
{
    double r = 0.0;

    if (! has_seed)
        random_seed(tp);

    r = (tp_num)random()/(tp_num)RAND_MAX;

    return (tp_number(r));
}

/*
 * setstate(state)
 *
 * for compatibility.
 */
tp_obj random_setstate(TP)
{
    return (tp_None);
}

/*
 * getstate()
 *
 * for compatibility.
 */
tp_obj random_getstate(TP)
{
    return (tp_None);
}

/*
 * jumpahead()
 *
 * for compatibility.
 */
tp_obj random_jumpahead(TP)
{
    return (tp_None);
}

#else

#error no underlying random generator is specified

#endif

/************************************************************
 * some usual distributions
 ************************************************************/

/*
 * return real number in range [a, b)
 * a and b can be negtive, but a must be less than b.
 */
tp_obj random_uniform(TP)
{
    double a = TP_NUM();
    double b = TP_NUM();
    double r = 0.0;
    tp_obj rvo;         /* random variable object */

    if (a >= b)
        tp_raise(tp_None,tp_printf(tp, "%s: a(%f) must be less than b(%f)", a, b));

    rvo = random_random(tp);
    r = a + (b - a) * rvo.number.val;
    
    return (tp_number(r));
}

/*
 * Normal distribution
 * @mu      mean
 * @sigma   standard deviation
 *-----------------------------
 * Uses Kinderman and Monahan method. Reference: Kinderman,
 * A.J. and Monahan, J.F., "Computer generation of random
 * variables using the ratio of uniform deviates", ACM Trans
 * Math Software, 3, (1977), pp257-260.
 */
tp_obj random_normalvariate(TP)
{
    double mu = TP_NUM();
    double sigma = TP_NUM();
    double NV_MAGICCONST;
    double u1, u2;
    double z, zz;
    double r = 0.0;
    tp_obj rvo;         /* random variable object */

    NV_MAGICCONST = 4.0 * exp(-0.5) / sqrt(2.0);
    while (1) {
        rvo = random_random(tp);
        u1  = rvo.number.val;
        rvo = random_random(tp);
        u2  = 1.0 - rvo.number.val;
        z   = NV_MAGICCONST * (u1 - 0.5) / u2;
        zz  = z * z / 4.0;
        if (zz <= - log(u2))
            break;
    }

    r = mu + z * sigma;

    return (tp_number(r));
}

/*
 * Log normal distribution
 *
 * If take natural logarithm on log normal distribution, normal
 * distribution with mean mu and standard deviation sigma will
 * return.
 * @mu      mean, can be any value
 * @sigma   standard deviation, must be > 0.
 */
tp_obj random_lognormvariate(TP)
{
    double mu = TP_NUM();
    double sigma = TP_NUM();
    tp_obj params;
    tp_obj normvar;     /* normal distribution variate */
    double r = 0.0;

    /*
     * call random_normalvariate() actually
     */
    params = tp_params_v(tp, 2, tp_number(mu), tp_number(sigma));
    normvar = tp_ez_call(tp, "random", "normalvariate", params);
    r = exp(normvar.number.val);

    return (tp_number(r));
}

/*
 * Exponential distribution
 *
 * @lambda      reciprocal of mean.
 * return value range (0, +inf)
 */
tp_obj random_expovariate(TP)
{
    double lambda = TP_NUM();
    double u, r;
    tp_obj rvo;

    do {
        rvo = random_random(tp);
        u = rvo.number.val;
    } while (u <= 0.0000001);

    r = -log(u) / lambda;

    return (tp_number(r));
}

/*
 * Circular data distribution.
 *
 * mu is the mean angle, expressed in radians between 0 and 2*pi, and
 * kappa is the concentration parameter, which must be greater than or
 * equal to zero.  If kappa is equal to zero, this distribution reduces
 * to a uniform random angle over the range 0 to 2*pi.
 *
 * mu:    mean angle (in radians between 0 and 2*pi)
 * kappa: concentration parameter kappa (>= 0)
 * if kappa = 0 generate uniform random angle
 *
 * Based upon an algorithm published in: Fisher, N.I.,
 * "Statistical Analysis of Circular Data", Cambridge
 * University Press, 1993.
 *
 * Thanks to Magnus Kessler for a correction to the
 * implementation of step 4.
 */
tp_obj random_vonmisesvariate(TP)
{
    double mu = TP_NUM();
    double kappa = TP_NUM();
    tp_obj rvo;
    double a, b, c, r;
    double u1, u2, u3, z, f;
    double theta;
    double TWOPI = 2.0 * M_PI;

    if (kappa <= 1e-6) {
        rvo = random_random(tp);
        theta = TWOPI * rvo.number.val;
        return (tp_number(theta));
    }

    a = 1.0 + sqrt(1.0 + 4.0 * kappa * kappa);
    b = (a - sqrt(2.0 * a))/(2.0 * kappa);
    r = (1.0 + b * b)/(2.0 * b);

    while (1) {
        rvo = random_random(tp);
        u1 = rvo.number.val;

        z = cos(M_PI * u1);
        f = (1.0 + r * z)/(r + z);
        c = kappa * (r - f);

        rvo = random_random(tp);
        u2 = rvo.number.val;

        if ((u2 < (c * (2.0 - c))) ||
            (u2 <= (c * exp(1.0 - c))))
            break;
    }

    rvo = random_random(tp);
    u3 = rvo.number.val;
    if (u3 > 0.5)
        theta = fmod(mu, TWOPI) + acos(f);
    else
        theta = fmod(mu, TWOPI) - acos(f);

    return (tp_number(theta));
}

/*
 * Gamma distribution.  Not the gamma function!
 *
 * Conditions on the parameters are alpha > 0 and beta > 0.
 */
tp_obj random_gammavariate(TP)
{
    double alpha = TP_NUM();
    double beta  = TP_NUM();
    tp_obj rvo;
    double res;
    double LOG4 = log(4.0);
    double SG_MAGICCONST = 1.0 + log(4.5);

    /*
     * alpha > 0, beta > 0, mean is alpha*beta, variance is alpha*beta**2
     * Warning: a few older sources define the gamma distribution in terms
     * of alpha > -1.0
     */
    if ((alpha <= 0.0) || (beta <= 0.0))
        tp_raise(tp_None,tp_printf(tp, "%s: alpha(%f) and beta(%f) must be > 0.0",
                __func__, alpha, beta));

    if (alpha > 1.0) {

        /*
         * Uses R.C.H. Cheng, "The generation of Gamma
         * variables with non-integral shape parameters",
         * Applied Statistics, (1977), 26, No. 1, p71-74
         */

        double ainv;
        double bbb, ccc;
        double u1, u2;
        double v, x, z, r;

        ainv = sqrt(2.0 * alpha - 1.0);
        bbb = alpha - LOG4;
        ccc = alpha + ainv;

        while (1) {
            rvo = random_random(tp);
            u1 = rvo.number.val;
            if (! ((1e-7 < u1) && (u1 < 0.9999999)))
                continue;
            rvo = random_random(tp);
            u2 = 1.0 - rvo.number.val;
            v = log(u1 / (1.0 - u1)) / ainv;
            x = alpha * exp(v);
            z = u1 * u1 * u2;
            r = bbb + ccc * v - x;
            if ((r + SG_MAGICCONST - 4.5 * z >= 0.0) ||
                (r >= log(z))) {
                res = x * beta;
                return (tp_number(res));
            }
        }
    }
    else if (alpha == 1.0) {

        /*
         * expovariate(1)
         */

        double u;

        do {
            rvo = random_random(tp);
            u = rvo.number.val;
        } while (u <= 1e-7);

        res = - log(u) * beta;
        return (tp_number(res));
    } else {

        /*
         * alpha is between 0 and 1 (exclusive)
         *
         * Uses ALGORITHM GS of Statistical Computing - Kennedy & Gentle
         */

        double b, p, u, u1, x;

        while (1) {
            rvo = random_random(tp);
            u = rvo.number.val;
            b = (M_E + alpha) / M_E;
            p = b * u;
            if (p <= 1.0)
                /*FIXME: x = p ** (1.0/alpha)*/
                x = pow(p, 1.0/alpha);
            else
                x = - log((b - p) / alpha);
            rvo = random_random(tp);
            u1 = rvo.number.val;
            if (p > 1.0) {
                /*FIXME: if u1 <= x ** (alpha - 1.0):*/
                if (u1 <= pow(x, alpha - 1.0))
                    break;
            }
            else if (u1 <= exp(-x))
                break;
        }

        res = x * beta;
        return (tp_number(res));
    }
}

/*
 * Beta distribution.
 *
 *        Conditions on the parameters are alpha > 0 and beta > 0.
 *        Returned values range between 0 and 1.
 *        
 *        # This version due to Janne Sinkkonen, and matches all the std
 *        # texts (e.g., Knuth Vol 2 Ed 3 pg 134 "the beta distribution").
 *        
 * See also:
 * http://sourceforge.net/bugs/?func=detailbug&bug_id=130030&group_id=5470
 * for Ivan Frohne's insightful analysis of why the original implementation:
 *
 *    def betavariate(self, alpha, beta):
 *        # Discrete Event Simulation in C, pp 87-88.
 *
 *        y = self.expovariate(alpha)
 *        z = self.expovariate(1.0/beta)
 *        return z/(y+z)
 *
 * was dead wrong, and how it probably got that way.
 *
 */
tp_obj random_betavariate(TP)
{
    double alpha = TP_NUM();
    double beta  = TP_NUM();
    double t;
    double r = 0.0;    
    tp_obj y;
    tp_obj params;

    params = tp_params_v(tp, 2, tp_number(alpha), tp_number(1.0));
    y = tp_ez_call(tp, "random", "gammavariate", params);
    if (y.number.val == 0) {
        return (y);
    } else {
        params = tp_params_v(tp, 2, tp_number(beta), tp_number(1.0));
        t = y.number.val;
        y = tp_ez_call(tp, "random", "gammavariate", params);
        r = t / (t + y.number.val);
        return (tp_number(r));
    }
}

/*
 * Pareto distribution.  alpha is the shape parameter.
 * # Jain, pg. 495
 */
tp_obj random_paretovariate(TP)
{
    double alpha = TP_NUM();
    double u;
    double r;
    tp_obj rvo;
    
    rvo = random_random(tp);
    u = 1.0 - rvo.number.val;
    r = 1.0 / pow(u, 1.0/alpha);
    
    return (tp_number(r));
}

/*
 * Weibull distribution.
 *
 * alpha is the scale parameter and beta is the shape parameter.
 *
 * Jain, pg. 499; bug fix courtesy Bill Arms
 */
tp_obj random_weibullvariate(TP)
{
    double alpha = TP_NUM();
    double beta  = TP_NUM();
    double u;
    double r;
    tp_obj rvo;
    
    rvo = random_random(tp);
    u = 1.0 - rvo.number.val;
    r = alpha * pow(-log(u), 1.0/beta);
    return (tp_number(r));
}

/*
 * randomly select an element from range ([start,] stop[, step])
 *
 * 'stop' must be larger than 'start', both can be negative;
 * 'step' must be integer larger than zero.
 */
tp_obj random_randrange(TP)
{
    tp_obj start = TP_OBJ();
    tp_obj stop = TP_DEFAULT(tp_None);
    tp_obj step = TP_DEFAULT(tp_number(1));
    tp_obj rvo = random_random(tp);
    int istart = (int)start.number.val;
    int istep = (int)step.number.val;
    int istop;
    int iwidth;
    double res;
    
    if (stop.type == TP_NONE) {
        /*
                        * if only one argument, then start just means stop
                        */
        istop = istart;
        res = (rvo.number.val * istop);
        return (tp_number(res));
    } else if (stop.type == TP_NUMBER) {
        istop = (int)stop.number.val;
        iwidth = istop - istart;
        if (iwidth < 0)
            tp_raise(tp_None,tp_printf(tp, "%s", "stop must be > start"));
        if (istep <= 0)
            tp_raise(tp_None,tp_printf(tp, "%s", "step must be integer larger than 0"));
            
        if (istep == 1) {
            res = (int)(istart + (int)(rvo.number.val * iwidth));
            return (tp_number(res));
        } else {
            int n = (iwidth + istep - 1) / istep;
            res = (int)(istart + istep * (int)(n * rvo.number.val));
            return (tp_number(res));
        }
    } else {
        tp_raise(tp_None,tp_printf(tp, "%s", "wrong type of stop"));
    }
}

/*
 * return random integer between [a, b]
 */
tp_obj random_randint(TP)
{
    double a = TP_NUM();
    double b = TP_NUM();
    tp_obj r;
    tp_obj params;

    params = tp_params_v(tp, 2, tp_number(a), tp_number(b + 1));
    r = tp_ez_call(tp, "random", "randrange", params);
    return (r);
}

/*
 * return a random element of sequence 'seq'. 'seq' mustn't be empty.
 */
tp_obj random_choice(TP)
{
    tp_obj seq = TP_OBJ();
    tp_obj len;
    tp_obj rvo;
    tp_obj r;
    int i;
    
    len = tp_len(tp, seq);
    if (len.number.val <= 0)
        tp_raise(tp_None,tp_printf(tp, "%s", "seq mustn't be empty"));
    
    rvo = random_random(tp);
    i = (int)(len.number.val * rvo.number.val);
    r = tp_get(tp, seq, tp_number(i));
    
    return (r);
}

/*
 * shuffle sequence 'seq' in place, return None
 */
tp_obj random_shuffle(TP)
{
    tp_obj seq = TP_OBJ();
    tp_obj elmi;
    tp_obj elmj;
    tp_obj params;
    tp_obj rvo;
    tp_obj len = tp_len(tp, seq);
    int i, j;
    
    if (len.number.val <= 0)
        return (tp_None);
    
    for (i = len.number.val - 1; i > len.number.val / 2; i--) {
        /*
                       * randomly exchange elment i and elment j, element i from the behind end of 'seq', while
                       * element j from the front end of 'seq'.
                       */
        params = tp_params_v(tp, 2, tp_number(0), tp_number(len.number.val / 2));
        rvo = tp_ez_call(tp, "random", "randint", params);
        j = (int)rvo.number.val;
        elmi = tp_get(tp, seq, tp_number(i));
        elmj = tp_get(tp, seq, tp_number(j));        
        
        tp_set(tp, seq, tp_number(i), elmj);
        tp_set(tp, seq, tp_number(j), elmi);
    }
    
    for (i = len.number.val / 2; i >= 0; i--) {
        /*
                       * randomly exchange elment i and elment j, element i from the front end of 'seq', while
                       * element j from the behind end of 'seq'.
                       */
        params = tp_params_v(tp, 2, tp_number(len.number.val / 2), tp_number(len.number.val - 1));
        rvo = tp_ez_call(tp, "random", "randint", params);
        j = (int)rvo.number.val;
        elmi = tp_get(tp, seq, tp_number(i));
        elmj = tp_get(tp, seq, tp_number(j));
        
        tp_set(tp, seq, tp_number(i), elmj);
        tp_set(tp, seq, tp_number(j), elmi);
    }
    
    return (tp_None);
}
