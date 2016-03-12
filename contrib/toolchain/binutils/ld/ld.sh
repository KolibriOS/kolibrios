#SHS
/kolibrios/as -o deffilep.o deffilep.s
/kolibrios/as -o ei386pe.o ei386pe.s
/kolibrios/as -o ldbuildid.o ldbuildid.s
/kolibrios/as -o ldcref.o ldcref.s
/kolibrios/as -o ldctor.o ldctor.s
/kolibrios/as -o ldemul.o ldemul.s
/kolibrios/as -o ldexp.o ldexp.s
/kolibrios/as -o ldfile.o ldfile.s
/kolibrios/as -o ldgram.o ldgram.s
/kolibrios/as -o ldlang.o ldlang.s
/kolibrios/as -o ldlex-wrapper.o ldlex-wrapper.s
/kolibrios/as -o ldmain.o ldmain.s
/kolibrios/as -o ldmisc.o ldmisc.s
/kolibrios/as -o ldver.o ldver.s
/kolibrios/as -o ldwrite.o ldwrite.s
/kolibrios/as -o lexsup.o lexsup.s
/kolibrios/as -o mri.o mri.s
/kolibrios/as -o pe-dll.o pe-dll.s
/kolibrios/ld @ld.lnk
/kolibrios/objcopy ld-new ld -O binary



