CC = kos32-gcc
AR = kos32-ar
LD = kos32-ld

INSTALLDIR:= /home/autobuild/tools/win32/lib

CFLAGS = -c -O2 -fno-builtin -fno-ident -fomit-frame-pointer -DMISSING_SYSCALL_NAMES
LDFLAGS = -shared -s -T libcdll.lds --out-implib libc.dll.a --image-base 0 
#LDFLAGS+= --output-def libc.orig.def
ARFLAGS = crs

SDK_DIR:= $(abspath ../../..)

LIBC_TOPDIR   = .
LIBC_INCLUDES = $(LIBC_TOPDIR)/include

NAME:=	  libc

DEFINES:= -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -U_MSC_VER
DEFINES+= -D_MB_EXTENDED_CHARSETS_WINDOWS=1
DEFINES+= -D_IEEE_LIBM -DHAVE_RENAME -DBUILD_LIBC -DPACKAGE_NAME=\"newlib\"
DEFINES+= -DPACKAGE_TARNAME=\"newlib\" -DPACKAGE_VERSION=\"2.4.0\"
SEFINES+= -DPACKAGE_STRING=\"newlib\ 2.4.0\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\"

INCLUDES:=  -I $(LIBC_INCLUDES)

STATIC_SRCS:=					\
         		crt/start.S		\
         		crt/crt1.c		\
			crt/crt2.c		\
         		crt/exit.S

LIBDLL_SRCS:=					\
			crt/dllstart.c 		\
         		crt/exit.S           	\
			crt/pseudo-reloc.c	\
			crt/setjmp.S


LIBCDLL_SRCS:=					\
			crt/crtdll.c		\
			crt/crt2.c		\
			crt/pseudo-reloc.c	\
			crt/exit.S		\
			pe/loader.c

CORE_SRCS:=                   			\
			argz/buf_findstr.c	\
			argz/envz_get.c		\
			crt/console.asm		\
			crt/gthr-kos32.c	\
			crt/thread.S         	\
			crt/setjmp.S		\
			crt/cpu_features.c	\
			crt/tls.c		\
			ctype/ctype_.c 		\
			ctype/isascii.c		\
			ctype/isblank.c		\
			ctype/isalnum.c      	\
			ctype/isalpha.c      	\
			ctype/iscntrl.c      	\
			ctype/isdigit.c      	\
			ctype/islower.c      	\
			ctype/isupper.c      	\
			ctype/isprint.c      	\
			ctype/ispunct.c      	\
			ctype/isspace.c      	\
			ctype/iswctype.c     	\
			ctype/iswalnum.c     	\
			ctype/iswalpha.c     	\
			ctype/iswblank.c     	\
			ctype/iswcntrl.c     	\
    		ctype/iswdigit.c     	\
    		ctype/iswgraph.c     	\
    		ctype/iswlower.c     	\
    		ctype/iswprint.c     	\
    		ctype/iswpunct.c     	\
    		ctype/iswspace.c     	\
    		ctype/iswupper.c     	\
    		ctype/iswxdigit.c    	\
    		ctype/isxdigit.c     	\
			ctype/jp2uc.c		\
			ctype/toascii.c		\
			ctype/tolower.c      	\
			ctype/toupper.c		\
			ctype/towctrans.c    	\
			ctype/towlower.c     	\
			ctype/towupper.c     	\
			ctype/wctrans.c		\
			ctype/wctype.c       	\
			errno/errno.c        	\
			locale/locale.c		\
			locale/lctype.c		\
			locale/ldpart.c		\
			reent/closer.c       	\
			reent/fstatr.c       	\
			reent/getreent.c     	\
			reent/gettimeofdayr.c	\
			reent/impure.c       	\
			reent/init_reent.c	\
			reent/isattyr.c		\
			reent/linkr.c		\
			reent/lseekr.c       	\
			reent/mutex.c		\
			reent/openr.c        	\
			reent/readr.c        	\
			reent/renamer.c		\
			reent/statr.c		\
			reent/timesr.c		\
			reent/unlinkr.c		\
			reent/writer.c       	\
			search/qsort.c       	\
			search/bsearch.c     	\
			signal/signal.c		\
			sys/access.c		\
			sys/clock_gettime.c	\
			sys/close.c		\
			sys/conio.c		\
			sys/create.c         	\
			sys/errno.c		\
			sys/finfo.c          	\
			sys/fsize.c          	\
			sys/fstat.c		\
			sys/gettod.c		\
			sys/io.c		\
			sys/ioread.c		\
			sys/iowrite.c		\
			sys/isatty.c		\
			sys/lseek.c		\
			sys/open.c		\
			sys/read.c           	\
			sys/_rename.c		\
			sys/stat.c		\
			sys/unlink.c		\
			sys/write.c          	\
			sys/io_alloc.S		\
			time/asctime.c       	\
			time/asctime_r.c     	\
			time/clock.c         	\
			time/ctime.c         	\
			time/ctime_r.c       	\
			time/difftime.c		\
			time/gettzinfo.c     	\
			time/gmtime.c        	\
			time/gmtime_r.c		\
			time/mktime.c        	\
			time/month_lengths.c	\
			time/lcltime.c       	\
			time/lcltime_r.c     	\
			time/strftime.c		\
			time/time.c          	\
			time/tzcalc_limits.c	\
			time/timelocal.c	\
			time/tzlock.c        	\
			time/tzset.c		\
			time/tzset_r.c		\
			time/tzvars.c


STDLIB_SRCS=                 			\
    		__atexit.c          	\
    		__call_atexit.c     	\
    		abort.c              	\
    		abs.c               	\
			assert.c		\
			atexit.c		\
    		atof.c              	\
    		atoi.c              	\
			atol.c			\
			btowc.c			\
    		calloc.c             	\
			cxa_atexit.c		\
			cxa_finalize.c		\
    		div.c               	\
    		dtoa.c               	\
    		dtoastub.c          	\
			efgcvt.c		\
			ecvtbuf.c		\
			eprintf.c		\
			erand48.c		\
    		exit.c              	\
    		gdtoa-gethex.c      	\
    		gdtoa-hexnan.c       	\
    		getenv.c             	\
			getenv_r.c		\
			itoa.c			\
			ldiv.c			\
			labs.c			\
			ldtoa.c			\
         		malloc.c             	\
        		mallocr.c           	\
			mblen.c			\
			mblen_r.c		\
			mbrlen.c		\
         		mbrtowc.c           	\
			mbsinit.c		\
			mbsnrtowcs.c		\
			mbsrtowcs.c		\
			mbstowcs.c		\
			mbstowcs_r.c		\
			mbtowc.c		\
			mbtowc_r.c		\
         		mlock.c              	\
         		mprec.c              	\
        		rand.c               	\
         		rand_r.c            	\
         		rand48.c            	\
			random.c		\
         		realloc.c            	\
			sb_charsets.c		\
         		seed48.c             	\
         		srand48.c            	\
         		strtod.c            	\
			strtodg.c		\
         		strtol.c             	\
         		strtold.c            	\
         		strtoll.c            	\
         		strtoll_r.c         	\
			strtorx.c		\
         		strtoul.c            	\
         		strtoull.c           	\
         		strtoull_r.c        	\
         		system.c            	\
			utoa.c			\
         		wcrtomb.c           	\
			wcsnrtombs.c		\
			wcsrtombs.c		\
			wcstod.c		\
			wcstol.c		\
			wcstold.c		\
			wcstoll.c		\
			wcstoll_r.c		\
			wcstombs.c		\
			wcstombs_r.c		\
			wcstoul.c		\
			wcstoull.c		\
			wcstoull_r.c		\
			wctob.c			\
			wctomb.c		\
			wctomb_r.c


STRING_SRCS= \
			bcmp.c			\
			bcopy.c			\
			bzero.c			\
			explicit_bzero.c	\
			gnu_basename.c		\
			index.c			\
			memccpy.c		\
			memchr.c		\
			memcmp.c		\
			memcpy.c		\
			memmem.c		\
			memmove.c		\
			mempcpy.c		\
			memrchr.c		\
			memset.c		\
			rawmemchr.c		\
			rindex.c		\
			stpcpy.c		\
			stpncpy.c		\
			strcasecmp.c		\
			strcasestr.c		\
			strcat.c		\
			strchr.c		\
			strchrnul.c		\
			strcmp.c		\
			strcoll.c		\
			strcpy.c		\
			strcspn.c		\
			strdup.c		\
			strdup_r.c		\
			strerror.c		\
			strerror_r.c		\
			strlcat.c		\
			strlcpy.c		\
			strlen.c		\
			strlwr.c		\
			strncasecmp.c		\
			strncat.c		\
			strncmp.c		\
			strncpy.c		\
			strndup.c		\
			strndup_r.c		\
			strnlen.c		\
			strpbrk.c		\
			strrchr.c		\
			strsep.c		\
			strspn.c		\
			strstr.c		\
			strtok.c		\
			strtok_r.c		\
			strupr.c		\
			strxfrm.c		\
			swab.c			\
			u_strerr.c		\
			wcpcpy.c		\
			wcpncpy.c		\
			wcscasecmp.c		\
			wcscat.c		\
			wcschr.c		\
			wcscmp.c		\
			wcscoll.c		\
			wcscpy.c		\
			wcscspn.c		\
			wcsdup.c		\
			wcslcat.c		\
			wcslcpy.c		\
			wcslen.c		\
			wcsncasecmp.c		\
			wcsncat.c		\
			wcsncmp.c		\
			wcsncpy.c		\
			wcsnlen.c		\
			wcspbrk.c		\
			wcsrchr.c		\
			wcsspn.c		\
			wcsstr.c		\
			wcstok.c		\
			wcswidth.c		\
			wcsxfrm.c		\
			wcwidth.c		\
			wmemchr.c		\
			wmemcmp.c		\
			wmemcpy.c		\
			wmemmove.c		\
			wmemset.c


STDIO_SRCS=					\
			asiprintf.c		\
			asniprintf.c		\
			asnprintf.c		\
			asprintf.c		\
			clearerr.c		\
			clearerr_u.c		\
			diprintf.c		\
			dprintf.c		\
			fclose.c		\
			fcloseall.c		\
			fdopen.c		\
			feof.c			\
			feof_u.c		\
			ferror.c		\
			ferror_u.c		\
			fflush.c		\
			fflush_u.c		\
			fgetc.c			\
			fgetc_u.c		\
			fgetpos.c		\
			fgets.c			\
			fgets_u.c		\
			fgetwc.c		\
			fgetwc_u.c		\
			fgetws.c		\
			fgetws_u.c		\
			fileno.c		\
			fileno_u.c		\
			findfp.c		\
			fiprintf.c		\
			fiscanf.c		\
			flags.c			\
			fmemopen.c		\
			fopen.c			\
			fopencookie.c		\
			fprintf.c		\
			fpurge.c		\
			fputc.c			\
			fputc_u.c		\
			fputs.c			\
			fputs_u.c		\
			fputwc.c		\
			fputwc_u.c		\
			fputws.c		\
			fputws_u.c		\
			fsetpos.c		\
			funopen.c		\
			fread.c			\
			fread_u.c		\
			freopen.c		\
			fscanf.c		\
			fseek.c			\
			fseeko.c		\
			fsetlocking.c		\
			ftell.c			\
			ftello.c		\
			fvwrite.c		\
			fwalk.c			\
			fwide.c			\
			fwprintf.c		\
			fwrite.c		\
			fwrite_u.c		\
			fwscanf.c		\
			getc.c			\
			getc_u.c		\
			getchar.c		\
			getchar_u.c		\
			getdelim.c		\
			getline.c		\
			gets.c			\
			getw.c			\
			getwc.c			\
			getwc_u.c		\
			getwchar.c		\
			getwchar_u.c		\
			iprintf.c		\
			iscanf.c		\
			makebuf.c		\
			mktemp.c		\
			open_memstream.c	\
			perror.c		\
			printf.c		\
			putc.c			\
			putc_u.c		\
			putchar.c		\
			putchar_u.c		\
			puts.c			\
			putw.c			\
			putwc.c			\
			putwc_u.c		\
			putwchar.c		\
			putwchar_u.c		\
			refill.c		\
			remove.c		\
			rename.c		\
			rewind.c		\
			rget.c			\
			scanf.c			\
			sccl.c			\
			setbuf.c		\
			setbuffer.c		\
			setlinebuf.c		\
			setvbuf.c		\
			siprintf.c		\
			siscanf.c		\
			sniprintf.c		\
			snprintf.c		\
			sprintf.c		\
			sscanf.c		\
			stdio.c			\
			stdio_ext.c		\
			swprintf.c		\
			swscanf.c		\
			tmpfile.c		\
			tmpnam.c		\
			ungetc.c		\
			ungetwc.c		\
			vasiprintf.c		\
			vasniprintf.c		\
			vasnprintf.c		\
			vasprintf.c		\
			vdiprintf.c		\
			vdprintf.c		\
			vfwscanf.c		\
			viprintf.c		\
			viscanf.c		\
			vprintf.c		\
			vscanf.c		\
			vsiprintf.c		\
			vsiscanf.c		\
			vsprintf.c		\
			vsniprintf.c		\
			vsnprintf.c		\
			vsscanf.c		\
			vswprintf.c		\
			vswscanf.c		\
			vwprintf.c		\
			vwscanf.c		\
			wbuf.c			\
			wprintf.c		\
			wscanf.c		\
			wsetup.c


MATH_SRCS = 	e_acos.c e_acosh.c e_asin.c e_atan2.c e_atanh.c e_cosh.c e_exp.c e_fmod.c	\
		e_hypot.c e_j0.c e_j1.c e_jn.c e_log.c e_log10.c e_pow.c e_rem_pio2.c 		\
		e_remainder.c e_scalb.c e_sinh.c e_sqrt.c ef_acos.c ef_acosh.c ef_asin.c	\
		ef_atan2.c ef_atanh.c ef_cosh.c ef_exp.c ef_fmod.c ef_hypot.c ef_j0.c ef_j1.c	\
		ef_jn.c ef_log.c ef_log10.c ef_pow.c ef_rem_pio2.c ef_remainder.c ef_scalb.c	\
		ef_sinh.c ef_sqrt.c er_gamma.c er_lgamma.c erf_gamma.c erf_lgamma.c f_exp.c	\
		f_expf.c f_llrint.c f_llrintf.c f_llrintl.c f_lrint.c f_lrintf.c f_lrintl.c	\
		f_pow.c f_powf.c f_rint.c f_rintf.c f_rintl.c feclearexcept.c fetestexcept.c	\
		k_cos.c	k_rem_pio2.c k_sin.c k_standard.c k_tan.c kf_cos.c kf_rem_pio2.c	\
		kf_sin.c kf_tan.c s_asinh.c s_atan.c s_cbrt.c s_ceil.c s_copysign.c s_cos.c	\
		s_erf.c	s_exp10.c s_expm1.c s_fabs.c s_fdim.c s_finite.c s_floor.c s_fma.c	\
		s_fmax.c s_fmin.c s_fpclassify.c						\
		s_frexp.c s_ilogb.c s_infconst.c s_infinity.c s_isinf.c s_isinfd.c s_isnan.c	\
		s_isnand.c s_ldexp.c s_lib_ver.c s_llrint.c s_llround.c s_log1p.c s_log2.c	\
		s_logb.c s_lrint.c s_lround.c s_matherr.c s_modf.c s_nan.c s_nearbyint.c	\
		s_nextafter.c s_pow10.c s_remquo.c s_rint.c s_round.c s_scalbln.c s_scalbn.c	\
		s_signbit.c s_signif.c s_sin.c s_tan.c s_tanh.c s_trunc.c scalblnl.c scalbnl.c	\
		sf_asinh.c sf_atan.c sf_cbrt.c sf_ceil.c sf_copysign.c sf_cos.c sf_erf.c 	\
		sf_exp10.c sf_expm1.c sf_fabs.c sf_fdim.c sf_finite.c sf_floor.c sf_fma.c	\
		sf_fmax.c sf_fmin.c sf_fpclassify.c sf_frexp.c sf_ilogb.c sf_infinity.c		\
		sf_isinf.c sf_isinff.c sf_isnan.c sf_isnanf.c sf_ldexp.c sf_llrint.c		\
		sf_llround.c sf_log1p.c sf_log2.c sf_logb.c sf_lrint.c sf_lround.c sf_modf.c	\
		sf_nan.c sf_nearbyint.c sf_nextafter.c sf_pow10.c sf_remquo.c sf_rint.c		\
		sf_round.c sf_scalbln.c sf_scalbn.c sf_signif.c sf_sin.c sf_tan.c sf_tanh.c	\
		sf_trunc.c w_acos.c w_acosh.c w_asin.c w_atan2.c w_atanh.c w_cosh.c w_drem.c	\
		w_exp.c w_exp2.c w_fmod.c w_gamma.c w_hypot.c w_j0.c w_j1.c w_jn.c w_lgamma.c	\
		w_log.c w_log10.c w_pow.c w_remainder.c w_scalb.c w_sincos.c w_sinh.c w_sqrt.c	\
		w_tgamma.c wf_acos.c wf_acosh.c wf_asin.c wf_atan2.c wf_atanh.c wf_cosh.c	\
		wf_drem.c wf_exp.c wf_exp2.c wf_fmod.c wf_gamma.c wf_hypot.c wf_j0.c wf_j1.c	\
		wf_jn.c wf_lgamma.c wf_log.c wf_log10.c wf_pow.c wf_remainder.c wf_scalb.c	\
		wf_sincos.c wf_sinh.c wf_sqrt.c wf_tgamma.c wr_gamma.c wr_lgamma.c wrf_gamma.c	\
		wrf_lgamma.c	\
		f_atan2.S f_atan2f.S f_frexp.S f_frexpf.S f_ldexp.S f_ldexpf.S f_log.S		\
		f_log10.S f_log10f.S f_logf.S f_tan.S f_tanf.S


STATIC_OBJS =  $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(STATIC_SRCS)))

LIBCRT_OBJS =  $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(LIBCRT_SRCS)))

LIBDLL_OBJS =  $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(LIBDLL_SRCS)))

LIBCDLL_OBJS =  $(patsubst %.S, %.o, $(patsubst %.c, %.o, $(LIBCDLL_SRCS)))

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
		stdio/svfiwprintf.o 	\
		stdio/svfwprintf.o	\
		stdio/vfiwprintf.o	\
		stdio/vfwprintf.o	\
		stdio/vfscanf.o		\
		stdio/vfiscanf.o	\
		stdio/svscanf.o		\
		stdio/svfiscanf.o	\
		stdio/svfiwscanf.o	\
		stdio/svfwscanf.o

ifeq ($(findstring static,$(MAKECMDGOALS)),static)

LIB_SRCS:=  	$(STATIC_SRCS)
LIB_OBJS:=  	$(STATIC_OBJS)

else

LIB_SRCS:=  	$(LIBCDLL_SRCS)
LIB_OBJS:=  	$(LIBCDLL_OBJS)

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
		$(PRINTF_OBJS)		\
		$(MATH_OBJS)

LIB_OBJS+= time/wcsftime.o

shared: $(NAME).dll libdll.a


$(NAME).dll: $(LIB_OBJS) $(SRC_DEP) Makefile
	$(LD) $(LDFLAGS) $(LIBPATH) -o $@ $(LIB_OBJS) -lgcc --version-script libc.ver
#sed -e "s/ @[^ ]*//" libc.orig.def > libc.def
#sed -f cmd2.sed libc.def > mem
#sed -f newlib.sed mem > libc.inc

install: libc.dll libc.dll.a libdll.a
	mv -f libc.dll $(SDK_DIR)/bin
	mv -f libc.dll.a $(INSTALLDIR)
	mv -f libdll.a $(INSTALLDIR)

libdll.a: $(LIBDLL_OBJS) Makefile
	$(AR) $(ARFLAGS) libdll.a $(LIBDLL_OBJS)

static: $(NAME).a

$(NAME).a: $(LIB_OBJS) $(SRC_DEP) Makefile
	$(AR) $(ARFLAGS) $(NAME).a $(LIB_OBJS)
	mv -f libc.a $(INSTALLDIR)

crt/crt1.o: crt/crt1.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fno-delete-null-pointer-checks crt/crt1.c -o $@

crt/crtdll.o: crt/crtdll.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fno-delete-null-pointer-checks crt/crtdll.c -o $@

pe/loader.o: pe/loader.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fno-delete-null-pointer-checks pe/loader.c -o $@

reent/renamer.o: reent/renamer.c
	$(CC) $(CFLAGS) $(DEFINES) -D_COMPILING_NEWLIB $(INCLUDES) reent/renamer.c -o $@

stdio/svfiprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DINTEGER_ONLY -DSTRING_ONLY stdio/vfprintf.c -o $@

stdio/svfprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DSTRING_ONLY stdio/vfprintf.c -o $@

stdio/vfiprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DINTEGER_ONLY stdio/vfprintf.c -o $@

stdio/vfprintf.o: stdio/vfprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums stdio/vfprintf.c -o $@

stdio/svfiwprintf.o: stdio/vfwprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DINTEGER_ONLY -DSTRING_ONLY stdio/vfwprintf.c -o $@

stdio/svfwprintf.o: stdio/vfwprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DSTRING_ONLY stdio/vfwprintf.c -o $@

stdio/vfiwprintf.o: stdio/vfwprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums -DINTEGER_ONLY stdio/vfwprintf.c -o $@

stdio/vfwprintf.o: stdio/vfwprintf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -fshort-enums stdio/vfwprintf.c -o $@
 
stdio/svfiscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DINTEGER_ONLY -DSTRING_ONLY $(INCLUDES) stdio/vfscanf.c -o $@

stdio/svscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DSTRING_ONLY $(INCLUDES) stdio/vfscanf.c -o $@

stdio/vfscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES)	stdio/vfscanf.c -o $@

stdio/vfiscanf.o: stdio/vfscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DINTEGER_ONLY $(INCLUDES) stdio/vfscanf.c -o $@

stdio/svfiwscanf.o: stdio/vfwscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DINTEGER_ONLY -DSTRING_ONLY $(INCLUDES) stdio/vfwscanf.c -o $@
 
stdio/svfwscanf.o: stdio/vfwscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DSTRING_ONLY $(INCLUDES) stdio/vfwscanf.c -o $@
 
stdio/vfiwscanf.o: stdio/vfwscanf.c
	$(CC) $(CFLAGS) $(DEFINES) -DINTEGER_ONLY $(INCLUDES) stdio/vfwscanf.c -o $@

time/wcsftime.o: time/strftime.c
	$(CC) $(CFLAGS) $(DEFINES) -DMAKE_WCSFTIME $(INCLUDES) time/strftime.c -o $@


%.obj : %.asm Makefile
	fasm $< $

%.o : %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

%.o : %.S Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<


clean:
	-rm -f */*.o




