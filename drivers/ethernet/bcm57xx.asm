;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2026. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;  Broadcom NetXtreme 57xx driver for KolibriOS                   ;;
;;                                                                 ;;
;;  Chip families: 5700/5701, 5703/5704, 5705, 5750/5751, 57780    ;;
;;  No firmware download, no jumbo frames, no checksum offload.    ;;
;;                                                                 ;;
;;  references:                                                    ;;
;;    OpenBSD if_bge.c / if_bgereg.h (Bill Paul)                   ;;
;;    Broadcom 57XX Programmer's Guide                             ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native
entry START

        CURRENT_API             = 0x0200
        COMPATIBLE_API          = 0x0100
        API_VERSION             = (COMPATIBLE_API shl 16) + CURRENT_API

; configureable area

        MAX_DEVICES             = 16

        __DEBUG__               = 1
        __DEBUG_LEVEL__         = 2     ; 1 = verbose (per packet), 2 = normal

; How many events of each kind get logged in full. After that only the
; ordinals that are a power of two are printed, so a long run stays
; readable but never goes completely silent - which matters, because a
; hard cap hides exactly the moment things stop working.
        DBG_EVENTS              = 16

; Ring sizes are fixed by the chip, not chosen. The send and standard
; receive rings are mirrored into NIC SRAM at hard wired addresses:
; the send mirror occupies 0x4000..0x6000, which is exactly 512 send
; descriptors, and the standard receive mirror starts right after it at
; 0x6000. Both Linux tg3 and OpenBSD program these two rings at those
; sizes and nothing else, and the mirror address is handed to the chip
; in the ring control block, so a smaller host ring does not describe a
; smaller mirror - it just leaves the chip walking off the end of ours.
        TX_RING_SIZE            = 512
        RX_STD_SIZE             = 512

; The receive return ring is host resident and its length does come from
; the ring control block, but the chip has a per-family maximum for it
; (tg3's TG3_RX_RET_MAX_SIZE_5705 / _5700) and both reference drivers
; simply use that maximum. The actual value is picked per chip in
; block_init; this is only how much room to reserve.
        RX_RET_MAX              = 1024

; How many receive buffers we keep posted. The kernel has NET_BUFFERS
; (512) buffers in total and the ethernet input queue can hold another
; ETH_QUEUE_SIZE (255) of them, so keeping too many posted here starves
; the transmit path: eth_output() fails silently when the pool is empty
; and that looks exactly like a dead transmitter.
        RX_BUFFERS              = 64

        RX_BUF_SIZE             = 1536

; Interrupt coalescing. One interrupt per frame costs more than the
; frame itself at gigabit rates. Transmit completion is also reclaimed
; from transmit() itself, so these only affect latency, never progress.
        RX_COAL_TICKS           = 72    ; microseconds
        TX_COAL_TICKS           = 150
        RX_MAX_COAL_BDS         = 5     ; frames
        TX_MAX_COAL_BDS         = 8
        RX_MAX_COAL_BDS_INT     = 5     ; while an interrupt is pending
        TX_MAX_COAL_BDS_INT     = 5

; Perform a real core-clock reset (OpenBSD bge_reset).
;
; DISABLED BY DEFAULT. On PCI Express parts (5750/5751 and later) the
; link retrains after the reset and configuration space reads back as
; 0xFFFFFFFF for far longer than we wait; restoring the PCIe capability
; block is also needed. A 5751 came up with "chip id 0xFFFFFFFF" this
; way. The plain MAC reset below is what the chips in the field were
; brought up with, so that is the default.
        USE_CORE_RESET          = 0

; end configureable area

section '.flat' readable writable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'
include '../netdrv.inc'
include 'bcm57xx.inc'

macro assert_pow2 name, value {
  if (bsr value) > (bsf value)
    display name, ' must be a power of two', 13, 10
    err
  end if
}

assert_pow2 'TX_RING_SIZE', TX_RING_SIZE
assert_pow2 'RX_RET_MAX', RX_RET_MAX
assert_pow2 'RX_STD_SIZE', RX_STD_SIZE

if RX_BUFFERS >= RX_STD_SIZE
  display 'RX_BUFFERS must be smaller than RX_STD_SIZE', 13, 10
  err
end if

TX_BD_SHIFT             = bsf sizeof.tx_bd      ; 4
RX_BD_SHIFT             = bsf sizeof.rx_bd      ; 5

;-----------------------------------------------------------------------
; Layout of the shared DMA area. Everything the chip reads or writes by
; itself lives here, in one physically contiguous block.
;-----------------------------------------------------------------------

DMA_STATUS_OFS          = 0x0000                        ; status block
DMA_STATS_OFS           = 0x0100                        ; stats block, pre-5705
DMA_TX_OFS              = 0x0200
DMA_RXRET_OFS           = DMA_TX_OFS + TX_RING_SIZE * sizeof.tx_bd
DMA_RXSTD_OFS           = DMA_RXRET_OFS + RX_RET_MAX * sizeof.rx_bd
DMA_USED                = DMA_RXSTD_OFS + RX_STD_SIZE * sizeof.rx_bd

; kernel_alloc() hands out a physically contiguous run only for whole
; groups of eight pages, so always ask for a multiple of 32 KiB.
DMA_ALLOC               = ((DMA_USED + 8*4096 - 1) / (8*4096)) * (8*4096)


struct  device          ETH_DEVICE

        mmio_addr       dd ?
        pci_bus         dd ?
        pci_dev         dd ?
        irq_line        dd ?
        irq_attached    dd ?

        vendor_id       dd ?
        device_id       dd ?
        chip_id         dd ?            ; full chip id
        asic_rev        dd ?            ; chip_id shr 12
        is_5705_plus    dd ?            ; 0 = 5700 class programming model

        dma_virt        dd ?            ; shared DMA block
        dma_phys        dd ?
        status_blk      dd ?            ; pointers into the DMA block
        tx_ring         dd ?
        rx_ret          dd ?
        rx_std          dd ?

        rx_ret_size     dd ?            ; return ring length, per chip
        rx_ret_mask     dd ?            ; rx_ret_size - 1

        tx_prod         dd ?            ; next send slot to fill
        tx_cons         dd ?            ; next send slot to reclaim
        rx_ret_cons     dd ?            ; next return slot to process
        rx_std_prod     dd ?            ; next std slot to post
        rx_posted       dd ?            ; buffers currently owned by the chip

        irq_count       dd ?            ; diagnostics only
        txfull_count    dd ?
        oom_count       dd ?

        tx_buffs        rd TX_RING_SIZE
        rx_buffs        rd RX_STD_SIZE

ends


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc START             ;;
;;                        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc START c, reason:dword, cmdline:dword

        cmp     [reason], DRV_ENTRY
        jne     .fail

        DEBUGF  2,"Loading driver, rings tx %u, rx return %u, rx std %u, buffers %u\n",\
        TX_RING_SIZE, RX_RET_MAX, RX_STD_SIZE, RX_BUFFERS
        invoke  RegService, my_service, service_proc
        ret

  .fail:
        xor     eax, eax
        ret

endp


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                        ;;
;; proc SERVICE_PROC      ;;
;;                        ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc service_proc stdcall, ioctl:dword

        mov     edx, [ioctl]
        mov     eax, [edx + IOCTL.io_code]

;------------------------------------------------------

        cmp     eax, 0 ;SRV_GETVERSION
        jne     @F

        cmp     [edx + IOCTL.out_size], 4
        jb      .fail
        mov     eax, [edx + IOCTL.output]
        mov     [eax], dword API_VERSION

        xor     eax, eax
        ret

;------------------------------------------------------
  @@:
        cmp     eax, 1 ;SRV_HOOK
        jne     .fail

        cmp     [edx + IOCTL.inp_size], 3
        jb      .fail

        mov     eax, [edx + IOCTL.input]
        cmp     byte [eax], 1
        jne     .fail

; check if the device is already listed

        mov     esi, device_list
        mov     ecx, [devices]
        test    ecx, ecx
        jz      .firstdevice

        mov     ax, [eax+1]
  .nextdevice:
        mov     ebx, [esi]
        cmp     al, byte [ebx + device.pci_bus]
        jne     .next
        cmp     ah, byte [ebx + device.pci_dev]
        je      .find_devicenum
  .next:
        add     esi, 4
        loop    .nextdevice

  .firstdevice:
        cmp     [devices], MAX_DEVICES
        jae     .too_many

        allocate_and_clear ebx, sizeof.device, .no_memory

        mov     [ebx + device.reset], reset
        mov     [ebx + device.transmit], transmit
        mov     [ebx + device.unload], unload
        mov     [ebx + device.name], my_service

        mov     eax, [edx + IOCTL.input]
        movzx   ecx, byte [eax+1]
        mov     [ebx + device.pci_bus], ecx
        movzx   ecx, byte [eax+2]
        mov     [ebx + device.pci_dev], ecx

        DEBUGF  2,"Hooking bus:%u dev:%u\n", [ebx + device.pci_bus], [ebx + device.pci_dev]

; Read the identity first. If this comes back all ones the card is not
; answering at all and everything after it would be noise.
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.vendor_id
        mov     ecx, eax
        and     eax, 0xffff
        mov     [ebx + device.vendor_id], eax
        shr     ecx, 16
        mov     [ebx + device.device_id], ecx
        DEBUGF  2,"PCI id %x:%x\n", [ebx + device.vendor_id]:4, [ebx + device.device_id]:4

        cmp     [ebx + device.vendor_id], 0xffff
        je      .no_response
        cmp     [ebx + device.vendor_id], 0x14e4        ; Broadcom
        je      @f
        DEBUGF  2,"Vendor is not Broadcom, refusing to drive it\n"
        jmp     .destroy
  @@:

; Some boards come up in D3 with unusable BARs, force D0 first.
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_PWRMGMT_CMD
        DEBUGF  2,"PMCSR was 0x%x\n", eax
        and     eax, not PCI_PWR_STATE_MASK
        or      eax, PCI_PWR_PME_ENABLE
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_PWRMGMT_CMD, eax
        mov     esi, 1
        invoke  Sleep

        call    pci_enable
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        DEBUGF  2,"PCI command now 0x%x\n", eax

        call    dump_bars

        stdcall PCI_find_mmio, [ebx + device.pci_bus], [ebx + device.pci_dev]
        test    eax, eax
        jnz     @f
        DEBUGF  2,"No usable 32 bit MMIO BAR, cannot drive this card\n"
        jmp     .destroy
  @@:
        DEBUGF  2,"MMIO physical base 0x%x\n", eax

        invoke  MapIoMem, eax, 10000h, PG_SW+PG_NOCACHE
        test    eax, eax
        jnz     @f
        DEBUGF  2,"MapIoMem failed\n"
        jmp     .destroy
  @@:
        mov     [ebx + device.mmio_addr], eax

        invoke  PciRead8, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.interrupt_line
        movzx   eax, al
        mov     [ebx + device.irq_line], eax

        DEBUGF  2,"MMIO mapped at 0x%x, irq %u\n", [ebx + device.mmio_addr], [ebx + device.irq_line]

        call    alloc_dma
        test    eax, eax
        jnz     .destroy

        call    probe
        test    eax, eax
        jnz     .destroy

        mov     eax, [devices]
        mov     [device_list+4*eax], ebx
        inc     [devices]

        call    reset
        test    eax, eax
        jnz     .unlist

        mov     [ebx + device.type], NET_TYPE_ETH
        invoke  NetRegDev
        cmp     eax, -1
        jne     @f
        DEBUGF  2,"NetRegDev refused the device\n"
        jmp     .unlist
  @@:
        DEBUGF  2,"Registered as network device %u\n", eax
        ret

  .find_devicenum:
        invoke  NetPtrToNum
        mov     eax, edi
        DEBUGF  2,"Device already registered as %u\n", eax
        ret

  .no_response:
        DEBUGF  2,"No response from bus:%u dev:%u, vendor id reads 0xffff\n",\
        [ebx + device.pci_bus], [ebx + device.pci_dev]
        jmp     .destroy

  .too_many:
        DEBUGF  2,"Too many devices, MAX_DEVICES is %u\n", MAX_DEVICES
        jmp     .fail

  .no_memory:
        DEBUGF  2,"Could not allocate the device structure\n"
        jmp     .fail

; The device made it into device_list but never came up. Take it back
; out again.
  .unlist:
        dec     [devices]
        mov     eax, [devices]
        mov     dword [device_list+4*eax], 0

; The kernel exports no way to detach an interrupt handler, and the
; handler holds a raw pointer to this structure. Once it is hooked the
; structure has to stay alive, so silence the chip and leak it rather
; than leave the handler pointing at freed memory.
        cmp     [ebx + device.irq_attached], 0
        je      .destroy
        call    chip_stop
        DEBUGF  2,"Device left hooked, structure cannot be released\n"
        or      eax, -1
        ret

  .destroy:
        call    free_dma
        invoke  KernelFree, ebx
  .fail:
        or      eax, -1
        ret

endp


;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;
;;                                                                        ;;
;;        Hardware dependent code                                         ;;
;;                                                                        ;;
;;/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\;;

; The kernel exports no way to detach an interrupt handler, so the
; driver cannot be unloaded once a device is hooked.
unload:
        or      eax, -1
        ret


;***************************************************************************
;  dbg_worthy - decide whether this event ordinal gets a log line
;
;  in:  eax = event number, counting from one
;  out: CF clear when it should be logged
;
;  The first DBG_EVENTS of everything are printed, and after that only
;  the powers of two. A run of ten thousand frames then costs about
;  twenty lines but still shows 256, 512, 1024 and so on, so a failure
;  that happens at a round number is visible instead of being cut off
;  by a hard cap.
;***************************************************************************

dbg_worthy:

        push    ecx
        cmp     eax, DBG_EVENTS
        jbe     .yes
        lea     ecx, [eax - 1]
        test    eax, ecx
        jnz     .no
  .yes:
        pop     ecx
        clc
        ret
  .no:
        pop     ecx
        stc
        ret


;***************************************************************************
;  dump_bars - show every base address register
;
;  PCI_find_mmio only reports success or failure, and a card behind a
;  bridge that hands out a 64 bit BAR above 4 GiB looks identical to a
;  dead card from here. Print the raw values so the difference is
;  obvious in the log.
;***************************************************************************

dump_bars:

        push    eax ecx esi

        mov     esi, PCI_header00.base_addr_0
        xor     ecx, ecx
  .loop:
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], esi
        DEBUGF  2,"BAR%u = 0x%x\n", ecx, eax
        add     esi, 4
        inc     ecx
        cmp     ecx, 6
        jb      .loop

        pop     esi ecx eax
        ret


;***************************************************************************
;  alloc_dma / free_dma
;
;  One contiguous block holds the status block, the statistics block and
;  all three descriptor rings. Returns eax = 0 on success.
;***************************************************************************

alloc_dma:

        invoke  KernelAlloc, DMA_ALLOC
        test    eax, eax
        jz      .fail
        mov     [ebx + device.dma_virt], eax

        mov     edi, eax
        mov     ecx, DMA_ALLOC / 4
        xor     eax, eax
        rep     stosd

        mov     eax, [ebx + device.dma_virt]
        invoke  GetPhysAddr
        mov     [ebx + device.dma_phys], eax

        mov     eax, [ebx + device.dma_virt]
        lea     edx, [eax + DMA_STATUS_OFS]
        mov     [ebx + device.status_blk], edx
        lea     edx, [eax + DMA_TX_OFS]
        mov     [ebx + device.tx_ring], edx
        lea     edx, [eax + DMA_RXRET_OFS]
        mov     [ebx + device.rx_ret], edx
        lea     edx, [eax + DMA_RXSTD_OFS]
        mov     [ebx + device.rx_std], edx

        DEBUGF  2,"DMA area virt 0x%x phys 0x%x size %u\n",\
        [ebx + device.dma_virt], [ebx + device.dma_phys], DMA_ALLOC
        DEBUGF  2,"  status 0x%x  tx 0x%x  rxret 0x%x  rxstd 0x%x (physical)\n",\
        DMA_STATUS_OFS, DMA_TX_OFS, DMA_RXRET_OFS, DMA_RXSTD_OFS

        xor     eax, eax
        ret

  .fail:
        DEBUGF  2,"Could not allocate %u bytes of DMA memory\n", DMA_ALLOC
        or      eax, -1
        ret


free_dma:

        mov     eax, [ebx + device.dma_virt]
        test    eax, eax
        jz      @f
        invoke  KernelFree, eax
        mov     [ebx + device.dma_virt], 0
  @@:
        ret


;***************************************************************************
;  pci_enable - memory space + bus mastering, keeping the status word
;***************************************************************************

pci_enable:

        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        or      eax, PCI_CMD_MMIO or PCI_CMD_MASTER
        and     eax, 0x0000ffff                 ; do not write back status bits
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax
        ret


;***************************************************************************
;  probe
;
;  Identify the chip, put it into a known state and read the station
;  address. Returns eax = 0 on success.
;***************************************************************************

probe:

; Indirect register access and the endian swap options have to be live
; before anything else touches the chip.
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL, BGE_INIT
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL
        DEBUGF  2,"MISC_CTL after init write 0x%x\n", eax

        call    read_chip_id
        test    eax, eax
        jnz     .gone

; DMA read/write control. 5700/5701 need memory-read-multiple and
; assert-all-byte-enables on top of the usual watermarks.
        mov     eax, PCIDMARWCTL_DEFAULT
        cmp     [ebx + device.is_5705_plus], 0
        jne     @f
        or      eax, PCIDMARWCTL_USE_MRM or PCIDMARWCTL_ASRT_ALL_BE
  @@:
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_DMA_RW_CTL, eax
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_DMA_RW_CTL
        DEBUGF  2,"DMA_RW_CTL 0x%x\n", eax

        call    chip_reset

; A core reset reloads the identification registers, and on a part that
; did not survive it they now read as all ones.
        call    read_chip_id
        test    eax, eax
        jnz     .gone

        mov     eax, [ebx + device.chip_id]
        mov     esi, chip_table
  .name_loop:
        cmp     dword [esi+4], 0                ; the name pointer terminates
        je      .name_unknown                   ; the table, chip id 0 is valid
        cmp     eax, [esi]
        je      .name_found
        add     esi, 8
        jmp     .name_loop
  .name_found:
        mov     esi, [esi+4]
        mov     [ebx + device.name], esi
        DEBUGF  2,"Found %s, chip id 0x%x asic rev 0x%x\n", esi,\
        [ebx + device.chip_id], [ebx + device.asic_rev]
        jmp     .name_done
  .name_unknown:
        mov     [ebx + device.name], name_unknown
        DEBUGF  2,"Unknown BCM57xx, chip id 0x%x asic rev 0x%x\n",\
        [ebx + device.chip_id], [ebx + device.asic_rev]
        DEBUGF  2,"  driving it anyway, please report the ids above\n"
  .name_done:
        cmp     [ebx + device.is_5705_plus], 0
        jne     @f
        DEBUGF  2,"5700 class programming model\n"
        jmp     .family_done
  @@:
        DEBUGF  2,"5705 class programming model\n"
  .family_done:

        call    read_mac
        test    eax, eax
        jnz     .fail

        xor     eax, eax
        ret

  .gone:
        DEBUGF  2,"Chip stopped answering, identification reads 0x%x\n",\
        [ebx + device.chip_id]
if USE_CORE_RESET
        DEBUGF  2,"  the core reset is enabled, try USE_CORE_RESET = 0\n"
end if
  .fail:
        DEBUGF  2,"Probe failed\n"
        or      eax, -1
        ret


;***************************************************************************
;  read_chip_id
;
;  Returns eax = 0 when the chip answered, -1 when the register read
;  back as all ones (device off the bus).
;***************************************************************************

read_chip_id:

        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL
        shr     eax, PCIMISCCTL_ASICREV_SHIFT
        mov     ecx, eax
        shr     ecx, 12
; 5717 and later park 0x0f here and keep the real id in a separate
; configuration register.
        cmp     ecx, ASICREV_USE_PRODID_REG
        jne     @f
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_PRODID_ASICREV
        mov     ecx, eax
        shr     ecx, 12
  @@:
        mov     [ebx + device.chip_id], eax
        mov     [ebx + device.asic_rev], ecx

; Which programming model this part wants. The 5700 class needs the
; buffer manager pools, the mini receive ring, the list selector, a
; statistics block and a longer return ring; everything from the 5705
; onwards does not.
;
; This cannot be a "revision >= 5705" test, because the revision numbers
; are not chronological: the BCM5700 carries revision 0x07, which is
; higher than the 0x03 of the 5705 it predates. Only these four
; revisions are 5700 class, so name them.
        mov     edx, 1
        cmp     ecx, ASICREV_BCM5701            ; 0x00, 5700/5701
        je      .old_family
        cmp     ecx, ASICREV_BCM5703            ; 0x01
        je      .old_family
        cmp     ecx, ASICREV_BCM5704            ; 0x02
        je      .old_family
        cmp     ecx, ASICREV_BCM5700            ; 0x07
        jne     @f
  .old_family:
        xor     edx, edx
  @@:
        mov     [ebx + device.is_5705_plus], edx

        cmp     eax, 0xffffffff
        je      .gone
        cmp     eax, 0xffff
        je      .gone
        xor     eax, eax
        ret
  .gone:
        or      eax, -1
        ret


;***************************************************************************
;  chip_reset
;***************************************************************************

chip_reset:

if USE_CORE_RESET

        DEBUGF  2,"Core clock reset\n"

; Save the configuration registers the reset clobbers.
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command
        push    eax
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_CACHESZ
        push    eax
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.base_addr_0
        push    eax
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.base_addr_1
        push    eax
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_DMA_RW_CTL
        push    eax

        mov     edx, MISCCFG_RESET_CORE_CLOCKS or MISCCFG_TIMER_PRESCALE_66MHZ
        mov     eax, MISC_CFG
        call    csr_write

; Wait for configuration space to answer again. A PCI Express part
; retrains its link here and reads back as all ones until it is done.
        mov     edi, 20                         ; up to 200 ms
  .wait_alive:
        mov     esi, 1
        invoke  Sleep
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.vendor_id
        cmp     ax, 0xffff
        jne     .alive
        dec     edi
        jnz     .wait_alive
        DEBUGF  2,"Device did not come back after the core reset\n"
  .alive:

; Restore, indirect access last: nothing else works until it is back.
        pop     eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_DMA_RW_CTL, eax
        pop     eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.base_addr_1, eax
        pop     eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.base_addr_0, eax
        pop     eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_CACHESZ, eax
        pop     eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_header00.command, eax

        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL, BGE_INIT
        call    pci_enable

else

; Bounce the MAC only. This is what the chips in the field were brought
; up with; see the comment on USE_CORE_RESET.
        mov     edx, MAC_MODE_RESET
        mov     eax, MAC_MODE
        call    csr_write
        mov     esi, 1
        invoke  Sleep
        xor     edx, edx
        mov     eax, MAC_MODE
        call    csr_write

end if

; Timer prescaler, always driven from the 66 MHz core clock.
        mov     edx, MISCCFG_TIMER_PRESCALE_66MHZ
        mov     eax, MISC_CFG
        call    csr_write

; The memory arbiter gates every access to NIC internal memory. Without
; it the buffer manager on 5700/5701 never starts.
        mov     edx, MARBMODE_ENABLE
        mov     eax, MARB_MODE
        call    csr_write
        mov     eax, MARB_MODE
        call    csr_read
        DEBUGF  2,"Memory arbiter mode 0x%x\n", eax

; Byte and word swapping for descriptors and data, little endian host.
        mov     edx, MODECTL_BYTESWAP_DATA or MODECTL_WORDSWAP_DATA \
                  or MODECTL_WORDSWAP_NONFRAME
        mov     eax, MODE_CTRL
        call    csr_write

        ret


;***************************************************************************
;  read_mac
;
;  1) NIC internal memory at 0x0c14, tagged with the "HK" signature
;  2) the MAC address registers, if the boot code already filled them
;  3) the serial EEPROM
;
;  Returns eax = 0 on success.
;***************************************************************************

read_mac:

;--- via NIC memory ----------------------------------------------------
        mov     eax, NICMEM_SOFTWARE_MAC
        call    mem_read
        mov     edx, eax
        shr     edx, 16
        cmp     dx, 0x484b                      ; "HK"
        jne     .try_registers

        mov     byte [ebx + device.mac+0], ah
        mov     byte [ebx + device.mac+1], al

        mov     eax, NICMEM_SOFTWARE_MAC + 4
        call    mem_read
        mov     byte [ebx + device.mac+5], al
        mov     byte [ebx + device.mac+4], ah
        shr     eax, 16
        mov     byte [ebx + device.mac+3], al
        mov     byte [ebx + device.mac+2], ah

        call    mac_is_valid
        jc      @f
        DEBUGF  2,"Station address from NIC memory\n"
        jmp     .done
  @@:
        DEBUGF  2,"NIC memory holds an invalid station address\n"

;--- via the MAC address registers -------------------------------------
  .try_registers:
        mov     esi, [ebx + device.mmio_addr]
        mov     eax, [esi + MAC_ADDR0_HI]       ; 0x0000 mac[0] mac[1]
        mov     edx, [esi + MAC_ADDR0_LO]       ; mac[2..5]
        DEBUGF  2,"MAC_ADDR0 registers 0x%x 0x%x\n", eax, edx

        mov     byte [ebx + device.mac+0], ah
        mov     byte [ebx + device.mac+1], al
        mov     eax, edx
        shr     eax, 16
        mov     byte [ebx + device.mac+2], ah
        mov     byte [ebx + device.mac+3], al
        mov     byte [ebx + device.mac+4], dh
        mov     byte [ebx + device.mac+5], dl

        call    mac_is_valid
        jc      @f
        DEBUGF  2,"Station address from the MAC registers\n"
        jmp     .done
  @@:

;--- via the serial EEPROM ---------------------------------------------
        DEBUGF  2,"Reading the station address from EEPROM\n"
        mov     esi, [ebx + device.mmio_addr]

        mov     eax, [esi + MISC_LOCAL_CTL]
        or      eax, MLC_AUTO_EEPROM
        mov     [esi + MISC_LOCAL_CTL], eax

        mov     dword [esi + EE_ADDR], EEADDR_RESET or (HALFCLK_384SCL shl 16)
        mov     ecx, 100
        call    delay_us

        push    ebp                             ; service_proc's frame pointer
        lea     edi, [ebx + device.mac]
        mov     edx, EE_MAC_OFFSET + 2
        mov     ebp, 6
  .ee_loop:
        mov     eax, EE_READCMD
        or      eax, edx
        mov     [esi + EE_ADDR], eax

        mov     ecx, 10
        call    delay_us

        mov     ecx, 10000
  .ee_wait:
        mov     eax, [esi + EE_ADDR]
        test    eax, EEADDR_DONE
        jnz     .ee_got
        dec     ecx
        jnz     .ee_wait
        DEBUGF  2,"EEPROM timeout at offset 0x%x, EE_ADDR 0x%x\n", edx, eax
        pop     ebp
        jmp     .fail

  .ee_got:
        mov     eax, [esi + EE_DATA]
        mov     ecx, edx
        and     ecx, 3
        shl     ecx, 3
        shr     eax, cl
        stosb
        inc     edx
        dec     ebp
        jnz     .ee_loop
        pop     ebp

        call    mac_is_valid
        jc      .fail
        DEBUGF  2,"Station address from EEPROM\n"

  .done:
        DEBUGF  2,"MAC = %x-%x-%x-%x-%x-%x\n",\
        [ebx + device.mac+0]:2, [ebx + device.mac+1]:2, [ebx + device.mac+2]:2,\
        [ebx + device.mac+3]:2, [ebx + device.mac+4]:2, [ebx + device.mac+5]:2
        xor     eax, eax
        ret

  .fail:
        DEBUGF  2,"Could not read the station address\n"
        or      eax, -1
        ret


;***************************************************************************
;  mac_is_valid - CF clear when device.mac is neither all zero nor all ff
;***************************************************************************

mac_is_valid:

        push    eax ecx edx esi edi
        lea     esi, [ebx + device.mac]
        xor     eax, eax                        ; eax = or of all bytes
        mov     ecx, 0xff                       ; ecx = and of all bytes
        mov     edi, 6
  @@:
        movzx   edx, byte [esi]
        or      eax, edx
        and     ecx, edx
        inc     esi
        dec     edi
        jnz     @b

        test    eax, eax                        ; all zero?
        jz      .bad
        cmp     ecx, 0xff                       ; all ones?
        je      .bad
        test    byte [ebx + device.mac], 1      ; group address?
        jnz     .bad
        clc
        pop     edi esi edx ecx eax
        ret
  .bad:
        stc
        pop     edi esi edx ecx eax
        ret


;***************************************************************************
;  reset - full bring-up, also used as the kernel's reset entry point
;
;  Returns eax = 0 on success. Safe to call more than once.
;***************************************************************************

reset:

        DEBUGF  2,"Bring-up starting\n"

; Stop the chip from touching memory we are about to reshuffle.
        call    chip_stop
        call    free_buffers

        mov     [ebx + device.tx_prod], 0
        mov     [ebx + device.tx_cons], 0
        mov     [ebx + device.rx_ret_cons], 0
        mov     [ebx + device.rx_std_prod], 0
        mov     [ebx + device.rx_posted], 0
        mov     [ebx + device.irq_count], 0
        mov     [ebx + device.txfull_count], 0
        mov     [ebx + device.oom_count], 0

        mov     edi, [ebx + device.dma_virt]
        mov     ecx, DMA_ALLOC / 4
        xor     eax, eax
        rep     stosd

        call    chip_reset

        call    block_init
        test    eax, eax
        jnz     .fail

; Fill the receive ring before the MAC starts accepting frames.
        call    rx_refill
        DEBUGF  2,"Posted %u of %u receive buffers\n",\
        [ebx + device.rx_posted], RX_BUFFERS
        cmp     [ebx + device.rx_posted], 0
        jne     @f
        DEBUGF  2,"No receive buffers available at all\n"
        jmp     .fail
  @@:

        call    mac_enable

; The interrupt handler must not run before the rings are valid, and we
; have no way to detach it again, so hook it exactly once.
        cmp     [ebx + device.irq_attached], 0
        jne     .irq_done
        invoke  AttachIntHandler, [ebx + device.irq_line], int_handler, ebx
        test    eax, eax
        jnz     @f
        DEBUGF  2,"Could not attach handler for irq %u\n", [ebx + device.irq_line]
        jmp     .fail
  @@:
        mov     [ebx + device.irq_attached], 1
        DEBUGF  2,"Interrupt handler attached to irq %u\n", [ebx + device.irq_line]
  .irq_done:

        call    irq_unmask

        mov     [ebx + device.mtu], 1514

; Wait for the PHY here, in thread context. Auto-negotiation needs a
; second or two after the MAC is started, and until it finishes there is
; nothing sensible to report; if we just sampled once and moved on the
; interface would come up looking permanently unplugged even with the
; cable in, which is exactly what happened before.
;
; The link stays reported as down for the duration. ETH_LINK_UNKNOWN is
; not a "not looked yet" value - it means the link is up at an unknown
; speed - so it must not be used here.
        mov     [ebx + device.state], ETH_LINK_DOWN
        mov     edi, 200                        ; hundredths of a second
  .link_wait:
        call    check_link
        mov     eax, [ebx + device.state]
        and     eax, ETH_LINK_SPEED_MASK
        jnz     .link_done
        push    edi
        mov     esi, 1
        invoke  Sleep
        pop     edi
        dec     edi
        jnz     .link_wait
        DEBUGF  2,"No link after two seconds, carrying on\n"
  .link_done:
        DEBUGF  2,"Link state after bring-up 0x%x\n", [ebx + device.state]
        call    dump_state

        DEBUGF  2,"Bring-up done\n"
        xor     eax, eax
        ret

  .fail:
        DEBUGF  2,"Bring-up failed\n"
        call    dump_state
        call    chip_stop
        or      eax, -1
        ret


;***************************************************************************
;  dump_state - one shot picture of everything that matters
;
;  This is the first thing to look at in a bring-up log: it says whether
;  the state machines actually started and whether the ring control
;  blocks reached the chip intact.
;***************************************************************************

dump_state:

        push    eax ecx edx esi

        mov     esi, [ebx + device.mmio_addr]
        test    esi, esi
        jz      .done

        DEBUGF  2,"--- chip state ---\n"
        DEBUGF  2,"MODE_CTRL 0x%x  MAC_MODE 0x%x  HCC_MODE 0x%x\n",\
        [esi + MODE_CTRL], [esi + MAC_MODE], [esi + HCC_MODE]
        DEBUGF  2,"RX_MODE 0x%x  TX_MODE 0x%x  MAC_STS 0x%x\n",\
        [esi + MAC_RX_MODE], [esi + MAC_TX_MODE], [esi + MAC_STS]
        DEBUGF  2,"MI_STS 0x%x  MI_MODE 0x%x  RX_MTU %u\n",\
        [esi + MI_STS], [esi + MI_MODE], [esi + MAC_RX_MTU]
        DEBUGF  2,"WDMA 0x%x  RDMA 0x%x  BMAN 0x%x  MARB 0x%x\n",\
        [esi + WDMA_MODE], [esi + RDMA_MODE], [esi + BMAN_MODE], [esi + MARB_MODE]
        DEBUGF  2,"RXLP 0x%x  RDC 0x%x  RBDI 0x%x  RDBDI 0x%x\n",\
        [esi + RXLP_MODE], [esi + RDC_MODE], [esi + RBDI_MODE], [esi + RDBDI_MODE]
        DEBUGF  2,"SDI 0x%x  SDC 0x%x  SBDI 0x%x  SBDC 0x%x\n",\
        [esi + SDI_MODE], [esi + SDC_MODE], [esi + SBDI_MODE], [esi + SBDC_MODE]
        DEBUGF  2,"RX_STD RCB haddr 0x%x maxlen 0x%x nicaddr 0x%x\n",\
        [esi + RX_STD_RCB_HADDR_LO], [esi + RX_STD_RCB_MAXLEN_FLAGS],\
        [esi + RX_STD_RCB_NICADDR]
        DEBUGF  2,"HCC statusblk 0x%x  rx_ticks %u  rx_bds %u\n",\
        [esi + HCC_STATUSBLK_ADDR_LO], [esi + HCC_RX_COAL_TICKS],\
        [esi + HCC_RX_MAX_COAL_BDS]

; The send and return ring control blocks live in NIC memory, so they
; have to be read back through the memory window.
        mov     eax, NICMEM_SEND_RING_RCB + RCB_HADDR_LO
        call    mem_read
        mov     ecx, eax
        mov     eax, NICMEM_SEND_RING_RCB + RCB_MAXLEN_FLAGS
        call    mem_read
        mov     edx, eax
        mov     eax, NICMEM_SEND_RING_RCB + RCB_NICADDR
        call    mem_read
        DEBUGF  2,"TX RCB haddr 0x%x maxlen 0x%x nicaddr 0x%x\n", ecx, edx, eax

        mov     eax, NICMEM_RX_RETURN_RING_RCB + RCB_HADDR_LO
        call    mem_read
        mov     ecx, eax
        mov     eax, NICMEM_RX_RETURN_RING_RCB + RCB_MAXLEN_FLAGS
        call    mem_read
        DEBUGF  2,"RX return RCB haddr 0x%x maxlen 0x%x\n", ecx, eax

        mov     esi, [ebx + device.status_blk]
        DEBUGF  2,"status block word 0x%x tag 0x%x rx_prod %u tx_cons %u\n",\
        [esi + status_block.status_word], [esi + status_block.tag],\
        [esi + status_block.rx_prod_idx]:2, [esi + status_block.tx_cons_idx]:2
        DEBUGF  2,"rings tx_prod %u tx_cons %u rx_ret_cons %u/%u rx_std_prod %u posted %u\n",\
        [ebx + device.tx_prod], [ebx + device.tx_cons],\
        [ebx + device.rx_ret_cons], [ebx + device.rx_ret_size],\
        [ebx + device.rx_std_prod], [ebx + device.rx_posted]
        DEBUGF  2,"counters irq %u txfull %u oom %u tx %u rx %u\n",\
        [ebx + device.irq_count], [ebx + device.txfull_count],\
        [ebx + device.oom_count], [ebx + device.packets_tx],\
        [ebx + device.packets_rx]
        DEBUGF  2,"------------------\n"
  .done:
        pop     esi edx ecx eax
        ret


;***************************************************************************
;  chip_stop - quiesce the DMA engines and mask interrupts
;***************************************************************************

chip_stop:

        push    eax ecx edx esi

        mov     esi, [ebx + device.mmio_addr]
        test    esi, esi
        jz      .done

; Mask at the chip and at the PCI bridge.
        mov     dword [esi + MBX_IRQ0_LO], 1
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL
        or      eax, PCIMISCCTL_MASK_PCI_INTR
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL, eax

        mov     esi, [ebx + device.mmio_addr]
        and     dword [esi + MAC_RX_MODE], not RX_MODE_ENABLE
        and     dword [esi + MAC_TX_MODE], not TX_MODE_ENABLE
        and     dword [esi + MAC_MODE], not (MAC_MODE_TXDMA_EN or MAC_MODE_RXDMA_EN)
        and     dword [esi + RDMA_MODE], not RDMAMODE_ENABLE
        and     dword [esi + WDMA_MODE], not WDMAMODE_ENABLE
        and     dword [esi + HCC_MODE], not HCCMODE_ENABLE

        mov     ecx, 100
        call    delay_us
  .done:
        pop     esi edx ecx eax
        ret


;***************************************************************************
;  block_init - program every state machine, ring and coalescing knob
;
;  Follows the order of OpenBSD's bge_blockinit(). Returns eax = 0 on
;  success.
;***************************************************************************

block_init:

;-----------------------------------------------------------------------
; Buffer manager. 5705 and later carve up their own SRAM and only need
; the watermarks; older parts also want the pool base addresses.
;-----------------------------------------------------------------------
        cmp     [ebx + device.is_5705_plus], 0
        jne     .bman_5705plus

        DEBUGF  2,"Buffer manager setup for the 5700 family\n"
        mov     edx, BUFFPOOL_1
        mov     eax, BMAN_MBUFPOOL_BASEADDR
        call    csr_write
        mov     edx, BUFFPOOL_1_LEN
        mov     eax, BMAN_MBUFPOOL_LEN
        call    csr_write
        mov     edx, DMA_DESCRIPTORS
        mov     eax, BMAN_DMA_DESCPOOL_BASEADDR
        call    csr_write
        mov     edx, DMA_DESCRIPTORS_LEN
        mov     eax, BMAN_DMA_DESCPOOL_LEN
        call    csr_write
        mov     edx, 0x50
        mov     eax, BMAN_MBUFPOOL_READDMA_LOWAT
        call    csr_write
        mov     edx, 0x20
        mov     eax, BMAN_MBUFPOOL_MACRX_LOWAT
        call    csr_write
        mov     edx, 0x60
        mov     eax, BMAN_MBUFPOOL_HIWAT
        call    csr_write
        jmp     .bman_common

  .bman_5705plus:
        xor     edx, edx
        mov     eax, BMAN_MBUFPOOL_READDMA_LOWAT
        call    csr_write
        mov     edx, 0x10
        mov     eax, BMAN_MBUFPOOL_MACRX_LOWAT
        call    csr_write
        mov     edx, 0x60
        mov     eax, BMAN_MBUFPOOL_HIWAT
        call    csr_write

  .bman_common:
        mov     edx, 5
        mov     eax, BMAN_DMA_DESCPOOL_LOWAT
        call    csr_write
        mov     edx, 10
        mov     eax, BMAN_DMA_DESCPOOL_HIWAT
        call    csr_write

        mov     edx, BMANMODE_ENABLE or BMANMODE_LOMBUF_ATTN
        mov     eax, BMAN_MODE
        call    csr_write

        mov     edi, 2000
  .bman_wait:
        mov     eax, BMAN_MODE
        call    csr_read
        test    eax, BMANMODE_ENABLE
        jnz     .bman_ok
        mov     ecx, 10
        call    delay_us
        dec     edi
        jnz     .bman_wait
        DEBUGF  2,"Buffer manager did not start, BMAN_MODE 0x%x\n", eax
        or      eax, -1
        ret
  .bman_ok:
        DEBUGF  2,"Buffer manager running, BMAN_MODE 0x%x\n", eax

;-----------------------------------------------------------------------
; Flush the flow through queues.
;-----------------------------------------------------------------------
        mov     edx, 0xffffffff
        mov     eax, FTQ_RESET
        call    csr_write
        xor     edx, edx
        mov     eax, FTQ_RESET
        call    csr_write

        mov     edi, 2000
  .ftq_wait:
        mov     eax, FTQ_RESET
        call    csr_read
        test    eax, eax
        jz      .ftq_ok
        mov     ecx, 10
        call    delay_us
        dec     edi
        jnz     .ftq_wait
        DEBUGF  2,"Flow through queue reset timed out, FTQ 0x%x\n", eax
        or      eax, -1
        ret
  .ftq_ok:

;-----------------------------------------------------------------------
; Receive ring control blocks.
;-----------------------------------------------------------------------

; Jumbo ring, unused.
        mov     edx, RCB_FLAG_RING_DISABLED
        mov     eax, RX_JUMBO_RCB_MAXLEN_FLAGS
        call    csr_write

; Mini ring, only present before 5705.
        cmp     [ebx + device.is_5705_plus], 0
        jne     @f
        mov     edx, RCB_FLAG_RING_DISABLED
        mov     eax, RX_MINI_RCB_MAXLEN_FLAGS
        call    csr_write
  @@:

; Standard producer ring. Before 5705 the maxlen field carries the
; largest frame the chip may place, afterwards it carries the number of
; descriptors in the ring.
        xor     edx, edx
        mov     eax, RX_STD_RCB_HADDR_HI
        call    csr_write
        mov     edx, [ebx + device.dma_phys]
        add     edx, DMA_RXSTD_OFS
        mov     eax, RX_STD_RCB_HADDR_LO
        call    csr_write

        mov     edx, 1518 shl 16
        cmp     [ebx + device.is_5705_plus], 0
        je      @f
        mov     edx, RX_STD_SIZE shl 16
  @@:
        mov     eax, RX_STD_RCB_MAXLEN_FLAGS
        call    csr_write

        mov     edx, NICMEM_STD_RX_RINGS
        mov     eax, RX_STD_RCB_NICADDR
        call    csr_write

; When the chip's internal descriptor cache drops below this it fetches
; more from the ring. Scaled to how many buffers we actually post.
        mov     edx, RX_BUFFERS / 8
        mov     eax, RBDI_STD_REPL_THRESH
        call    csr_write

;-----------------------------------------------------------------------
; Send ring control blocks, in NIC memory. Disable every ring we do not
; use, then program ring 0.
;-----------------------------------------------------------------------
        mov     edi, 16                         ; rings available pre-5705
        cmp     [ebx + device.is_5705_plus], 0
        je      @f
        mov     edi, 4
  @@:
        mov     esi, NICMEM_SEND_RING_RCB
  .tx_rcb_clear:
        lea     eax, [esi + RCB_MAXLEN_FLAGS]
        mov     edx, RCB_FLAG_RING_DISABLED
        call    mem_write
        lea     eax, [esi + RCB_NICADDR]
        xor     edx, edx
        call    mem_write
        add     esi, sizeof.rcb
        dec     edi
        jnz     .tx_rcb_clear

        mov     eax, NICMEM_SEND_RING_RCB + RCB_HADDR_HI
        xor     edx, edx
        call    mem_write
        mov     eax, NICMEM_SEND_RING_RCB + RCB_HADDR_LO
        mov     edx, [ebx + device.dma_phys]
        add     edx, DMA_TX_OFS
        call    mem_write
        mov     eax, NICMEM_SEND_RING_RCB + RCB_MAXLEN_FLAGS
        mov     edx, TX_RING_SIZE shl 16
        call    mem_write

; The send ring is mirrored into NIC SRAM on every part in this family,
; at the fixed address below, and the chip needs to be told where that
; mirror is. tg3 and OpenBSD both write it unconditionally; the earlier
; version of this driver only did it before 5705 and left a zero here
; otherwise, which is what made a small host ring look like it worked.
        mov     eax, NICMEM_SEND_RING_RCB + RCB_NICADDR
        mov     edx, NICMEM_SEND_RING_1_TO_4
        call    mem_write

;-----------------------------------------------------------------------
; Receive return ring control blocks, also in NIC memory.
;
; The length is the architectural maximum for the family, the same value
; tg3 uses: 512 from 5705 onwards, 1024 on the 5700 class parts.
;-----------------------------------------------------------------------
        mov     eax, 1024
        cmp     [ebx + device.is_5705_plus], 0
        je      @f
        mov     eax, 512
  @@:
        mov     [ebx + device.rx_ret_size], eax
        dec     eax
        mov     [ebx + device.rx_ret_mask], eax
        DEBUGF  2,"Receive return ring length %u\n", [ebx + device.rx_ret_size]

        mov     edi, 16
        cmp     [ebx + device.is_5705_plus], 0
        je      @f
        mov     edi, 1
  @@:
        mov     esi, NICMEM_RX_RETURN_RING_RCB
  .rx_rcb_clear:
        lea     eax, [esi + RCB_HADDR_HI]
        xor     edx, edx
        call    mem_write
        lea     eax, [esi + RCB_HADDR_LO]
        xor     edx, edx
        call    mem_write
        lea     eax, [esi + RCB_MAXLEN_FLAGS]
        mov     edx, [ebx + device.rx_ret_size]
        shl     edx, 16
        or      edx, RCB_FLAG_RING_DISABLED
        call    mem_write
        lea     eax, [esi + RCB_NICADDR]
        xor     edx, edx
        call    mem_write
        add     esi, sizeof.rcb
        dec     edi
        jnz     .rx_rcb_clear

        mov     eax, NICMEM_RX_RETURN_RING_RCB + RCB_HADDR_HI
        xor     edx, edx
        call    mem_write
        mov     eax, NICMEM_RX_RETURN_RING_RCB + RCB_HADDR_LO
        mov     edx, [ebx + device.dma_phys]
        add     edx, DMA_RXRET_OFS
        call    mem_write
        mov     eax, NICMEM_RX_RETURN_RING_RCB + RCB_MAXLEN_FLAGS
        mov     edx, [ebx + device.rx_ret_size]
        shl     edx, 16
        call    mem_write

        DEBUGF  2,"Ring control blocks programmed\n"

;-----------------------------------------------------------------------
; Reset every producer and consumer mailbox.
;-----------------------------------------------------------------------
        xor     edx, edx
        mov     eax, MBX_RX_STD_PROD_LO
        call    mbx_write
        xor     edx, edx
        mov     eax, MBX_RX_JUMBO_PROD_LO
        call    mbx_write
        xor     edx, edx
        mov     eax, MBX_RX_MINI_PROD_LO
        call    mbx_write
        xor     edx, edx
        mov     eax, MBX_RX_CONS0_LO
        call    mbx_write
        xor     edx, edx
        mov     eax, MBX_TX_HOST_PROD0_LO
        call    mbx_write

;-----------------------------------------------------------------------
; Station address and transmit parameters.
;-----------------------------------------------------------------------
        movzx   eax, byte [ebx + device.mac+0]
        movzx   edx, byte [ebx + device.mac+1]
        shl     eax, 8
        or      edx, eax
        mov     eax, MAC_ADDR0_HI
        call    csr_write

        movzx   eax, byte [ebx + device.mac+2]
        shl     eax, 24
        movzx   ecx, byte [ebx + device.mac+3]
        shl     ecx, 16
        or      eax, ecx
        movzx   ecx, byte [ebx + device.mac+4]
        shl     ecx, 8
        or      eax, ecx
        movzx   ecx, byte [ebx + device.mac+5]
        or      eax, ecx
        mov     edx, eax
        mov     eax, MAC_ADDR0_LO
        call    csr_write

; Random backoff seed, the sum of the address bytes.
        xor     edx, edx
        xor     ecx, ecx
  @@:
        movzx   eax, byte [ebx + device.mac + ecx]
        add     edx, eax
        inc     ecx
        cmp     ecx, 6
        jb      @b
        and     edx, TX_BACKOFF_SEED_MASK
        mov     eax, MAC_TX_BACKOFF_SEED
        call    csr_write

        mov     edx, TX_LENGTHS_DEFAULT         ; inter packet gap
        mov     eax, MAC_TX_LENGTHS
        call    csr_write

        mov     edx, 1518
        mov     eax, MAC_RX_MTU
        call    csr_write

;-----------------------------------------------------------------------
; Receive rules and list placement. Frames that match no rule go to
; return ring 0; without these two the chip drops everything.
;-----------------------------------------------------------------------
        mov     edx, 0x08
        mov     eax, RX_RULES_CFG
        call    csr_write

        mov     edx, RXLP_CFG_DEFAULT
        mov     eax, RXLP_CFG
        call    csr_write

        mov     edx, 0x007fffff
        mov     eax, RXLP_STATS_ENABLE_MASK
        call    csr_write
        mov     edx, 1
        mov     eax, RXLP_STATS_CTL
        call    csr_write

        mov     edx, 0x007fffff
        mov     eax, SDI_STATS_ENABLE_MASK
        call    csr_write
        mov     edx, 1
        mov     eax, SDI_STATS_CTL
        call    csr_write

;-----------------------------------------------------------------------
; Host coalescing: where the status block lives and how often the chip
; is allowed to interrupt us.
;-----------------------------------------------------------------------
        xor     edx, edx
        mov     eax, HCC_STATUSBLK_ADDR_HI
        call    csr_write
        mov     edx, [ebx + device.dma_phys]
        add     edx, DMA_STATUS_OFS
        mov     eax, HCC_STATUSBLK_ADDR_LO
        call    csr_write

; Pre-5705 parts insist on a statistics block as well and on the NIC
; memory addresses of both blocks.
        cmp     [ebx + device.is_5705_plus], 0
        jne     .hcc_5705plus
        xor     edx, edx
        mov     eax, HCC_STATS_ADDR_HI
        call    csr_write
        mov     edx, [ebx + device.dma_phys]
        add     edx, DMA_STATS_OFS
        mov     eax, HCC_STATS_ADDR_LO
        call    csr_write
        mov     edx, STATS_BLOCK_NIC
        mov     eax, HCC_STATS_BASEADDR
        call    csr_write
        mov     edx, STATUS_BLOCK_NIC
        mov     eax, HCC_STATUSBLK_BASEADDR
        call    csr_write
        mov     edx, 1000000
        mov     eax, HCC_STATS_TICKS
        call    csr_write
  .hcc_5705plus:

        mov     edx, RX_COAL_TICKS
        mov     eax, HCC_RX_COAL_TICKS
        call    csr_write
        mov     edx, TX_COAL_TICKS
        mov     eax, HCC_TX_COAL_TICKS
        call    csr_write
        mov     edx, RX_MAX_COAL_BDS
        mov     eax, HCC_RX_MAX_COAL_BDS
        call    csr_write
        mov     edx, TX_MAX_COAL_BDS
        mov     eax, HCC_TX_MAX_COAL_BDS
        call    csr_write
        xor     edx, edx
        mov     eax, HCC_RX_COAL_TICKS_INT
        call    csr_write
        xor     edx, edx
        mov     eax, HCC_TX_COAL_TICKS_INT
        call    csr_write
        mov     edx, RX_MAX_COAL_BDS_INT
        mov     eax, HCC_RX_MAX_COAL_BDS_INT
        call    csr_write
        mov     edx, TX_MAX_COAL_BDS_INT
        mov     eax, HCC_TX_MAX_COAL_BDS_INT
        call    csr_write

        mov     edx, HCCMODE_ENABLE or HCCMODE_ATTN
        cmp     [ebx + device.is_5705_plus], 0
        je      @f
        or      edx, HCCMODE_STATBLKSZ_32BYTE
  @@:
        mov     eax, HCC_MODE
        call    csr_write

;-----------------------------------------------------------------------
; Turn the state machines on. Order matters: consumers first, so that
; nothing is produced into a block that is still halted.
;-----------------------------------------------------------------------
        mov     edx, RBDCMODE_ENABLE or RBDCMODE_ATTN
        mov     eax, RBDC_MODE
        call    csr_write

        mov     edx, RXLPMODE_ENABLE
        mov     eax, RXLP_MODE
        call    csr_write

        cmp     [ebx + device.is_5705_plus], 0
        jne     @f
        mov     edx, RXLSMODE_ENABLE
        mov     eax, RXLS_MODE
        call    csr_write
  @@:
        mov     edx, RDCMODE_ENABLE or RDCMODE_ATTN
        mov     eax, RDC_MODE
        call    csr_write

        mov     edx, RBDIMODE_ENABLE or RBDIMODE_ATTN
        mov     eax, RBDI_MODE
        call    csr_write

        mov     edx, RDBDIMODE_ENABLE or RDBDIMODE_ATTN
        mov     eax, RDBDI_MODE
        call    csr_write

        mov     edx, SBDCMODE_ENABLE
        mov     eax, SBDC_MODE
        call    csr_write

        mov     edx, SDCMODE_ENABLE
        mov     eax, SDC_MODE
        call    csr_write

        mov     edx, SDIMODE_ENABLE
        mov     eax, SDI_MODE
        call    csr_write

        mov     edx, SBDIMODE_ENABLE
        mov     eax, SBDI_MODE
        call    csr_write

        mov     edx, SRSMODE_ENABLE
        mov     eax, SRS_MODE
        call    csr_write

;-----------------------------------------------------------------------
; DMA engines, plus the errata workarounds for the later parts.
;-----------------------------------------------------------------------
        mov     edx, WDMAMODE_ENABLE or WDMAMODE_ALL_ATTNS
        cmp     [ebx + device.asic_rev], ASICREV_BCM57780
        jne     @f
        or      edx, WDMAMODE_STATUS_TAG_FIX
  @@:
        mov     eax, WDMA_MODE
        call    csr_write
        mov     ecx, 40
        call    delay_us

        mov     edx, RDMAMODE_ENABLE or RDMAMODE_ALL_ATTNS
        cmp     [ebx + device.asic_rev], ASICREV_BCM57780
        jne     @f
        or      edx, RDMAMODE_BD_SBD_CRPT_ATTN or RDMAMODE_MBUF_RBD_CRPT_ATTN \
                  or RDMAMODE_MBUF_SBD_CRPT_ATTN or RDMAMODE_FIFO_LONG_BURST
  @@:
        mov     eax, RDMA_MODE
        call    csr_write
        mov     ecx, 40
        call    delay_us

        cmp     [ebx + device.asic_rev], ASICREV_BCM57780
        jne     @f
        mov     eax, RDMA_RSRVCTRL
        call    csr_read
        or      eax, RDMA_RSRVCTRL_FIFO_OFLW_FIX
        mov     edx, eax
        mov     eax, RDMA_RSRVCTRL
        call    csr_write
        DEBUGF  2,"57780 DMA workarounds applied\n"
  @@:

; Interrupt on attention.
        mov     eax, MISC_LOCAL_CTL
        call    csr_read
        or      eax, MLC_INTR_ONATTN
        mov     edx, eax
        mov     eax, MISC_LOCAL_CTL
        call    csr_write

; Host stack up, host owns the send descriptors, report attentions.
        mov     edx, MODECTL_BYTESWAP_DATA or MODECTL_WORDSWAP_DATA \
                  or MODECTL_WORDSWAP_NONFRAME \
                  or MODECTL_HOST_STACKUP or MODECTL_HOST_SENDBDS \
                  or MODECTL_MAC_ATTN_INTR or MODECTL_TX_ATTN_INTR \
                  or MODECTL_RX_ATTN_INTR or MODECTL_DMA_ATTN_INTR
; 5701 B5 corrupts data on 64 bit PCI, force 32 bit transfers.
        cmp     [ebx + device.chip_id], CHIPID_BCM5701_B5
        jne     @f
        or      edx, MODECTL_FORCE_PCI32
        DEBUGF  2,"5701 B5, forcing 32 bit PCI transfers\n"
  @@:
        mov     eax, MODE_CTRL
        call    csr_write

        DEBUGF  2,"State machines enabled\n"
        xor     eax, eax
        ret


;***************************************************************************
;  mac_enable - start the MAC once the rings are ready
;***************************************************************************

mac_enable:

        mov     edx, MAC_MODE_PORT_MODE_GMII \
                  or MAC_MODE_TXDMA_EN or MAC_MODE_RXDMA_EN \
                  or MAC_MODE_FRMHDR_DMA_EN \
                  or MAC_MODE_RX_STATS_EN or MAC_MODE_TX_STATS_EN \
                  or MAC_MODE_RX_STATS_CLEAR or MAC_MODE_TX_STATS_CLEAR
        mov     eax, MAC_MODE
        call    csr_write
        mov     ecx, 40
        call    delay_us

; Auto-poll the PHY so link changes raise an attention on their own.
        mov     edx, MIMODE_CLK_10MHZ or MIMODE_AUTOPOLL or MIMODE_PHYADDR_1
        mov     eax, MI_MODE
        call    csr_write
        mov     ecx, 80
        call    delay_us

        mov     edx, TX_MODE_ENABLE
        cmp     [ebx + device.asic_rev], ASICREV_BCM57780
        jne     @f
        or      edx, TXMODE_MBUF_LOCKUP_FIX
  @@:
        mov     eax, MAC_TX_MODE
        call    csr_write
        mov     ecx, 40
        call    delay_us

        mov     edx, RX_MODE_ENABLE
        mov     eax, MAC_RX_MODE
        call    csr_write
        mov     ecx, 40
        call    delay_us

; Clear the sticky attention bits and arm the link change event.
        mov     esi, [ebx + device.mmio_addr]
        mov     dword [esi + MAC_STS], MACSTAT_STICKY
        mov     eax, [esi + MAC_EVT_ENB]
        or      eax, EVTENB_LINK_CHANGED
        mov     [esi + MAC_EVT_ENB], eax

        DEBUGF  2,"MAC enabled, MAC_MODE 0x%x RX_MODE 0x%x TX_MODE 0x%x\n",\
        [esi + MAC_MODE], [esi + MAC_RX_MODE], [esi + MAC_TX_MODE]
        ret


;***************************************************************************
;  irq_unmask - let the chip drive INTA again
;***************************************************************************

irq_unmask:

        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL
        or      eax, PCIMISCCTL_CLEAR_INTA
        and     eax, not PCIMISCCTL_MASK_PCI_INTR
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL, eax
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MISC_CTL
        DEBUGF  2,"Interrupts unmasked, MISC_CTL 0x%x (bit1 must be 0)\n", eax

        xor     edx, edx
        mov     eax, MBX_IRQ0_LO
        call    mbx_write

        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_PCISTATE
        DEBUGF  2,"PCISTATE 0x%x (bit1 set means INTA is not asserted)\n", eax
        ret


;***************************************************************************
;  rx_refill - top the receive ring back up to RX_BUFFERS
;
;  Posting is deliberately batched: the producer mailbox is written once
;  at the end instead of once per descriptor.
;***************************************************************************

rx_refill:

        push    eax ecx edx esi edi

  .loop:
        cmp     [ebx + device.rx_posted], RX_BUFFERS
        jae     .publish

        invoke  NetAlloc, RX_BUF_SIZE + NET_BUFF.data
        test    eax, eax
        jnz     @f
        inc     [ebx + device.oom_count]
        mov     eax, [ebx + device.oom_count]
        call    dbg_worthy
        jc      .publish
        DEBUGF  2,"Out of net buffers (%u so far), only %u of %u posted\n",\
        [ebx + device.oom_count], [ebx + device.rx_posted], RX_BUFFERS
        jmp     .publish
  @@:

        mov     ecx, [ebx + device.rx_std_prod]
        mov     [ebx + device.rx_buffs + ecx*4], eax
        mov     [eax + NET_BUFF.device], ebx
        mov     [eax + NET_BUFF.offset], NET_BUFF.data

        push    ecx
        add     eax, NET_BUFF.data
        invoke  GetPhysAddr
        pop     ecx

        mov     edi, [ebx + device.rx_std]
        mov     edx, ecx
        shl     edx, RX_BD_SHIFT
        add     edi, edx

        mov     dword [edi + rx_bd.addr_hi], 0
        mov     [edi + rx_bd.addr_lo], eax
        mov     [edi + rx_bd.len], RX_BUF_SIZE
        mov     [edi + rx_bd.idx], cx           ; comes back in the return BD
        mov     [edi + rx_bd.flags], RXBDFLAG_END

        inc     ecx
        and     ecx, RX_STD_SIZE - 1
        mov     [ebx + device.rx_std_prod], ecx
        inc     [ebx + device.rx_posted]
        jmp     .loop

  .publish:
        mov     edx, [ebx + device.rx_std_prod]
        mov     eax, MBX_RX_STD_PROD_LO
        call    mbx_write

        pop     edi esi edx ecx eax
        ret


;***************************************************************************
;  free_buffers - hand every buffer we still own back to the kernel
;***************************************************************************

free_buffers:

        push    eax ecx esi

        xor     esi, esi
  .rx_loop:
        mov     eax, [ebx + device.rx_buffs + esi*4]
        test    eax, eax
        jz      .rx_next
        mov     dword [ebx + device.rx_buffs + esi*4], 0
        push    esi
        invoke  NetFree, eax
        pop     esi
  .rx_next:
        inc     esi
        cmp     esi, RX_STD_SIZE
        jb      .rx_loop

        xor     esi, esi
  .tx_loop:
        mov     eax, [ebx + device.tx_buffs + esi*4]
        test    eax, eax
        jz      .tx_next
        mov     dword [ebx + device.tx_buffs + esi*4], 0
        push    esi
        invoke  NetFree, eax
        pop     esi
  .tx_next:
        inc     esi
        cmp     esi, TX_RING_SIZE
        jb      .tx_loop

        mov     [ebx + device.rx_posted], 0
        pop     esi ecx eax
        ret


;***************************************************************************
;  tx_clean - release every send descriptor the chip is done with
;
;  Called both from the interrupt handler and from transmit(). Doing it
;  in transmit() as well means the send ring keeps draining even if the
;  completion interrupt is late, coalesced away or lost, which is the
;  difference between a slow transmitter and a dead one.
;
;  All registers preserved.
;***************************************************************************

tx_clean:

        push    eax ecx edx esi

        mov     esi, [ebx + device.status_blk]
        movzx   eax, [esi + status_block.tx_cons_idx]
        and     eax, TX_RING_SIZE - 1
        mov     ecx, [ebx + device.tx_cons]
  .loop:
        cmp     ecx, eax
        je      .done
        mov     edx, [ebx + device.tx_buffs + ecx*4]
        test    edx, edx
        jz      .next
        mov     dword [ebx + device.tx_buffs + ecx*4], 0
        push    eax ecx
        invoke  NetFree, edx
        pop     ecx eax
  .next:
        inc     ecx
        and     ecx, TX_RING_SIZE - 1
        jmp     .loop
  .done:
        mov     [ebx + device.tx_cons], ecx
        pop     esi edx ecx eax
        ret


;***************************************************************************
;  transmit
;***************************************************************************

align 16
proc transmit stdcall bufferptr

        spin_lock_irqsave

        mov     esi, [bufferptr]
        DEBUGF  1,"TX %u bytes\n", [esi + NET_BUFF.length]

        cmp     [esi + NET_BUFF.length], 1514
        ja      .error
        cmp     [esi + NET_BUFF.length], 60
        jb      .error

; Reclaim whatever the chip has already sent before deciding the ring
; is full.
        call    tx_clean

; Keep one slot free so that producer == consumer always means empty.
        mov     ecx, [ebx + device.tx_prod]
        mov     eax, ecx
        inc     eax
        and     eax, TX_RING_SIZE - 1
        cmp     eax, [ebx + device.tx_cons]
        je      .overflow

        mov     esi, [bufferptr]
        mov     [ebx + device.tx_buffs + ecx*4], esi

        push    ecx
        mov     eax, esi
        add     eax, [esi + NET_BUFF.offset]
        invoke  GetPhysAddr
        pop     ecx

        mov     edi, [ebx + device.tx_ring]
        mov     edx, ecx
        shl     edx, TX_BD_SHIFT
        add     edi, edx

        mov     esi, [bufferptr]
        mov     dword [edi + tx_bd.addr_hi], 0
        mov     [edi + tx_bd.addr_lo], eax
        mov     eax, [esi + NET_BUFF.length]
        shl     eax, 16
        or      eax, TXBDFLAG_END
        mov     [edi + tx_bd.len_flags], eax
        mov     dword [edi + tx_bd.vlan_tag], 0

        inc     ecx
        and     ecx, TX_RING_SIZE - 1
        mov     [ebx + device.tx_prod], ecx

        mov     edx, ecx
        mov     eax, MBX_TX_HOST_PROD0_LO
        call    mbx_write

        inc     [ebx + device.packets_tx]
        mov     eax, [esi + NET_BUFF.length]
        add     dword [ebx + device.bytes_tx], eax
        adc     dword [ebx + device.bytes_tx + 4], 0

        mov     eax, [ebx + device.packets_tx]
        call    dbg_worthy
        jc      @f
        push    esi
        mov     esi, [ebx + device.status_blk]
        movzx   ecx, [esi + status_block.tx_cons_idx]
        pop     esi
        DEBUGF  2,"TX %u: %u bytes, prod %u cons %u chip_cons %u\n",\
        [ebx + device.packets_tx], [esi + NET_BUFF.length],\
        [ebx + device.tx_prod], [ebx + device.tx_cons], ecx
  @@:
        spin_unlock_irqrestore
        xor     eax, eax
        ret

  .overflow:
        inc     [ebx + device.packets_tx_drop]
        inc     [ebx + device.txfull_count]
        mov     eax, [ebx + device.txfull_count]
        call    dbg_worthy
        jc      @f
; If the chip's own consumer index is stuck while ours keeps growing,
; the send engine has stopped fetching descriptors. That distinction is
; the whole point of printing all three.
        mov     esi, [ebx + device.status_blk]
        movzx   ecx, [esi + status_block.tx_cons_idx]
        mov     edx, [esi + status_block.status_word]
        DEBUGF  2,"Send ring full (%u): prod %u cons %u chip_cons %u status 0x%x\n",\
        [ebx + device.txfull_count], [ebx + device.tx_prod],\
        [ebx + device.tx_cons], ecx, edx
; The very first time this happens, say everything there is to say.
        cmp     [ebx + device.txfull_count], 1
        jne     @f
        call    dump_state
  @@:
        invoke  NetFree, [bufferptr]
        spin_unlock_irqrestore
        or      eax, -1
        ret

  .error:
        DEBUGF  2,"Refusing a %u byte frame\n", [esi + NET_BUFF.length]
        inc     [ebx + device.packets_tx_err]
        invoke  NetFree, [bufferptr]
        spin_unlock_irqrestore
        or      eax, -1
        ret

endp


;***************************************************************************
;  int_handler
;***************************************************************************

align 16
int_handler:

        push    ebx esi edi ecx edx
        mov     ebx, [esp + 4*6]

        mov     esi, [ebx + device.mmio_addr]
        test    esi, esi
        jz      .not_ours
        mov     edi, [ebx + device.status_blk]
        test    edi, edi
        jz      .not_ours

; Stop the chip from raising further interrupts while we work. This also
; deasserts INTA, so it has to happen before we decide whether the
; interrupt was ours at all.
        mov     edx, 1
        mov     eax, MBX_IRQ0_LO
        call    mbx_write

        xor     ecx, ecx                        ; work found?

        mov     eax, [edi + status_block.status_word]
        test    eax, STATUSFLAG_UPDATED
        jz      @f
        mov     dword [edi + status_block.status_word], 0
        inc     ecx
  @@:
        push    eax                             ; status word, for the log
        mov     eax, [esi + MAC_STS]
        test    eax, MACSTAT_LINK_CHANGED
        jz      @f
        mov     dword [esi + MAC_STS], MACSTAT_STICKY
        call    check_link
        inc     ecx
  @@:
        pop     eax

        inc     [ebx + device.irq_count]
        push    ecx
        push    eax
        mov     eax, [ebx + device.irq_count]
        call    dbg_worthy
        pop     eax
        jc      @f
        movzx   edx, [edi + status_block.rx_prod_idx]
        DEBUGF  2,"IRQ %u: status 0x%x rx_prod %u tx_cons %u ret_cons %u posted %u\n",\
        [ebx + device.irq_count], eax, edx,\
        [edi + status_block.tx_cons_idx]:2, [ebx + device.rx_ret_cons],\
        [ebx + device.rx_posted]
  @@:
        pop     ecx

        test    ecx, ecx
        jz      .idle

        call    tx_clean

;----- harvest received frames -----------------------------------------
  .rx_loop:
        mov     edi, [ebx + device.status_blk]
        movzx   eax, [edi + status_block.rx_prod_idx]
        and     eax, [ebx + device.rx_ret_mask]
        mov     ecx, [ebx + device.rx_ret_cons]
        cmp     eax, ecx
        je      .rx_done

        mov     esi, [ebx + device.rx_ret]
        shl     ecx, RX_BD_SHIFT
        add     esi, ecx                        ; esi -> return descriptor

        mov     ecx, [ebx + device.rx_ret_cons]
        inc     ecx
        and     ecx, [ebx + device.rx_ret_mask]
        mov     [ebx + device.rx_ret_cons], ecx

; The chip copies our opaque index back, so we always know which
; producer slot the buffer came from.
        movzx   edx, [esi + rx_bd.idx]
        and     edx, RX_STD_SIZE - 1
        mov     eax, [ebx + device.rx_buffs + edx*4]
        test    eax, eax
        jnz     @f
        DEBUGF  1,"RX return slot points at empty producer slot %u\n", edx
        jmp     .rx_loop
  @@:
        mov     dword [ebx + device.rx_buffs + edx*4], 0
        dec     [ebx + device.rx_posted]

        movzx   ecx, [esi + rx_bd.flags]
        test    ecx, RXBDFLAG_ERROR
        jnz     .rx_bad

        movzx   edx, [esi + rx_bd.len]
        cmp     edx, 64
        jb      .rx_bad
        cmp     edx, 1518
        ja      .rx_bad
        sub     edx, 4                          ; drop the FCS

        mov     esi, eax
        mov     [esi + NET_BUFF.length], edx
        mov     [esi + NET_BUFF.device], ebx
        mov     [esi + NET_BUFF.offset], NET_BUFF.data

        inc     [ebx + device.packets_rx]
        add     dword [ebx + device.bytes_rx], edx
        adc     dword [ebx + device.bytes_rx + 4], 0

        push    edx
        mov     eax, [ebx + device.packets_rx]
        call    dbg_worthy
        pop     edx
        jc      @f
        DEBUGF  2,"RX %u: %u bytes, ret_cons %u posted %u\n",\
        [ebx + device.packets_rx], edx, [ebx + device.rx_ret_cons],\
        [ebx + device.rx_posted]
  @@:
        DEBUGF  1,"RX %u bytes\n", edx

        push    ebx
        push    .rx_next
        push    esi
        jmp     [EthInput]
  .rx_next:
        pop     ebx
        jmp     .rx_loop

  .rx_bad:
        inc     [ebx + device.packets_rx_err]
        push    eax
        mov     eax, [ebx + device.packets_rx_err]
        call    dbg_worthy
        pop     eax
        jc      @f
        movzx   ecx, [esi + rx_bd.flags]
        movzx   edx, [esi + rx_bd.len]
        DEBUGF  2,"Bad frame, flags 0x%x len %u\n", ecx, edx
  @@:
        invoke  NetFree, eax
        jmp     .rx_loop

  .rx_done:
; Both mailbox updates happen once per interrupt, not once per frame.
        call    rx_refill
        mov     edx, [ebx + device.rx_ret_cons]
        mov     eax, MBX_RX_CONS0_LO
        call    mbx_write

        xor     edx, edx
        mov     eax, MBX_IRQ0_LO
        call    mbx_write

        mov     eax, 1
        pop     edx ecx edi esi ebx
        ret

  .idle:
        xor     edx, edx
        mov     eax, MBX_IRQ0_LO
        call    mbx_write
  .not_ours:
        xor     eax, eax
        pop     edx ecx edi esi ebx
        ret


;***************************************************************************
;  check_link - read the negotiated speed and tell the stack
;
;  All registers preserved.
;***************************************************************************

check_link:

        push    eax ebx ecx edx esi edi

        mov     esi, [ebx + device.mmio_addr]

; Ask the PHY directly rather than looking at MI_STS. MI_STS only ever
; changes when the MI auto-poller has actually run, so straight after
; bring-up it still reads whatever it held before and the first link
; check comes out wrong.
        mov     eax, PHY_BMSR
        call    phy_read
        cmp     eax, -1
        je      .fallback
        mov     edi, eax
        mov     eax, PHY_BMSR                   ; latching bits, read twice
        call    phy_read
        cmp     eax, -1
        je      .fallback
        DEBUGF  1,"check_link: BMSR 0x%x/0x%x MI_STS 0x%x\n",\
        edi, eax, [esi + MI_STS]
        test    eax, PHY_BMSR_LINK
        jz      .down

; The PHY reports carrier before auto-negotiation finishes, and the
; auxiliary status register holds nonsense until it does. Reading it too
; early is what turns a gigabit link into a reported 10 Mbit half duplex
; one and makes the link look like it is flapping.
        test    eax, PHY_BMSR_AUTONEG_COMP
        jnz     @f
        DEBUGF  2,"Link is up but auto-negotiation is still running, BMSR 0x%x\n", eax
        jmp     .leave                          ; another attention will follow
  @@:
        mov     eax, PHY_AUX_STAT
        call    phy_read
        cmp     eax, -1
        je      .fallback
        DEBUGF  1,"check_link: AUX_STAT 0x%x\n", eax

        and     eax, AUX_SPEED_MASK
        cmp     eax, AUX_SPEED_10HALF
        je      .s10h
        cmp     eax, AUX_SPEED_10FULL
        je      .s10f
        cmp     eax, AUX_SPEED_100HALF
        je      .s100h
        cmp     eax, AUX_SPEED_100T4
        je      .s100h
        cmp     eax, AUX_SPEED_100FULL
        je      .s100f
        cmp     eax, AUX_SPEED_1000HALF
        je      .s1000h
        cmp     eax, AUX_SPEED_1000FULL
        je      .s1000f
        DEBUGF  2,"Unknown speed code 0x%x in AUX_STAT\n", eax
        jmp     .fallback

  .s10h:
        mov     ecx, ETH_LINK_SPEED_10M
        mov     edx, MAC_MODE_PORT_MODE_MII
        jmp     .apply
  .s10f:
        mov     ecx, ETH_LINK_SPEED_10M or ETH_LINK_FULL_DUPLEX
        mov     edx, MAC_MODE_PORT_MODE_MII
        jmp     .apply
  .s100h:
        mov     ecx, ETH_LINK_SPEED_100M
        mov     edx, MAC_MODE_PORT_MODE_MII
        jmp     .apply
  .s100f:
        mov     ecx, ETH_LINK_SPEED_100M or ETH_LINK_FULL_DUPLEX
        mov     edx, MAC_MODE_PORT_MODE_MII
        jmp     .apply
  .s1000h:
        mov     ecx, ETH_LINK_SPEED_1G
        mov     edx, MAC_MODE_PORT_MODE_GMII
        jmp     .apply
  .s1000f:
        mov     ecx, ETH_LINK_SPEED_1G or ETH_LINK_FULL_DUPLEX
        mov     edx, MAC_MODE_PORT_MODE_GMII
        jmp     .apply

; The PHY did not answer. The link is up, so guess from the only other
; hint the MAC gives us.
  .fallback:
        mov     eax, [esi + MI_STS]
        test    eax, MISTS_LINK
        jz      .down
        mov     ecx, ETH_LINK_SPEED_100M or ETH_LINK_FULL_DUPLEX
        test    eax, MISTS_10MBPS
        jz      @f
        mov     ecx, ETH_LINK_SPEED_10M or ETH_LINK_FULL_DUPLEX
  @@:
        mov     edx, MAC_MODE_PORT_MODE_MII
        DEBUGF  2,"PHY unreadable, assuming state 0x%x\n", ecx

  .apply:
        mov     eax, [esi + MAC_MODE]
        and     eax, not (MAC_MODE_PORT_MODE_MASK or MAC_MODE_HALF_DUPLEX)
        or      eax, edx
        test    ecx, ETH_LINK_FULL_DUPLEX
        jnz     @f
        or      eax, MAC_MODE_HALF_DUPLEX
  @@:
        mov     [esi + MAC_MODE], eax

        cmp     [ebx + device.state], ecx
        je      .leave
        mov     [ebx + device.state], ecx
        DEBUGF  2,"Link up, state 0x%x, MAC_MODE 0x%x\n", ecx, eax
        invoke  NetLinkChanged
  .leave:
        pop     edi esi edx ecx ebx eax
        ret

  .down:
        cmp     [ebx + device.state], ETH_LINK_DOWN
        je      .leave
        mov     [ebx + device.state], ETH_LINK_DOWN
        DEBUGF  2,"Link down, prod %u cons %u\n",\
        [ebx + device.tx_prod], [ebx + device.tx_cons]

; Anything still queued for transmission is never going to complete now
; that the carrier is gone, and the chip will not advance its consumer
; index past it either. Without reclaiming those slots by hand the send
; ring shrinks on every link flap until it is permanently full.
        call    tx_reclaim_all

        invoke  NetLinkChanged
        pop     edi esi edx ecx ebx eax
        ret


;***************************************************************************
;  tx_reclaim_all - give up on the whole send ring
;
;  Only safe while the carrier is down, when the send engine cannot be
;  reading a descriptor. All registers preserved.
;***************************************************************************

tx_reclaim_all:

        push    eax ecx edx

        xor     ecx, ecx
  .loop:
        mov     edx, [ebx + device.tx_buffs + ecx*4]
        test    edx, edx
        jz      .next
        mov     dword [ebx + device.tx_buffs + ecx*4], 0
        inc     [ebx + device.packets_tx_drop]
        push    ecx
        invoke  NetFree, edx
        pop     ecx
  .next:
        inc     ecx
        cmp     ecx, TX_RING_SIZE
        jb      .loop

        mov     eax, [ebx + device.tx_prod]
        mov     [ebx + device.tx_cons], eax

        pop     edx ecx eax
        ret


;***************************************************************************
;  phy_read - read one PHY register through the MI interface
;
;  in:  eax = register number
;  out: eax = value, or -1 on failure
;***************************************************************************

phy_read:

        push    ebx ecx edx esi edi

        mov     esi, [ebx + device.mmio_addr]

; Auto-polling and manual access cannot be used at the same time.
        mov     edi, [esi + MI_MODE]
        test    edi, MIMODE_AUTOPOLL
        jz      @f
        mov     edx, edi
        and     edx, not MIMODE_AUTOPOLL
        mov     [esi + MI_MODE], edx
        mov     ecx, 80
        call    delay_us
  @@:
        and     eax, 0x1f
        shl     eax, MICOMM_REG_SHIFT
        or      eax, MICOMM_CMD_READ or MICOMM_BUSY or (PHY_ADDR shl MICOMM_PHY_SHIFT)
        mov     [esi + MI_COMM], eax

        mov     ecx, 5000
  .wait:
        mov     eax, [esi + MI_COMM]
        test    eax, MICOMM_BUSY
        jz      .ready
        dec     ecx
        jnz     .wait
        DEBUGF  1,"phy_read: timeout, MI_COMM 0x%x\n", eax
        or      eax, -1
        jmp     .restore

  .ready:
        test    eax, MICOMM_READFAIL
        jz      @f
        DEBUGF  1,"phy_read: read failed, MI_COMM 0x%x\n", eax
        or      eax, -1
        jmp     .restore
  @@:
        and     eax, MICOMM_DATA_MASK

  .restore:
        test    edi, MIMODE_AUTOPOLL
        jz      @f
        push    eax
        mov     [esi + MI_MODE], edi
        mov     ecx, 80
        call    delay_us
        pop     eax
  @@:
        pop     edi esi edx ecx ebx
        ret


;***************************************************************************
;  Low level register access
;
;  All CSR access goes through the memory mapped window. The indirect
;  PCI configuration path costs several microseconds per access and is
;  only needed for NIC internal memory.
;***************************************************************************

; csr_write: eax = register offset, edx = value. All registers preserved.
csr_write:
        push    eax esi
        mov     esi, [ebx + device.mmio_addr]
        mov     [esi + eax], edx
        pop     esi eax
        ret

; csr_read: eax = register offset in, value out. Other registers preserved.
csr_read:
        push    esi
        mov     esi, [ebx + device.mmio_addr]
        mov     eax, [esi + eax]
        pop     esi
        ret

; mbx_write: eax = mailbox offset, edx = value. All registers preserved.
;
; The read-back is not cosmetic. Mailbox writes are posted, and several
; parts in this family need the flush before the chip acts on the new
; producer index; without it a send can sit in the ring until the next
; unrelated write pushes it out.
mbx_write:
        push    eax esi
        mov     esi, [ebx + device.mmio_addr]
        mov     [esi + eax], edx
        mov     eax, [esi + eax]
        pop     esi eax
        ret

; mem_write: eax = NIC internal memory offset, edx = value.
; All registers preserved. Only used during bring-up.
mem_write:
        push    eax ecx edx
        push    edx
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MEMWIN_BASEADDR, eax
        pop     eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MEMWIN_DATA, eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MEMWIN_BASEADDR, 0
        pop     edx ecx eax
        ret

; mem_read: eax = NIC internal memory offset in, value out.
mem_read:
        push    ecx edx
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MEMWIN_BASEADDR, eax
        invoke  PciRead32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MEMWIN_DATA
        push    eax
        invoke  PciWrite32, [ebx + device.pci_bus], [ebx + device.pci_dev], PCI_MEMWIN_BASEADDR, 0
        pop     eax
        pop     edx ecx
        ret

; delay_us: busy wait for roughly ecx microseconds. Every read of a chip
; register costs a full bus turnaround, which is around a microsecond on
; PCI and less on PCI Express, so this always waits at least as long as
; asked for. All registers preserved.
delay_us:
        push    eax ecx esi
        mov     esi, [ebx + device.mmio_addr]
        test    esi, esi
        jz      .done
        test    ecx, ecx
        jz      .done
  @@:
        mov     eax, [esi + MODE_CTRL]
        dec     ecx
        jnz     @b
  .done:
        pop     esi ecx eax
        ret


; End of code


data fixups
end data

include '../peimport.inc'

my_service      db 'BCM57XX',0

; Chip id -> marketing name, terminated by a null name pointer rather
; than a null id, because chip id 0x0000 is a perfectly valid entry
; (5701 A0).
;
; This table only chooses the string that ends up in the device list and
; in the log. Everything the driver actually does is keyed off the ASIC
; revision, so an unlisted chip is still driven - it just reports itself
; as a plain BCM57xx. Adding an entry is therefore safe; it does not
; claim the part works.
align 4
chip_table:
        dd CHIPID_BCM5700_A0,   name_5700
        dd CHIPID_BCM5700_B0,   name_5700
        dd CHIPID_BCM5700_B2,   name_5700
        dd CHIPID_BCM5700_B3,   name_5700
        dd CHIPID_BCM5700_ALTIMA, name_5700
        dd CHIPID_BCM5700_C0,   name_5700

        dd CHIPID_BCM5701_A0,   name_5701
        dd CHIPID_BCM5701_B0,   name_5701
        dd CHIPID_BCM5701_B2,   name_5701
        dd CHIPID_BCM5701_B5,   name_5701

        dd CHIPID_BCM5703_A0,   name_5703
        dd CHIPID_BCM5703_A1,   name_5703
        dd CHIPID_BCM5703_A2,   name_5703
        dd CHIPID_BCM5703_A3,   name_5703
        dd CHIPID_BCM5703_B0,   name_5703

        dd CHIPID_BCM5704_A0,   name_5704
        dd CHIPID_BCM5704_A1,   name_5704
        dd CHIPID_BCM5704_A2,   name_5704
        dd CHIPID_BCM5704_A3,   name_5704
        dd CHIPID_BCM5704_B0,   name_5704

        dd CHIPID_BCM5705_A0,   name_5705
        dd CHIPID_BCM5705_A1,   name_5705
        dd CHIPID_BCM5705_A2,   name_5705
        dd CHIPID_BCM5705_A3,   name_5705

        dd CHIPID_BCM5750_A0,   name_5751
        dd CHIPID_BCM5750_A1,   name_5751
        dd CHIPID_BCM5750_B0,   name_5751
        dd CHIPID_BCM5750_B1,   name_5751
        dd CHIPID_BCM5750_C0,   name_5751
        dd CHIPID_BCM5750_C1,   name_5751
        dd CHIPID_BCM5750_C2,   name_5751

        dd CHIPID_BCM57780_A0,  name_57780
        dd CHIPID_BCM57780_A1,  name_57780

; Later members of the family. Their chip ids are in bcm57xx.inc, also
; commented out. None of these has been tried with this driver and
; several need setup it does not do: clock power management on the
; 5784/5761 class, the PHY workarounds and the shifted mailbox window on
; the 5906, jumbo frames on the 5714/5780. Uncomment the ids in the
; header and the lines below together, once one has actually been
; tested on hardware.
;
;       dd CHIPID_BCM5714_A0,   name_5714
;       dd CHIPID_BCM5714_B0,   name_5714
;       dd CHIPID_BCM5714_B3,   name_5714
;       dd CHIPID_BCM5715_A0,   name_5715
;       dd CHIPID_BCM5715_A1,   name_5715
;       dd CHIPID_BCM5715_A3,   name_5715
;       dd CHIPID_BCM5752_A0,   name_5752
;       dd CHIPID_BCM5752_A1,   name_5752
;       dd CHIPID_BCM5752_A2,   name_5752
;       dd CHIPID_BCM5755_A0,   name_5755
;       dd CHIPID_BCM5755_A1,   name_5755
;       dd CHIPID_BCM5755_A2,   name_5755
;       dd CHIPID_BCM5722_A0,   name_5722
;       dd CHIPID_BCM5787_A0,   name_5787
;       dd CHIPID_BCM5787_A1,   name_5787
;       dd CHIPID_BCM5787_A2,   name_5787
;       dd CHIPID_BCM5906_A1,   name_5906
;       dd CHIPID_BCM5906_A2,   name_5906

        dd 0, 0

name_5700       db 'BCM5700',0
name_5701       db 'BCM5701',0
name_5703       db 'BCM5703',0
name_5704       db 'BCM5704',0
name_5705       db 'BCM5705',0
name_5751       db 'BCM5750/5751',0
name_57780      db 'BCM57780',0
name_unknown    db 'BCM57xx',0

; Names for the entries above that are still commented out.
;name_5714      db 'BCM5714',0
;name_5715      db 'BCM5715',0
;name_5752      db 'BCM5752',0
;name_5755      db 'BCM5755',0
;name_5722      db 'BCM5722',0
;name_5787      db 'BCM5787',0
;name_5906      db 'BCM5906',0

include_debug_strings

align 4
devices         dd 0
device_list     rd MAX_DEVICES
