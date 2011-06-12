include 'cfg_bios.inc'
use16
;org     0

rom_header:
;       PnP Option ROM header
rom_signature	dw      0xAA55	                ; +0    : magic
rom_length      db      BIOS_BOOT_BLOCK_SIZE    ; +2    : number of 512byte blocks
rom_entry:
                jmp     init_entry              ; +3    : initialization entry point
                db      'AZ'
rom_reserved    rb      0x11                    ; +7    : reserved (17 bytes)
rom_pci_struc   dw      pci_header              ; +18h  : offset to PCI data structure
rom_expansion   dw      pnp_header              ; +1Ah  : offset to expansion header structure

align 16
pnp_header:
;       PnP Expansion Header
pnp_signature   db      '$PnP'                  ; +0    : magic
pnp_revision    db      1                       ; +4    : revision
pnp_length      db      2                       ; +5    : length (in 16byte paragraphs)
pnp_next        dw      0                       ; +6    : offset of the next header (0 if none)
pnp_reserv1     db      0                       ; +8
pnp_checksum    db      0                       ; +9    : checksum
pnp_devid       dd      0x0                     ; +A    : device identifier
pnp_manstr      dw      manstr                  ; +E    : pointer to manufacturer string
pnp_prodstr     dw      prodstr                 ; +10   : pointer to product name string
pnp_devtype1    db      2                       ; +12   : device type code
pnp_devtype2    dw      0x000
pnp_devind      db      0x14                    ; +15   : device indicators
pnp_bcv         dw      0                       ; +16   : boot connection vector (must be 0)
pnp_discv       dw      0                       ; +18   : disconnect vector
pnp_bev         dw      boot_entry              ; +1A   : boot entry vector
pnp_reserv2     dw      0                       ; +1C
pnp_info        dw      0                       ; +1E   : static resource information vector

align 16
pci_header:
;       PCI Data Structure
pci_magic       db      'PCIR'                  ; +0    : magic
pci_vendor      dw      BIOS_PCI_VENDOR         ; +4    : 
pci_device      dw      BIOS_PCI_DEVICE         ; +6    : sb700 PCI bridge
pci_vdata       dw      0                       ; +8    : vital product data offset
pci_length      dw      0x18                    ; +A    : PCI data structure length
pci_classrev    dd      BIOS_PCI_CLASS          ; +C    : rev.00 + class 04.00.00
pci_size        dw      BIOS_BOOT_BLOCK_SIZE    ; +10   : image length (512byte blocks)
pci_rev         dw      0                       ; +12
pci_codetype    db      0                       ; +14   : x86
pci_indicator   db      0x80                    ; +15   : last image
pci_reserved    dw      0             

align 4
manstr:
        db      'Kolibri-A Operation System',0
prodstr:
        db      'ver.ROM-0.1',0

align 4
boot_entry:
@@:


boot_failure:
        int     18h                             ; return to BIOS Boot sequence

align   4
init_entry:
        xor     ax, ax
        mov     [cs:rom_length], al
        mov     al, 0x20       
        retf


check = 0
repeat $-$$
        load a byte from $$+%-1
        check = a + check
end repeat

check_byte   db      0x100 - (check mod 256)

times   (512-$) db 0