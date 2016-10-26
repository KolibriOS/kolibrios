Compiler                = kos32-gcc
Linker					= kos32-ld

SDK_DIR:= $(abspath ../../../contrib/sdk)

Includes = -Id:\TEMP\mprog\kos\includes -I$(SDK_DIR)/sources/newlib/libc/include

Compatib_Posix = -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp

Compiler_Options = -c -fno-exceptions -D_KOS_ -U_Win32 -U_WIN32 -U__MINGW32__ \
	-mno-ms-bitfields -Wno-write-strings $(Includes) $(Compatib_Posix)

Exe_file                = cmm.kex


Path_Libs		= -Ld:/TEMP/Dev-Cpp/lib/kos -L $(SDK_DIR)/lib
#-Ld:/TEMP/Dev-Cpp/lib/gcc/mingw32/3.4.2 -Ld:/TEMP/Dev-Cpp/lib/gcc -Ld:/TEMP/Dev-Cpp/lib -Ld:/TEMP/Dev-Cpp/lib

Add_Libs		=

#Link_Libs		= -lc -lgcc
Link_Libs = -static -S -nostdlib -T $(SDK_DIR)/lib/app-dynamic.lds \
          --image-base 0 -lgcc -ldll -lc.dll 

My_Libs	 		= main.o port.o toka.o tokb.o tokc.o toke.o tokr.o errors.o debug.o outobj.o outpe.o disasm.o switch.o outle.o pointer.o new_type.o class.o res.o optreg.o libobj.o
 
####################
## Makefile rules ##
####################

all        : $(Exe_file)

$(Exe_file) : $(My_Libs)
	$(Linker) -o cmm.kex --stack 0x100000 $(Path_Libs) $(Add_Libs) $(My_Libs) $(Link_Libs)
	objcopy cmm.kex -O binary
	kpack cmm.kex

clean: 	
	rm *.o


# next are the exceptions that don't have to be compiled with the /zu option
# since they will never be called from a interrupt. hey.. I _tried_ to find
# a clean solution..

main.o :  main.cpp
	$(Compiler) $(Compiler_Options) $<

toka.o :  toka.cpp
	$(Compiler) $(Compiler_Options) $<

tokb.o :  tokb.cpp
	$(Compiler) $(Compiler_Options) $<

tokc.o :  tokc.cpp
	$(Compiler) $(Compiler_Options) $<

toke.o :  toke.cpp
	$(Compiler) $(Compiler_Options) $<

tokr.o :  tokr.cpp
	$(Compiler) $(Compiler_Options) $<

errors.o :errors.cpp
	$(Compiler) $(Compiler_Options) $<

debug.o : debug.cpp
	$(Compiler) $(Compiler_Options) $<

outobj.o : outobj.cpp
	$(Compiler) $(Compiler_Options) $<

outpe.o : outpe.cpp
	$(Compiler) $(Compiler_Options) $<

disasm.o : disasm.cpp
	$(Compiler) $(Compiler_Options) $<

switch.o : switch.cpp
	$(Compiler) $(Compiler_Options) $<

outle.o : outle.cpp
	$(Compiler) $(Compiler_Options) $<

pointer.o : pointer.cpp
	$(Compiler) $(Compiler_Options) $<

new_type.o : new_type.cpp
	$(Compiler) $(Compiler_Options) $<

class.o : class.cpp
	$(Compiler) $(Compiler_Options) $<

res.o : res.cpp
	$(Compiler) $(Compiler_Options) $<

optreg.o : optreg.cpp
	$(Compiler) $(Compiler_Options) $<

libobj.o : libobj.cpp
	$(Compiler) $(Compiler_Options) $<

port.o: port.cpp
	$(Compiler) $(Compiler_Options) $<

.cpp.o:
	$(Compiler) $(Compiler_Options) $<

.asm.o:
	fasm $<

