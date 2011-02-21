ifdef windir
ON_WINDOWS = 1
else
ifdef WINDIR
ON_WINDOWS = 1
endif
endif

ifndef ON_WINDOWS
VERSION_OS = linux
NEED_UNDERSCORES = undef
EXESUFFIX =
RM = rm -f
MV = mv
D_ECHO = echo
LIBDIR = $(MENUETDEV)/lib
ASMFMT = elf
else
ifdef HOME
VERSION_OS = cygwin
NEED_UNDERSCORES = define
EXESUFFIX = .exe
RM = rm -f
MV = mv
D_ECHO = echo
LIBDIR = $(MENUETDEV)/lib
ASMFMT = elf
else
VERSION_OS = MinGW
NEED_UNDERSCORES = define
EXESUFFIX = .exe
RM = del
MV = move
D_ECHO = echo.
ON_MINGW = 1
LIBDIR = $(MENUETDEV)\lib
ASMFMT = coff
endif
endif

HAS_DEVENV = 0
GPP_TOOLNAME = g++
STUBFMT = elf

MMKDEP = $(MENUETDEV)/linuxtools/mmkdep
MGCC = $(MENUETDEV)/linuxtools/mgcc
MGPP = $(MENUETDEV)/linuxtools/mgpp
MLD = $(MENUETDEV)/linuxtools/mld
MCHMEM = $(MENUETDEV)/linuxtools/mchmem

GCC32OPT =
AS32OPT =
LD32OPT =
ifneq (,$(findstring 64,$(shell gcc -dumpmachine)))
GCC32OPT = -m32
AS32OPT = --32
LD32OPT = -m$(ASMFMT)_i386
endif
