PCI_CMD_REG  = 04h             ; reg 04, command reg
 IO_ENA      = 0x00000001      ; i/o decode enable
 MEM_ENA     = 0x00000002      ; memory decode enable
 BM_ENA      = 0x00000004      ; bus master enable

pciRegRead8: ; register in CL!
    mov   bl, 4
    mov   bh, [bus]
    mov   ch, [devfn]
    mcall MF_PCI
ret

pciRegRead16:
    mov   bl, 5
    mov   bh, [bus]
    mov   ch, [devfn]
    mcall MF_PCI
ret

pciRegRead32:
    mov   bl, 6
    mov   bh, [bus]
    mov   ch, [devfn]
    mcall MF_PCI
ret

pciRegWrite8: ; value in DL!
    mov   bl, 8
    mov   bh, [bus]
    mov   ch, [devfn]
    mcall MF_PCI
ret

pciRegWrite16:
    mov   bl, 9
    mov   bh, [bus]
    mov   ch, [devfn]
    mcall MF_PCI
ret

pciRegWrite32:
    mov   bl, 10
    mov   bh, [bus]
    mov   ch, [devfn]
    mcall MF_PCI
ret