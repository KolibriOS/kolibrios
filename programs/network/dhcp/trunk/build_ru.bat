@erase lang.inc
@echo lang fix ru >lang.inc
@fasm dhcp.asm dhcp
@erase lang.inc
@pause