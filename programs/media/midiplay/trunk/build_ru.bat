@erase lang.inc
@echo lang fix ru >lang.inc
@fasm midiplay.asm midiplay
@erase lang.inc
@pause