
export CC = kos32-gcc
export AR = kos32-ar
export LD = kos32-ld
export STRIP = kos32-strip

INSTALLDIR:= /home/autobuild/tools/win32/mingw32/lib

export SDK_DIR:= $(abspath ../../../../sdk)

cpu_type = i386
enable_shared = no
double_type_size = 64
long_double_type_size = 96
decimal_float = yes
enable_vtable_verify = no
enable_decimal_float = bid
fixed_point = no

# List of extra object files that should be compiled for this target machine.
# The rules for compiling them should be in the t-* file for the machine.
EXTRA_PARTS = crtbegin.o crtend.o crtfastmath.o

CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -fbuilding-libgcc -fno-stack-protector

CFLAGS = -c -O2 -DIN_GCC -DIN_LIBGCC2 -DHAVE_CC_TLS -DUSE_EMUTLS
CFLAGS+= -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -UWINNT -U_MSC_VER
CFLAGS+= -W -Wall -Wno-narrowing -Wwrite-strings -Wcast-qual -Wstrict-prototypes
CFLAGS+= -Wold-style-definition $(CFLAGS_OPT)

DECNUMINC = -Iconfig/libbid -DENABLE_DECIMAL_BID_FORMAT

INCLUDES = -I. -I../gcc -I../include -I$(SDK_DIR)/sources/newlib/libc/include $(DECNUMINC)

gcc_compile = $(CC) $(INCLUDES) $(CFLAGS)

# Defined in libgcc2.c, included only in the static library.
LIB2FUNCS_ST = _eprintf __gcc_bcmp

# List of functions not to build from libgcc2.c.
LIB2FUNCS_EXCLUDE =

# These might cause a divide overflow trap and so are compiled with
# unwinder info.
LIB2_DIVMOD_FUNCS = _divdi3 _moddi3 _udivdi3 _umoddi3 _udiv_w_sdiv _udivmoddi4



objext = .o

FPBIT_FUNCS = _pack_sf _unpack_sf _addsub_sf _mul_sf _div_sf \
	_fpcmp_parts_sf _compare_sf _eq_sf _ne_sf _gt_sf _ge_sf \
	_lt_sf _le_sf _unord_sf _si_to_sf _sf_to_si _negate_sf _make_sf \
	_sf_to_df _sf_to_tf _thenan_sf _sf_to_usi _usi_to_sf

DPBIT_FUNCS = _pack_df _unpack_df _addsub_df _mul_df _div_df \
	_fpcmp_parts_df _compare_df _eq_df _ne_df _gt_df _ge_df \
	_lt_df _le_df _unord_df _si_to_df _df_to_si _negate_df _make_df \
	_df_to_sf _df_to_tf _thenan_df _df_to_usi _usi_to_df

TPBIT_FUNCS = _pack_tf _unpack_tf _addsub_tf _mul_tf _div_tf \
	_fpcmp_parts_tf _compare_tf _eq_tf _ne_tf _gt_tf _ge_tf \
	_lt_tf _le_tf _unord_tf _si_to_tf _tf_to_si _negate_tf _make_tf \
	_tf_to_df _tf_to_sf _thenan_tf _tf_to_usi _usi_to_tf

# Additional sources to handle exceptions; overridden by targets as needed.
LIB2ADDEH = unwind-dw2.c unwind-dw2-fde.c unwind-sjlj.c unwind-c.c
LIB2ADDEHSTATIC = $(LIB2ADDEH)

LIB2ADD = config/i386/gthr-kos32.c	\
	config/i386/kos32-app.c		\
	config/i386/libc-loader.c

EH_MODEL = dw2
CUSTOM_CRTSTUFF = yes


#crtbegin.o: config/i386/cygming-crtbegin.c

#	$(crt_compile) -fno-omit-frame-pointer  -c $<


#crtbeginS.o: config/i386/cygming-crtbegin.c

#	$(crt_compile) -fno-omit-frame-pointer  -c $< -DCRTSTUFFS_O


# We intentionally use a implementation-reserved init priority of 0,

# so allow the warning.

#crtend.o: config/i386/cygming-crtend.c

#	$(crt_compile) -fno-omit-frame-pointer -Wno-error -c $<

# This is an endfile, Use -minline-all-stringops to ensure

# that __builtin_memset doesn't refer to the lib function memset().

crtfastmath.o: config/i386/crtfastmath.c

	$(gcc_compile) -mfxsr -msse -c $<

LIB1ASMSRC = i386/start.S 
LIB1ASMFUNCS = _chkstk _chkstk_ms _start 

DFP_ENABLE = true

LIB2ADD += config/i386/cpuinfo.c
LIB2ADD += config/i386/sfp-exceptions.c


softfp_float_modes := tf
softfp_int_modes := si di ti

softfp_extensions := sftf dftf xftf

softfp_truncations := tfsf tfdf tfxf

softfp_exclude_libgcc2 := n

# Omit TImode functions

softfp_int_modes := si di


# Provide fallbacks for __builtin_copysignq and __builtin_fabsq.

LIB2ADD += config/i386/32/tf-signs.c

# While emutls.c has nothing to do with EH, it is in LIB2ADDEH*
# instead of LIB2ADD because that's the way to be sure on some targets
# (e.g. *-*-darwin*) only one copy of it is linked.
LIB2ADDEH += emutls.c
LIB2ADDEHSTATIC += emutls.c


softfp_float_funcs = add$(m)3 div$(m)3 eq$(m)2 ge$(m)2 le$(m)2 mul$(m)3 \
  neg$(m)2 sub$(m)3 unord$(m)2
softfp_floatint_funcs = fix$(m)$(i) fixuns$(m)$(i) \
  float$(i)$(m) floatun$(i)$(m)

softfp_func_list := \
  $(foreach m,$(softfp_float_modes), \
              $(softfp_float_funcs) \
              $(foreach i,$(softfp_int_modes), \
                          $(softfp_floatint_funcs))) \
  $(foreach e,$(softfp_extensions),extend$(e)2) \
  $(foreach t,$(softfp_truncations),trunc$(t)2) \
  $(softfp_extras)

ifeq ($(softfp_exclude_libgcc2),y)
# This list is taken from mklibgcc.in and doesn't presently allow for
# 64-bit targets where si should become di and di should become ti.
softfp_func_list := $(filter-out floatdidf floatdisf fixunsdfsi fixunssfsi \
  fixunsdfdi fixdfdi fixunssfdi fixsfdi fixxfdi fixunsxfdi \
  floatdixf fixunsxfsi fixtfdi fixunstfdi floatditf \
  floatundidf floatundisf floatundixf floatunditf,$(softfp_func_list))
endif

softfp_file_list := $(addsuffix .c,$(addprefix soft-fp/,$(softfp_func_list)))

# Disable missing prototype and type limit warnings.  The prototypes
# for the functions in the soft-fp files have not been brought across
# from glibc.

soft-fp-objects-base = $(basename $(notdir $(softfp_file_list)))

soft-fp-objects = $(addsuffix $(objext), $(soft-fp-objects-base)) \
  $(addsuffix _s$(objext), $(soft-fp-objects-base))

$(soft-fp-objects) : INTERNAL_CFLAGS += -Wno-missing-prototypes -Wno-type-limits

LIB2ADD += $(softfp_file_list)

ifneq ($(softfp_exclude_libgcc2),y)
# Functions in libgcc2.c are excluded for each soft-float mode (a
# target may have both soft-float and hard-float modes), for the fixed
# list of integer modes (si and di) for which libgcc2.c defines any
# such functions.  Depending on the target, the si and di symbols may
# in fact define di and ti functions.

LIB2FUNCS_EXCLUDE += \
  $(addprefix _,$(foreach m,$(softfp_float_modes), \
                            $(foreach i,si di, \
                                        $(softfp_floatint_funcs))))
endif

iterator = empty.mk $(patsubst %,static-object.mk,$(iter-items))


lib2funcs =	_muldi3 _negdi2 _lshrdi3 _ashldi3 _ashrdi3 _cmpdi2 _ucmpdi2	\
		_clear_cache _trampoline __main _absvsi2 _absvdi2 _addvsi3	\
		_addvdi3 _subvsi3 _subvdi3 _mulvsi3 _mulvdi3 _negvsi2 _negvdi2	\
		_ctors _ffssi2 _ffsdi2 _clz _clzsi2 _clzdi2 _ctzsi2 _ctzdi2	\
		_popcount_tab _popcountsi2 _popcountdi2 _paritysi2 _paritydi2	\
		_powisf2 _powidf2 _powixf2 _powitf2 _mulsc3 _muldc3 _mulxc3	\
		_multc3 _divsc3 _divdc3 _divxc3 _divtc3 _bswapsi2 _bswapdi2	\
		_clrsbsi2 _clrsbdi2

swfloatfuncs = $(patsubst %,_fixuns%XX,sf df xf)

dwfloatfuncs = $(patsubst %,_fix%XX,sf df xf tf) \
		$(patsubst %,_fixuns%XX,sf df xf tf) \
		$(patsubst %,_floatXX%,sf df xf tf) \
		$(patsubst %,_floatunXX%,sf df xf tf)

ifeq ($(LIB2_SIDITI_CONV_FUNCS),)
  lib2funcs += $(subst XX,si,$(swfloatfuncs)) 
  lib2funcs += $(subst XX,di,$(dwfloatfuncs))
endif

# These might cause a divide overflow trap and so are compiled with
# unwinder info.
LIB2_DIVMOD_FUNCS = _divdi3 _moddi3 _udivdi3 _umoddi3 _udiv_w_sdiv _udivmoddi4

# Remove any objects from lib2funcs and LIB2_DIVMOD_FUNCS that are
# defined as optimized assembly code in LIB1ASMFUNCS or as C code
# in LIB2FUNCS_EXCLUDE.
lib2funcs := $(filter-out $(LIB2FUNCS_EXCLUDE) $(LIB1ASMFUNCS),$(lib2funcs))
LIB2_DIVMOD_FUNCS := $(filter-out $(LIB2FUNCS_EXCLUDE) $(LIB1ASMFUNCS), \
		       $(LIB2_DIVMOD_FUNCS))

# Build "libgcc1" (assembly) components.

lib1asmfuncs-o = $(patsubst %,%$(objext),$(LIB1ASMFUNCS))
$(lib1asmfuncs-o): %$(objext): config/$(LIB1ASMSRC)
	$(gcc_compile) -DL$* -xassembler-with-cpp -c $< -o $@
libgcc-objects += $(lib1asmfuncs-o)



# Build lib2funcs.  For the static library also include LIB2FUNCS_ST.
lib2funcs-o = $(patsubst %,%$(objext),$(lib2funcs) $(LIB2FUNCS_ST))
$(lib2funcs-o): %$(objext): libgcc2.c
	$(gcc_compile) -DL$* -c $< -o $@
libgcc-objects += $(lib2funcs-o)

ifneq ($(LIB2_SIDITI_CONV_FUNCS),)
# Build libgcc2.c for each conversion function, with a specific
# L<func> definition and LIBGCC2_UNITS_PER_WORD setting.  The DImode
# functions are built with a wordsize of 4; the TImode functions are
# built with the same labels, but a wordsize of 8.

sifuncs := $(filter-out $(LIB2FUNCS_EXCLUDE),$(subst XX,si,$(swfloatfuncs)))
difuncs := $(filter-out $(LIB2FUNCS_EXCLUDE),$(subst XX,di,$(dwfloatfuncs)))
tifuncs := $(filter-out $(LIB2FUNCS_EXCLUDE),$(subst XX,ti,$(dwfloatfuncs)))

iter-items := $(sifuncs) $(difuncs) $(tifuncs)
iter-labels := $(sifuncs) $(difuncs) $(difuncs)
iter-sizes := $(patsubst %,4,$(sifuncs) $(difuncs)) $(patsubst %,8,$(tifuncs))

include empty.mk $(patsubst %,siditi-object.mk,$(iter-items))

libgcc-objects += $(patsubst %,%$(objext),$(sifuncs) $(difuncs) $(tifuncs))
endif

ifeq ($(LIB2_DIVMOD_EXCEPTION_FLAGS),)
# Provide default flags for compiling divmod functions, if they haven't been
# set already by a target-specific Makefile fragment.
LIB2_DIVMOD_EXCEPTION_FLAGS := -fexceptions -fnon-call-exceptions
endif

# Build LIB2_DIVMOD_FUNCS.
lib2-divmod-o = $(patsubst %,%$(objext),$(LIB2_DIVMOD_FUNCS))
$(lib2-divmod-o): %$(objext): libgcc2.c
	$(gcc_compile) -DL$* -c $< $(LIB2_DIVMOD_EXCEPTION_FLAGS) -o $@
libgcc-objects += $(lib2-divmod-o)

ifeq ($(TPBIT),)
# _sf_to_tf and _df_to_tf require tp-bit.c being compiled in.
FPBIT_FUNCS := $(filter-out _sf_to_tf,$(FPBIT_FUNCS))
DPBIT_FUNCS := $(filter-out _df_to_tf,$(DPBIT_FUNCS))
endif

FPBIT_FUNCS := $(filter-out $(LIB2FUNCS_EXCLUDE),$(FPBIT_FUNCS))
DPBIT_FUNCS := $(filter-out $(LIB2FUNCS_EXCLUDE),$(DPBIT_FUNCS))
TPBIT_FUNCS := $(filter-out $(LIB2FUNCS_EXCLUDE),$(TPBIT_FUNCS))

fpbit-src := fp-bit.c

# Build FPBIT.
ifneq ($(FPBIT),)
fpbit-o = $(patsubst %,%$(objext),$(FPBIT_FUNCS))
$(fpbit-o): %$(objext): $(fpbit-src)
	$(gcc_compile) -DFINE_GRAINED_LIBRARIES -DL$* -DFLOAT $(FPBIT_CFLAGS) -c $< -o $@
libgcc-objects += $(fpbit-o)
endif

# Build DPBIT.
ifneq ($(DPBIT),)
dpbit-o = $(patsubst %,%$(objext),$(DPBIT_FUNCS))
$(dpbit-o): %$(objext): $(fpbit-src)
	$(gcc_compile) -DFINE_GRAINED_LIBRARIES -DL$* $(DPBIT_CFLAGS) -c $< -o $@
libgcc-objects += $(dpbit-o)
endif

# Build TPBIT.
ifneq ($(TPBIT),)
tpbit-o = $(patsubst %,%$(objext),$(TPBIT_FUNCS))
$(tpbit-o): %$(objext): $(fpbit-src)
	$(gcc_compile) -DFINE_GRAINED_LIBRARIES -DL$* -DTFLOAT $(TPBIT_CFLAGS) -c $< -o $@
libgcc-objects += $(tpbit-o)
endif

# Build decimal floating point support.
ifeq ($(decimal_float),yes)

# If $DFP_ENABLE is set, then we want all data type sizes.
ifneq ($(DFP_ENABLE),)
D32PBIT = 1
D64PBIT = 1
D128PBIT = 1
endif

dfp-filenames =
ifneq ($(D32PBIT)$(D64PBIT)$(D128PBIT),)
ifeq ($(enable_decimal_float),bid)
dfp-filenames += bid_decimal_globals bid_decimal_data \
		 bid_binarydecimal bid_convert_data \
		 _isinfd32 _isinfd64 _isinfd128 bid64_noncomp \
		 bid128_noncomp bid128_fma bid_round bid_from_int \
		 bid64_add bid128_add bid64_div bid128_div \
		 bid64_mul bid128_mul bid64_compare bid128_compare \
		 bid128 bid32_to_bid64 bid32_to_bid128 bid64_to_bid128 \
		 bid64_to_int32 bid64_to_int64 \
		 bid64_to_uint32 bid64_to_uint64 \
		 bid128_to_int32 bid128_to_int64 \
		 bid128_to_uint32 bid128_to_uint64
else
dfp-filenames += decContext decNumber decExcept decRound decLibrary decDouble decPacked decQuad decSingle
endif
endif


dfp-objects = $(patsubst %,%$(objext),$(dfp-filenames))
ifeq ($(enable_decimal_float),bid)
$(dfp-objects): %$(objext): config/libbid/%.c
else
$(dfp-objects): %$(objext): ../libdecnumber/%.c
endif
	$(gcc_compile) -c $<
libgcc-objects += $(dfp-objects)

decbits-filenames =
ifneq ($(enable_decimal_float),bid)
ifneq ($(D32PBIT),)
decbits-filenames += decimal32
endif

ifneq ($(D64PBIT),)
decbits-filenames += decimal64
endif

ifneq ($(D128PBIT),)
decbits-filenames += decimal128
endif
endif

decbits-objects = $(patsubst %,%$(objext),$(decbits-filenames))
ifeq ($(enable_decimal_float),bid)
$(decbits-objects): %$(objext): config/libbid/%.c
else
$(decbits-objects): %$(objext): ../libdecnumber/$(enable_decimal_float)/%.c
endif
	$(gcc_compile) -c $<
libgcc-objects += $(decbits-objects)



# Next build individual support functions.
D32PBIT_FUNCS = _addsub_sd _div_sd _mul_sd _plus_sd _minus_sd \
	_eq_sd _ne_sd _lt_sd _gt_sd _le_sd _ge_sd \
	_sd_to_si _sd_to_di _sd_to_usi _sd_to_udi \
	_si_to_sd _di_to_sd _usi_to_sd _udi_to_sd \
	_sd_to_sf _sd_to_df _sd_to_xf _sd_to_tf \
	_sf_to_sd _df_to_sd _xf_to_sd _tf_to_sd \
	_sd_to_dd _sd_to_td _unord_sd _conv_sd

D64PBIT_FUNCS = _addsub_dd _div_dd _mul_dd _plus_dd _minus_dd \
	_eq_dd _ne_dd _lt_dd _gt_dd _le_dd _ge_dd \
	_dd_to_si _dd_to_di _dd_to_usi _dd_to_udi \
	_si_to_dd _di_to_dd _usi_to_dd _udi_to_dd \
	_dd_to_sf _dd_to_df _dd_to_xf _dd_to_tf \
	_sf_to_dd _df_to_dd _xf_to_dd _tf_to_dd \
	_dd_to_sd _dd_to_td _unord_dd _conv_dd

D128PBIT_FUNCS = _addsub_td _div_td _mul_td _plus_td _minus_td \
	_eq_td _ne_td _lt_td _gt_td _le_td _ge_td \
	_td_to_si _td_to_di _td_to_usi _td_to_udi \
	_si_to_td _di_to_td _usi_to_td _udi_to_td \
	_td_to_sf _td_to_df _td_to_xf _td_to_tf \
	_sf_to_td _df_to_td _xf_to_td _tf_to_td \
	_td_to_sd _td_to_dd _unord_td _conv_td

ifeq ($(enable_decimal_float),bid)
ifneq ($(D32PBIT),)
D32PBIT_FUNCS:=$(filter-out _plus_sd _minus_sd _conv_sd, $(D32PBIT_FUNCS))
endif

ifneq ($(D64PBIT),)
D64PBIT_FUNCS:=$(filter-out _plus_dd _minus_dd _conv_dd, $(D64PBIT_FUNCS))
endif

ifneq ($(D128PBIT),)
D128PBIT_FUNCS:=$(filter-out _plus_td _minus_td _conv_td, $(D128PBIT_FUNCS))
endif
endif

ifneq ($(D32PBIT),)
d32pbit-o = $(patsubst %,%$(objext),$(D32PBIT_FUNCS))
ifeq ($(enable_decimal_float),bid)
$(d32pbit-o): %$(objext): config/libbid/%.c
else
$(d32pbit-o): %$(objext): $(srcdir)/dfp-bit.c
endif
	$(gcc_compile) -DFINE_GRAINED_LIBRARIES -DL$* -DWIDTH=32 -c $< -o $@
libgcc-objects += $(d32pbit-o)
endif

ifneq ($(D64PBIT),)
d64pbit-o = $(patsubst %,%$(objext),$(D64PBIT_FUNCS))
ifeq ($(enable_decimal_float),bid)
$(d64pbit-o): %$(objext): config/libbid/%.c
else
$(d64pbit-o): %$(objext): $(srcdir)/dfp-bit.c
endif
	$(gcc_compile) -DFINE_GRAINED_LIBRARIES -DL$* -DWIDTH=64 -c $<
libgcc-objects += $(d64pbit-o)
endif

ifneq ($(D128PBIT),)
d128pbit-o = $(patsubst %,%$(objext),$(D128PBIT_FUNCS))
ifeq ($(enable_decimal_float),bid)
$(d128pbit-o): %$(objext): config/libbid/%.c
else
$(d128pbit-o): %$(objext): $(srcdir)/dfp-bit.c
endif
	$(gcc_compile) -DFINE_GRAINED_LIBRARIES -DL$* -DWIDTH=128 -c $<
libgcc-objects += $(d128pbit-o)
endif

endif

# Build LIB2ADD and LIB2ADD_ST.
ifneq ($(filter-out %.c %.S %.asm,$(LIB2ADD) $(LIB2ADD_ST)),)
$(error Unsupported files in LIB2ADD or LIB2ADD_ST.)
endif

libgcc-objects += $(addsuffix $(objext),$(basename $(notdir $(LIB2ADD))))
libgcc-objects += $(addsuffix $(objext),$(basename $(notdir $(LIB2ADD_ST))))

c_flags :=
iter-items := $(LIB2ADD) $(LIB2ADD_ST)
include $(iterator)

c_flags := -fexceptions

libgcc-objects += $(addsuffix $(objext),$(basename $(notdir $(LIB2ADDEH))))

iter-items := $(LIB2ADDEH)
include $(iterator)


# targets 

all: libgcc.a

libgcc.a : $(libgcc-objects) Makefile
	$(AR) crs libgcc.a $(libgcc-objects)

install: libgcc.a
	mv -f libgcc.a $(INSTALLDIR)/libgcc.a


 