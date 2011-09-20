
CC = gcc

DRV_DIR   = $(CURDIR)/../..

INCLUDES = -I./include -I$(DRV_DIR)/include -I$(DRV_DIR)/include/linux

DEFINES = -DACPI_LIBRARY -D_SINGLE_THREADED -D__KERNEL__ -DCONFIG_X86_32

CFLAGS= -Wall -c -O2 -fomit-frame-pointer -fno-builtin-printf -Wstrict-prototypes

NAME=	ACPICA

ACPICA_SRCS=	\
        	debugger/dbcmds.c \
        	debugger/dbdisply.c \
        	debugger/dbexec.c \
        	debugger/dbfileio.c \
        	debugger/dbhistry.c \
        	debugger/dbinput.c \
        	debugger/dbstats.c \
        	debugger/dbutils.c \
        	debugger/dbxface.c \
        	disassembler/dmbuffer.c \
        	disassembler/dmnames.c \
        	disassembler/dmobject.c \
        	disassembler/dmopcode.c \
        	disassembler/dmresrc.c \
        	disassembler/dmresrcl.c \
        	disassembler/dmresrcs.c \
        	disassembler/dmutils.c \
        	disassembler/dmwalk.c \
        	events/evevent.c \
                events/evglock.c \
        	events/evgpe.c \
        	events/evgpeblk.c \
        	events/evgpeinit.c \
        	events/evgpeutil.c \
        	events/evmisc.c \
        	events/evregion.c \
        	events/evrgnini.c \
        	events/evsci.c \
        	events/evxface.c \
        	events/evxfevnt.c \
        	events/evxfregn.c \
        	hardware/hwacpi.c \
        	hardware/hwgpe.c \
                hardware/hwpci.c \
        	hardware/hwregs.c \
        	hardware/hwsleep.c \
        	hardware/hwvalid.c \
        	hardware/hwxface.c \
                dispatcher/dsargs.c \
                dispatcher/dscontrol.c \
        	dispatcher/dsfield.c \
        	dispatcher/dsinit.c \
        	dispatcher/dsmethod.c \
        	dispatcher/dsmthdat.c \
        	dispatcher/dsobject.c \
        	dispatcher/dsopcode.c \
        	dispatcher/dsutils.c \
        	dispatcher/dswexec.c \
        	dispatcher/dswload.c \
                dispatcher/dswload2.c \
        	dispatcher/dswscope.c \
        	dispatcher/dswstate.c \
        	executer/exconfig.c \
        	executer/exconvrt.c \
        	executer/excreate.c \
        	executer/exdebug.c \
        	executer/exdump.c \
        	executer/exfield.c \
        	executer/exfldio.c \
        	executer/exmisc.c \
        	executer/exmutex.c \
        	executer/exnames.c \
        	executer/exoparg1.c \
        	executer/exoparg2.c \
        	executer/exoparg3.c \
        	executer/exoparg6.c \
        	executer/exprep.c \
        	executer/exregion.c \
        	executer/exresnte.c \
        	executer/exresolv.c \
        	executer/exresop.c \
        	executer/exstore.c \
        	executer/exstoren.c \
        	executer/exstorob.c \
        	executer/exsystem.c \
        	executer/exutils.c \
        	parser/psargs.c \
        	parser/psloop.c \
        	parser/psopcode.c \
        	parser/psparse.c \
        	parser/psscope.c \
        	parser/pstree.c \
        	parser/psutils.c \
        	parser/pswalk.c \
        	parser/psxface.c \
        	namespace/nsaccess.c \
        	namespace/nsalloc.c \
        	namespace/nsdump.c \
        	namespace/nsdumpdv.c \
        	namespace/nseval.c \
        	namespace/nsinit.c \
        	namespace/nsload.c \
        	namespace/nsnames.c \
        	namespace/nsobject.c \
        	namespace/nsparse.c \
        	namespace/nspredef.c \
        	namespace/nsrepair.c \
        	namespace/nsrepair2.c \
        	namespace/nssearch.c \
        	namespace/nsutils.c \
        	namespace/nswalk.c \
        	namespace/nsxfeval.c \
        	namespace/nsxfname.c \
        	namespace/nsxfobj.c \
        	resources/rsaddr.c \
        	resources/rscalc.c \
        	resources/rscreate.c \
        	resources/rsdump.c \
        	resources/rsio.c \
        	resources/rsinfo.c \
        	resources/rsirq.c \
        	resources/rslist.c \
        	resources/rsmemory.c \
        	resources/rsmisc.c \
        	resources/rsutils.c \
        	resources/rsxface.c \
        	tables/tbfadt.c \
        	tables/tbfind.c \
        	tables/tbinstal.c \
        	tables/tbutils.c \
        	tables/tbxface.c \
        	tables/tbxfroot.c \
        	utilities/utclib.c \
        	utilities/utalloc.c \
        	utilities/utcache.c \
        	utilities/utcopy.c \
        	utilities/utdebug.c \
        	utilities/utdelete.c \
        	utilities/uteval.c \
        	utilities/utglobal.c \
        	utilities/utids.c \
        	utilities/utinit.c \
        	utilities/utlock.c \
        	utilities/utmath.c \
        	utilities/utmisc.c \
        	utilities/utmutex.c \
        	utilities/utobject.c \
        	utilities/utresrc.c \
        	utilities/utstate.c \
        	utilities/uttrack.c \
        	utilities/utxface.c \
                utilities/utxferror.c \
                utilities/utdecode.c  \
                utilities/utosi.c  \
        	osunixxf.c

ACPICA_OBJS =  $(patsubst %.c,%.o, $(ACPICA_SRCS))

libacpica.a : $(ACPICA_OBJS) $(ACPICA_SRCS)
	$(AR) cvrs $@ $(ACPICA_OBJS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

clean :
	rm -f $(ACPICA_OBJS)

