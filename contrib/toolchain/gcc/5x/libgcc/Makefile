
export CC = kos32-gcc
export AR = kos32-ar
export LD = kos32-ld
export STRIP = kos32-strip

export SDK_DIR:= $(abspath ../../../../sdk)

double_type_size = 64
long_double_type_size = 96
decimal_float = yes
enable_decimal_float = bid
fixed_point = no

FPBIT = true

DPBIT = true
D32PBIT = 1
D64PBIT = 1
D128PBIT = 1

# List of functions not to build from libgcc2.c.

LIB2FUNCS_EXCLUDE =


CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -fbuilding-libgcc -fno-stack-protector

CFLAGS = -c -O2 -DIN_GCC -DIN_LIBGCC2 -DHAVE_CC_TLS -DUSE_EMUTLS -DENABLE_DECIMAL_BID_FORMAT
CFLAGS+= -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER
CFLAGS+= -W -Wall -Wno-narrowing -Wwrite-strings -Wcast-qual -Wstrict-prototypes
CFLAGS+= -Wmissing-prototypes -Wold-style-definition $(CFLAGS_OPT)

INCLUDES = -I. -I../gcc -I../include

gcc_compile = $(CC) $(INCLUDES) $(CFLAGS)

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

lib2funcs += $(subst XX,si,$(swfloatfuncs)) 
lib2funcs += $(subst XX,di,$(dwfloatfuncs))

lib2funcs-o = $(patsubst %,%$(objext),$(lib2funcs) $(LIB2FUNCS_ST))

$(lib2funcs-o): %$(objext): libgcc2.c
	$(gcc_compile) -DL$* -c $< -o $@

libgcc-objects =

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

# targets 

all: libgcc.a

libgcc.a : $(lib2funcs-o) $(libgcc-objects) Makefile
	$(AR) crs libgcc.a $(lib2funcs-o) $(libgcc-objects)
#	mv -f libbfd.a $(SDK_DIR)/lib


 