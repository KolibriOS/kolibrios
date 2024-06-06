@erase lang.inc
@echo lang fix en_US >lang.inc
@fasm midiplay.asm midiplay
@erase lang.inc
@pause
