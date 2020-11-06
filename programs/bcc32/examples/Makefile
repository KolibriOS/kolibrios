SVN_BCC32:=..
T2FASM:=$(SVN_BCC32)/t2fasm
INCLUDE:=$(SVN_BCC32)/include

# Path to Bin folder
KOS32_BCC:=/home/autobuild/borlandcpp/bin

# Filename
FN:=hello

hello_world:
	wine $(KOS32_BCC)/kos32-bcc -S -v- -R- -6 -a4 -O2 -Og -Oi -Ov -OS -k- -D__KOLIBRI__ -I$(INCLUDE) $(FN).cpp
	echo 'include "kos_make.inc"' > f_$(FN).asm
	$(T2FASM)/t2fasm < $(FN).asm >> f_$(FN).asm
	fasm f_$(FN).asm $(FN).kex
	
	rm f_$(FN).asm
	rm $(FN).asm
