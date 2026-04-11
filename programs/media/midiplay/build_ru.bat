@erase lang.inc
@echo lang fix ru_RU >lang.inc
@fasm midiplay.asm midiplay
@erase lang.inc
@pause
