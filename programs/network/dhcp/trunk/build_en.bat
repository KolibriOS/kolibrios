@erase lang.inc
@echo lang fix en >lang.inc
@fasm dhcp.asm dhcp
@erase lang.inc
@pause