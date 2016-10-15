
CC = kos32-gcc
FASM = fasm.exe

DRV_DIR   = $(CURDIR)/..

DRV_INCLUDES = $(DRV_DIR)/include

ACPI_INCLUDES = ./acpica/include

INCLUDES =	-I$(DRV_INCLUDES)	\
		-I$(DRV_INCLUDES)/asm 	\
		-I$(DRV_INCLUDES)/uapi
		
INCLUDES+= -I$(ACPI_INCLUDES) -I./

DEFINES	 = -D_LINUX -D__KERNEL__ -DCONFIG_X86 -DCONFIG_X86_32 -DCONFIG_PCI -DCONFIG_ACPI -DBUILDING_ACPICA
DEFINES += -DCONFIG_X86_CMPXCHG64 -DCONFIG_TINY_RCU -DCONFIG_X86_L1_CACHE_SHIFT=6
DEFINES += -DACPI_USE_LOCAL_CACHE -DACPI_NO_ERROR_MESSAGES
DEFINES += -DCONFIG_DMI -DCONFIG_ACPI_REDUCED_HARDWARE_ONLY -DKBUILD_MODNAME=\"acpi.dll\"

CFLAGS_OPT = -Os -march=i686 -fomit-frame-pointer  -fno-ident -mno-stack-arg-probe
CFLAGS_OPT+= -fno-builtin-printf -fno-builtin-snprintf -fno-builtin-sscanf -fno-builtin-vsscanf 
CFLAGS_OPT+= -fno-builtin-scnprintf -fno-builtin-vsprintf -fno-builtin-vsnprintf
CFLAGS_OPT+= -mno-ms-bitfields
CFLAGS = $(INCLUDES) $(DEFINES) $(CFLAGS_OPT)


PE_FLAGS=--major-os-version,0,--minor-os-version,7,--major-subsystem-version,0,--minor-subsystem-version,5,--subsystem,native

LDFLAGS=-e,_drvEntry,-nostdlib,-s,-pie,$(PE_FLAGS),--image-base,0,--file-alignment,512,--section-alignment,512

LIBPATH:= -L$(DRV_DIR)/ddk -L./acpica

LIBS:=    -lcore


NAME= acpi

NAME_SRCS=	acpi.c			\
		blacklist.c		\
		boot.c			\
		bus.c			\
		tables.c		\
		acpica/dsargs.c		\
		acpica/dscontrol.c	\
		acpica/dsdebug.c	\
		acpica/dsfield.c	\
		acpica/dsinit.c		\
		acpica/dsmethod.c	\
		acpica/dsmthdat.c	\
		acpica/dsobject.c	\
		acpica/dsopcode.c	\
		acpica/dsutils.c	\
		acpica/dswexec.c	\
		acpica/dswload.c	\
		acpica/dswload2.c	\
		acpica/dswscope.c	\
		acpica/dswstate.c	\
		acpica/evmisc.c		\
		acpica/evregion.c	\
		acpica/evrgnini.c	\
		acpica/exconfig.c	\
		acpica/exconvrt.c	\
		acpica/excreate.c	\
		acpica/exdebug.c	\
		acpica/exfield.c	\
		acpica/exfldio.c	\
		acpica/exmisc.c		\
		acpica/exmutex.c	\
		acpica/exnames.c	\
		acpica/exoparg1.c	\
		acpica/exoparg2.c	\
		acpica/exoparg3.c	\
		acpica/exoparg6.c	\
		acpica/exprep.c		\
		acpica/exresnte.c	\
		acpica/exresolv.c	\
		acpica/exresop.c	\
		acpica/exstore.c	\
		acpica/exstoren.c	\
		acpica/exstorob.c	\
		acpica/exsystem.c	\
		acpica/exutils.c	\
		acpica/nsaccess.c	\
		acpica/nsalloc.c	\
		acpica/nsarguments.c	\
		acpica/nsconvert.c	\
		acpica/nseval.c		\
		acpica/nsinit.c		\
		acpica/nsload.c		\
		acpica/nsnames.c	\
		acpica/nsobject.c	\
		acpica/nsparse.c	\
		acpica/nspredef.c	\
		acpica/nsprepkg.c	\
		acpica/nsrepair.c	\
		acpica/nsrepair2.c	\
		acpica/nssearch.c	\
		acpica/nsutils.c	\
		acpica/nswalk.c		\
		acpica/psargs.c		\
		acpica/psloop.c		\
		acpica/psobject.c	\
		acpica/psopcode.c	\
		acpica/psopinfo.c	\
		acpica/psparse.c	\
		acpica/psscope.c	\
		acpica/pstree.c		\
		acpica/psutils.c	\
		acpica/pswalk.c		\
		acpica/psxface.c	\
		acpica/tbdata.c		\
		acpica/tbfind.c		\
		acpica/tbfadt.c		\
		acpica/tbinstal.c	\
		acpica/tbprint.c	\
		acpica/tbutils.c	\
		acpica/tbxface.c	\
		acpica/tbxfload.c	\
		acpica/utaddress.c	\
		acpica/utalloc.c	\
		acpica/utbuffer.c	\
		acpica/utcache.c	\
		acpica/utcopy.c		\
		acpica/utdecode.c	\
		acpica/utdelete.c	\
		acpica/uterror.c	\
		acpica/utexcep.c	\
		acpica/utglobal.c	\
		acpica/uthex.c		\
		acpica/utinit.c		\
		acpica/utlock.c		\
		acpica/utmath.c		\
		acpica/utmisc.c		\
		acpica/utmutex.c	\
		acpica/utnonansi.c	\
		acpica/utobject.c	\
		acpica/utosi.c		\
		acpica/utownerid.c	\
		acpica/utpredef.c	\
		acpica/utresrc.c	\
		acpica/utstate.c	\
		acpica/utstring.c	\
		acpica/utxface.c	\
		acpica/utxferror.c	\
		acpica/utxfinit.c	\
		../ddk/debug/dbglog.c	\
		../ddk/io/create.c	\
		../ddk/io/finfo.c	\
		../ddk/io/ssize.c	\
		../ddk/io/write.c	\
		../ddk/malloc/malloc.c	\
		../ddk/linux/ctype.c	\
		../ddk/linux/div64.c	\
		../ddk/linux/dmi.c	\
		../ddk/linux/hexdump.c	\
		../ddk/stdio/vsprintf.c	\
		../ddk/string/strstr.c	\
		../ddk/string/_strncat.S\
		../ddk/string/_strncmp.S\
		../ddk/string/_strncpy.S\
		../ddk/string/_strnlen.S\
		../ddk/string/memcmp.S	\
		../ddk/string/strcat.S	\
		../ddk/string/strcmp.S	\
		../ddk/string/strcpy.S	\
		../ddk/string/strncmp.S	\
		../ddk/string/strncpy.S	\
		../ddk/string/strnlen.S	\
		$(NULL)



all: $(NAME).dll

NAME_OBJS =  $(patsubst %.S, %.o, $(patsubst %.asm, %.o,\
            $(patsubst %.c, %.o, $(NAME_SRCS))))


all: $(NAME).dll

$(NAME).dll: $(NAME_SRCS) acpi.lds Makefile
	$(CC) $(CFLAGS) -nostdlib -fwhole-program -flto -Wl,-Map,acpi.map,-T,acpi.lds,$(LDFLAGS) -Wl,$(LIBPATH) -o $@ $(NAME_SRCS) $(LIBS)
	strip $(NAME).dll
#	kpack $(NAME).dll 

%.o : %.S $(HFILES) Makefile
	as -o $@ $<



