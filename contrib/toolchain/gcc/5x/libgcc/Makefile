
export CC = kos32-gcc
export AR = kos32-ar
export LD = kos32-ld
export STRIP = kos32-strip

export SDK_DIR:= $(abspath ../../../../sdk)

CFLAGS_OPT+= -fomit-frame-pointer -fno-ident -mno-ms-bitfields
CFLAGS_OPT+= -fbuilding-libgcc -fno-stack-protector

CFLAGS = -c -O2 -DIN_GCC -DIN_LIBGCC2 -DHAVE_CC_TLS -DUSE_EMUTLS -DENABLE_DECIMAL_BID_FORMAT
CFLAGS+= -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -U_MSC_VER
CFLAGS+= -W -Wall -Wno-narrowing -Wwrite-strings -Wcast-qual -Wstrict-prototypes
CFLAGS+= -Wmissing-prototypes -Wold-style-definition $(CFLAGS_OPT)

INCLUDES = -I../gcc -I../include

objext = .o

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

# targets 

all: libgcc.a

lib2funcs-o = $(patsubst %,%$(objext),$(lib2funcs) $(LIB2FUNCS_ST))


$(lib2funcs-o): %$(objext): libgcc2.c
	$(CC) $(INCLUDES) $(CFLAGS) -DL$* -c $< -o $@

libgcc.a : $(lib2funcs-o) MAkefile
	$(AR) crs libgcc.a $(lib2funcs-o)
#	mv -f libbfd.a $(SDK_DIR)/lib


 