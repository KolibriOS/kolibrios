@erase lang.inc
@echo lang fix ru >lang.inc
@fasm telnet.asm telnet
@erase lang.inc
@pause