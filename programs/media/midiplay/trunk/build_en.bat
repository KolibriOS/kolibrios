@erase lang.inc
@echo lang fix en >lang.inc
@fasm midiplay.asm midiplay
@erase lang.inc
@pause