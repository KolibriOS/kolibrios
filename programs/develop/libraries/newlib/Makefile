
CC = gcc

CFLAGS = -c -O2 -fomit-frame-pointer
LDFLAGS = -nostdlib -shared -s -T libcdll.lds --out-implib libcimp.a --image-base 0 

LIBC_TOPDIR   = .
LIBC_INCLUDES = $(LIBC_TOPDIR)/include

NAME:=	  libc

DEFINES:=  

INCLUDES:=  -I $(LIBC_INCLUDES) 

AMZ_SRCS:=					\
			crt/crt_amz.S		\
			crt/chkstk.S		\
			crt/exit.S		\
			crt/pseudo-reloc.c	\
			crt/setjmp.S
			
STATIC_SRCS:=					\
			crt/start.S		\
			crt/crt1.c		\
			crt/chkstk.S		\
			crt/exit.S		\
			crt/setjmp.S		

DLL_SRCS:=					\
			crt/crtdll.c		\
			crt/chkstk.S		\
			crt/exit.S		\
			crt/setjmp.S		\
			pe/loader.c

CORE_SRCS:=					\
			argz/buf_findstr.c	\
			argz/envz_get.c		\
			crt/emutls.c		\
			crt/thread.S		\
			crt/tls.S		\
			crt/assert.c		\
			crt/cpu_features.c	\
			ctype/ctype_.c 		\
			ctype/isascii.c		\
			ctype/isblank.c		\
			ctype/isalnum.c 	\
			ctype/isalpha.c 	\
			ctype/iscntrl.c 	\
			ctype/isdigit.c 	\
			ctype/islower.c 	\
			ctype/isupper.c 	\
			ctype/isprint.c 	\
			ctype/ispunct.c 	\
			ctype/isspace.c 	\
			ctype/iswctype.c	\
			ctype/iswalnum.c	\
			ctype/iswalpha.c	\
			ctype/iswblank.c	\
			ctype/iswcntrl.c	\
			ctype/iswdigit.c	\
			ctype/iswgraph.c	\
			ctype/iswlower.c	\
			ctype/iswprint.c	\
			ctype/iswpunct.c	\
			ctype/iswspace.c	\
			ctype/iswupper.c	\
			ctype/iswxdigit.c	\
			ctype/isxdigit.c 	\
			ctype/toascii.c		\
			ctype/tolower.c 	\
			ctype/toupper.c		\
			ctype/towctrans.c	\
			ctype/towlower.c	\
			ctype/towupper.c	\
			ctype/wctrans.c		\
			ctype/wctype.c		\
			errno/errno.c		\
			locale/locale.c		\
			locale/lctype.c		\
			reent/impure.c		\
			reent/getreent.c	\
			reent/gettimeofdayr.c	\
			reent/hdlman.c		\
			reent/isattyr.c		\
			reent/openr.c		\
			reent/closer.c		\
			reent/readr.c		\
			reent/lseekr.c		\
			reent/fstatr.c		\
			reent/writer.c		\
			search/qsort.c		\
			search/bsearch.c	\
			signal/signal.c		\
			sys/create.c		\
			sys/delete.c		\
			sys/finfo.c		\
			sys/read.c		\
			sys/write.c		\
			sys/fsize.c		\
			sys/fload.c		\
			time/asctime.c		\
			time/asctime_r.c	\
			time/clock.c		\
			time/ctime.c		\
			time/ctime_r.c		\
			time/difftime.c		\
			time/gettzinfo.c	\
			time/gmtime.c		\
			time/gmtime_r.c		\
			time/mktime.c		\
			time/mktm_r.c		\
			time/lcltime.c		\
			time/lcltime_r.c	\
			time/strftime.c		\
			time/time.c		\
			time/tzlock.c		\
			time/tzvars.c		\
			unpack/unpacker.asm	


STDLIB_SRCS=				\
			__atexit.c	\
			__call_atexit.c	\
			abort.c		\
			abs.c		\
			atof.c		\
			atoi.c		\
			div.c		\
			dtoa.c		\
			dtoastub.c	\
			exit.c		\
			gdtoa-gethex.c	\
			gdtoa-hexnan.c	\
			getenv.c	\
			mprec.c		\
			mbtowc.c	\
			mbtowc_r.c	\
			mbrtowc.c	\
			mlock.c		\
			calloc.c	\
			malloc.c	\
			mallocr.c	\
			rand.c		\
			rand_r.c	\
			rand48.c	\
			realloc.c	\
			seed48.c	\
			srand48.c	\
			strtod.c	\
			strtol.c	\
			strtold.c	\
			strtoll.c	\
			strtoll_r.c	\
			strtoul.c	\
			strtoull.c	\
			strtoull_r.c	\
			system.c	\
			wcrtomb.c	\
			wctomb_r.c	


STRING_SRCS=		memcpy.c	\
			memcmp.c	\
			memmove.c	\
			memset.c	\
			memchr.c	\
			strcat.c	\
			strchr.c	\
			strcmp.c	\
			strcoll.c	\
			strcasecmp.c	\
			strncasecmp.c	\
			strncat.c	\
			strncmp.c	\
			strncpy.c	\
			strndup.c	\
			strndup_r.c	\
			strnlen.c	\
			strcasestr.c	\
			strdup.c	\
			strdup_r.c	\
			strerror.c	\
			strlen.c	\
			strrchr.c	\
			strpbrk.c	\
			strsep.c	\
			strstr.c	\
			strtok.c	\
			strtok_r.c	\
			strupr.c	\
			strcspn.c	\
			strspn.c	\
			strcpy.c	\
			u_strerr.c

STDIO_SRCS=				\
			printf.c	\
			putchar.c	\
			fgets.c		\
			fopen.c		\
			fclose.c	\
			fdopen.c	\
			fflush.c	\
			flags.c		\
			findfp.c	\
			fiprintf.c	\
			fiscanf.c	\
			fprintf.c	\
			fputc.c		\
			fputs.c		\
			fputwc.c	\
			fread.c		\
			freopen.c	\
			fscanf.c	\
			fseek.c		\
			fseeko.c	\
			ftell.c		\
			ftello.c	\
			fwrite.c	\
			fvwrite.c	\
			fwalk.c		\
			putc.c		\
			puts.c		\
			refill.c	\
			rget.c		\
			remove.c	\
			rename.c	\
			setvbuf.c	\
			stdio.c		\
			tmpfile.c	\
			tmpnam.c	\
			ungetc.c	\
			vscanf.c	\
			vsprintf.c	\
			vsnprintf.c	\
			vsscanf.c	\
			makebuf.c	\
			wsetup.c	\
			wbuf.c		\
			sccl.c		\
			snprintf.c	\
			sprintf.c	\
			sscanf.c
	

MATH_SRCS = 	acosf.c acosh.c acoshf.c acoshl.c acosl.c asinf.c asinh.c asinhf.c asinhl.c \
		asinl.c atan2f.c atan2l.c atanf.c atanh.c atanhf.c atanhl.c atanl.c cbrt.c \
		cbrtf.c cbrtl.c coshf.c coshl.c erfl.c expf.c expl.c expm1.c expm1f.c expm1l.c\
		fabs.c fabsf.c fabsl.c fdim.c fdimf.c fdiml.c fmal.c fmax.c fmaxf.c fmaxl.c\
		fmin.c fminf.c fminl.c fmodf.c fmodl.c fp_consts.c fp_constsf.c fp_constsl.c\
		fpclassify.c fpclassifyf.c fpclassifyl.c frexpf.c fucom.c hypotf.c isnan.c \
		isnanf.c isnanl.c ldexp.c ldexpf.c ldexpl.c lgamma.c lgammaf.c lgammal.c \
		llrint.c llrintf.c llrintl.c logb.c logbf.c logbl.c lrint.c lrintf.c lrintl.c\
		lround_generic.c modff.c modfl.c nextafterf.c nextafterl.c nexttoward.c \
		nexttowardf.c pow.c powf.c powi.c powif.c powil.c powl.c rint.c rintf.c \
		rintl.c round_generic.c s_erf.c sf_erf.c signbit.c signbitf.c signbitl.c \
		sinhf.c sinhl.c sqrtf.c sqrtl.c tanhf.c tanhl.c tgamma.c tgammaf.c tgammal.c \
		trunc.c truncf.c truncl.c e_sqrt.c e_sinh.c e_cosh.c e_hypot.c s_tanh.c	\
		s_roundf.c s_fpclassify.c s_isnand.c w_hypot.c s_modf.c e_atan2.c w_atan2.c\
		ceil.S ceilf.S ceill.S copysign.S copysignf.S copysignl.S cos.S cosf.S cosl.S exp.S exp2.S \
		exp2f.S exp2l.S floor.S floorf.S floorl.S fma.S fmaf.S frexp.S frexpl.S ilogb.S ilogbf.S \
		ilogbl.S log10.S log10f.S log10l.S log1p.S log1pf.S log1pl.S log2.S log2f.S log2l.S \
		log.S logf.S logl.S nearbyint.S nearbyintf.S nearbyintl.S remainder.S remainderf.S \
		remainderl.S remquo.S remquof.S remquol.S scalbn.S scalbnf.S scalbnl.S sin.S \
		sinf.S sinl.S tan.S tanf.S tanl.S s_expm1.S


AMZ_OBJS =  $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(AMZ_SRCS)))

STATIC_OBJS =  $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(STATIC_SRCS)))

DLL_OBJS =  $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(DLL_SRCS)))

CORE_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.obj,\
            $(patsubst %.c, %.o, $(CORE_SRCS))))

STDIO_OBJS =  $(patsubst %.c, stdio/%.o,$(STDIO_SRCS))


STRING_OBJS =  $(patsubst %.S, string/%.o, $(patsubst %.asm, string/%.o,\
               $(patsubst %.c, string/%.o, $(STRING_SRCS))))

STDLIB_OBJS =  $(patsubst %.S, stdlib/%.o, $(patsubst %.asm, stdlib/%.o,\
               $(patsubst %.c, stdlib/%.o, $(STDLIB_SRCS))))


MATH_OBJS =  $(patsubst %.S, math/%.o, $(patsubst %.asm, math/%.o,\
               $(patsubst %.c, math/%.o, $(MATH_SRCS))))


PRINTF_OBJS= 	stdio/vfprintf.o	\
		stdio/vfiprintf.o	\
		stdio/svfprintf.o	\
		stdio/svfiprintf.o	\
		stdio/vfscanf.o		\
		stdio/vfiscanf.o	\
		stdio/svscanf.o		\
		stdio/svfiscanf.o

ifeq ($(findstring static,$(MAKECMDGOALS)),static)

LIB_SRCS:=  	$(STATIC_SRCS)
LIB_OBJS:=  	$(STATIC_OBJS)

else

LIB_SRCS:=  	$(DLL_SRCS)
LIB_OBJS:=  	$(DLL_OBJS)

endif

LIB_SRCS+=				\
		$(CORE_SRCS)		\
		$(STDIO_SRCS) 		\
		$(STRING_SRCS)		\
		$(STDLIB_SRCS)

LIB_OBJS+=				\
		$(CORE_OBJS) 		\
		$(STRING_OBJS) 		\
		$(STDLIB_OBJS)		\
		$(STDIO_OBJS) 		\
		$(PRINTF_OBJS) 		\
		$(MATH_OBJS)



shared: $(NAME).dll libamz.a libc.obj


$(NAME).dll: $(LIB_OBJS) $(SRC_DEP) Makefile
	ld $(LDFLAGS) -L. -o $@ $(LIB_OBJS) -lgcc


libamz.a: $(AMZ_OBJS) Makefile
	ar rc libamz.a $(AMZ_OBJS)

libc.obj: $(NAME).dll
	fasm pe/libc.asm ./libc.obj


static: $(NAME).a

$(NAME).a: $(LIB_OBJS) $(SRC_DEP) Makefile
	ar rc $(NAME).a $(LIB_OBJS)



stdio/vfprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DFLOATING_POINT -c stdio/vfprintf.c -o $@

stdio/vfiprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DINTEGER_ONLY -c stdio/vfprintf.c -o $@

stdio/svfprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DSTRING_ONLY -c stdio/vfprintf.c -o $@

stdio/svfiprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DINTEGER_ONLY -DSTRING_ONLY -c stdio/vfprintf.c -o $@


stdio/vfscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES)	stdio/vfscanf.c -o $@

stdio/vfiscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DINTEGER_ONLY $(INCLUDES) stdio/vfscanf.c -o $@

stdio/svscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DSTRING_ONLY $(INCLUDES) stdio/vfscanf.c -o $@


stdio/svfiscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DINTEGER_ONLY -DSTRING_ONLY $(INCLUDES) stdio/vfscanf.c -o $@



%.obj : %.asm Makefile
	fasm $< $@

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<


clean: 
	-rm -f */*.o 


