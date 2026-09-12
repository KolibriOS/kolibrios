; Synopsys DesignWare I2C master, polled (PIO) mode only.
; This is the controller inside Intel LPSS blocks (Skylake and newer, plus
; Atom-derived SoCs); the same IP is used by many other vendors.
;
; The register file is always accessed as dwords. The controller must be
; disabled to change IC_TAR, so every transfer disables it, programs the
; target address, enables it back and feeds the TX FIFO by hand.

; ------------------------------ Registers ------------------------------------
DwIcCon             = 0x00
DwIcTar             = 0x04
DwIcDataCmd         = 0x10
DwIcSsSclHcnt       = 0x14
DwIcSsSclLcnt       = 0x18
DwIcFsSclHcnt       = 0x1C
DwIcFsSclLcnt       = 0x20
DwIcIntrStat        = 0x2C
DwIcIntrMask        = 0x30
DwIcRawIntrStat     = 0x34
DwIcRxTl            = 0x38
DwIcTxTl            = 0x3C
DwIcClrIntr         = 0x40
DwIcClrTxAbrt       = 0x54
DwIcEnable          = 0x6C
DwIcStatus          = 0x70
DwIcTxflr           = 0x74
DwIcRxflr           = 0x78
DwIcSdaHold         = 0x7C
DwIcTxAbrtSource    = 0x80
DwIcEnableStatus    = 0x9C
DwIcCompParam1      = 0xF4
DwIcCompVersion     = 0xF8
DwIcCompType        = 0xFC

; "DW_apb_i2c" component signature ('DW' + 0x0140).
DW_COMP_TYPE_VALUE  = 0x44570140

; IC_CON: master, standard speed (100 kHz), restarts allowed, slave disabled.
DW_CON_MASTER       = 1 shl 0
DW_CON_SPEED_STD    = 1 shl 1
DW_CON_SPEED_FAST   = 2 shl 1
DW_CON_RESTART_EN   = 1 shl 5
DW_CON_SLAVE_DIS    = 1 shl 6

; IC_DATA_CMD bits above the data byte.
DW_CMD_READ         = 1 shl 8
DW_CMD_STOP         = 1 shl 9
DW_CMD_RESTART      = 1 shl 10

; IC_STATUS bits
DW_STAT_TFNF        = 1 shl 1   ; TX FIFO not full
DW_STAT_TFE         = 1 shl 2   ; TX FIFO empty
DW_STAT_RFNE        = 1 shl 3   ; RX FIFO not empty
DW_STAT_MST_ACT     = 1 shl 5   ; master FSM busy

; IC_RAW_INTR_STAT bits
DW_INT_TX_ABRT      = 1 shl 6

; SCL timing for standard mode. The LPSS input clock is 100 MHz on the PCH
; parts and 133 MHz on the Atom-derived SoCs (Apollo Lake, Gemini Lake), and
; nothing in PCI config space says which one this is, so the counts are
; picked to satisfy the fast-mode minimums at the fastest of them: at
; 133 MHz that is tHIGH = 0.98 us and tLOW = 1.5 us, 400 kHz. On a 100 MHz
; part the same counts run the bus at 300 kHz, which is just as legal.
; Fast mode rather than standard: HID over I2C requires every device to
; support it, that is what Windows drives them at, and a polled host pays
; for every byte with busy-waiting - at 100 kHz a 64-byte input read costs
; 6 ms, more than a fast pad takes to produce the next report, and the
; backlog that builds up plays back as the cursor coasting after the finger
; has stopped. SDA hold stays well below tLOW.
DW_FS_HCNT          = 130
DW_FS_LCNT          = 200
DW_SDA_HOLD         = 28
; The AMD FCH feeds its DesignWare masters from a 150 MHz reference, so
; the same minimums need more counts there: 145/225 give tHIGH = 0.97 us,
; tLOW = 1.5 us, again 400 kHz; SDA hold 45 = 300 ns.
DW_AMD_FS_HCNT      = 145
DW_AMD_FS_LCNT      = 225
DW_AMD_SDA_HOLD     = 45

; A transfer gives up after [XferTimeout] hundredths of a second. The init
; value must cover a full report descriptor read: descriptors run up to 2 KB
; and the bus may be as slow as 77 kHz, which is ~240 ms of clock time for a
; single transfer. Polling is generous too now that it runs in its own thread
; rather than in a timer callback - a wedged bus costs this thread its slice
; and nothing else.
DW_TIMEOUT_INIT     = 60
DW_TIMEOUT_POLL     = 20

; Disables the controller and waits until it reports itself disabled.
; in: ebx = MMIO base. Clobbers eax, ecx.
proc dw_ctrl_disable
        mov     dword [ebx+DwIcEnable], 0
        mov     ecx, 100000
@@:
        test    dword [ebx+DwIcEnableStatus], 1
        jz      .done
        pause
        loop    @b
.done:
        ret
endp

; Aborts a stuck transfer cleanly: the ABORT bit makes the master issue STOP,
; flush the TX FIFO and raise TX_ABRT, which releases a slave that is holding
; the bus (a clock-stretching touchpad, for instance). Simply disabling the
; controller in the middle of a byte would leave SDA held low and every
; following transfer would time out too. Older cores without the ABORT bit
; just ignore the write and fall through after the wait.
; in: ebx = MMIO base. Clobbers eax, ecx.
proc dw_ctrl_abort
        mov     dword [ebx+DwIcEnable], 3
        mov     ecx, 100000
@@:
        test    dword [ebx+DwIcRawIntrStat], DW_INT_TX_ABRT
        jnz     @f
        pause
        loop    @b
@@:
        mov     eax, [ebx+DwIcClrTxAbrt]
        call    dw_ctrl_disable
        ret
endp

; One-time controller setup after mapping.
; in: ebx = MMIO base.
proc dw_ctrl_init
        push    eax ecx
        call    dw_ctrl_disable
        mov     dword [ebx+DwIcCon], DW_CON_MASTER + DW_CON_SPEED_FAST + \
                DW_CON_RESTART_EN + DW_CON_SLAVE_DIS
; The counts come from variables: they depend on the reference clock, which
; is decided by whoever found the controller (PCI LPSS or AMD FCH). The
; standard-mode registers get the same values scaled up so that they are
; sane should anything ever switch the speed bits back.
        mov     eax, [DwHcnt]
        mov     [ebx+DwIcFsSclHcnt], eax
        lea     eax, [eax*4]
        mov     [ebx+DwIcSsSclHcnt], eax
        mov     eax, [DwLcnt]
        mov     [ebx+DwIcFsSclLcnt], eax
        lea     eax, [eax*4]
        mov     [ebx+DwIcSsSclLcnt], eax
        mov     eax, [DwSdaHold]
        mov     [ebx+DwIcSdaHold], eax
        mov     dword [ebx+DwIcIntrMask], 0
        mov     dword [ebx+DwIcRxTl], 0
        mov     dword [ebx+DwIcTxTl], 0
        mov     eax, [ebx+DwIcClrIntr]
        pop     ecx eax
        ret
endp

; One combined I2C transaction: optional write phase, then optional read phase
; with a repeated START between them. Fully polled.
; out: eax = 0 on success, IC_TX_ABRT_SOURCE value on NAK/abort, -1 on timeout.
proc dw_xfer stdcall uses ebx esi edi, addr:dword, wbuf:dword, wlen:dword, \
        rbuf:dword, rlen:dword
locals
        wsent   dd ?
        rqueued dd ?
        rgot    dd ?
        t0      dd ?
endl
        mov     ebx, [I2cMmio]
        call    dw_ctrl_disable
        mov     eax, [addr]
        mov     [ebx+DwIcTar], eax
        mov     eax, [ebx+DwIcClrIntr]
        mov     dword [ebx+DwIcEnable], 1
        xor     eax, eax
        mov     [wsent], eax
        mov     [rqueued], eax
        mov     [rgot], eax
        invoke  GetTimerTicks
        mov     [t0], eax
.loop:
; Abort (NAK and friends) ends the transfer immediately.
        test    dword [ebx+DwIcRawIntrStat], DW_INT_TX_ABRT
        jnz     .abort
; Drain the RX FIFO first so it can never overflow.
        test    dword [ebx+DwIcStatus], DW_STAT_RFNE
        jz      .no_rx
        mov     edx, [rgot]
        cmp     edx, [rlen]
        jae     .no_rx
        mov     eax, [ebx+DwIcDataCmd]
        mov     edi, [rbuf]
        mov     [edi+edx], al
        inc     edx
        mov     [rgot], edx
        jmp     .loop
.no_rx:
; Feed the write phase.
        mov     eax, [wsent]
        cmp     eax, [wlen]
        jae     .push_reads
        test    dword [ebx+DwIcStatus], DW_STAT_TFNF
        jz      .check_done
        mov     esi, [wbuf]
        movzx   edx, byte [esi+eax]
        inc     eax
        cmp     eax, [wlen]
        jne     @f
        cmp     [rlen], 0
        jne     @f
        or      edx, DW_CMD_STOP        ; last byte of a write-only transfer
@@:
        mov     [ebx+DwIcDataCmd], edx
        mov     [wsent], eax
        jmp     .loop
.push_reads:
; Feed read commands, at most 8 outstanding so the RX FIFO keeps up.
        mov     eax, [rqueued]
        cmp     eax, [rlen]
        jae     .check_done
        mov     edx, eax
        sub     edx, [rgot]
        cmp     edx, 8
        jae     .check_done
        test    dword [ebx+DwIcStatus], DW_STAT_TFNF
        jz      .check_done
        mov     edx, DW_CMD_READ
        test    eax, eax
        jnz     @f
        cmp     [wlen], 0
        je      @f
        or      edx, DW_CMD_RESTART     ; repeated START after the write phase
@@:
        mov     ecx, [rlen]
        dec     ecx
        cmp     eax, ecx
        jne     @f
        or      edx, DW_CMD_STOP
@@:
        mov     [ebx+DwIcDataCmd], edx
        inc     eax
        mov     [rqueued], eax
        jmp     .loop
.check_done:
        mov     eax, [rgot]
        cmp     eax, [rlen]
        jne     .not_done
        mov     eax, [wsent]
        cmp     eax, [wlen]
        jne     .not_done
        mov     eax, [ebx+DwIcStatus]
        test    eax, DW_STAT_TFE
        jz      .not_done
        test    eax, DW_STAT_MST_ACT
        jnz     .not_done
        call    dw_ctrl_disable
        xor     eax, eax
        ret
.not_done:
        invoke  GetTimerTicks
        sub     eax, [t0]
        cmp     eax, [XferTimeout]
        jae     .timeout
        pause
        jmp     .loop
.abort:
        mov     esi, [ebx+DwIcTxAbrtSource]
        mov     eax, [ebx+DwIcClrTxAbrt]
        call    dw_ctrl_disable
        mov     eax, esi
        test    eax, eax
        jnz     @f
        or      eax, 0x80000000
@@:
        ret
.timeout:
        call    dw_ctrl_abort
        or      eax, -1
        ret
endp

; Reads one HID input report as a single continuous bus transaction, the way
; interrupt-driven hosts (Windows, Linux) do it: the optional register-address
; write and every read command are queued into the TX FIFO before the bytes
; start flowing, so nothing on the wire depends on how fast the host loop
; runs. The previous adaptive read (fetch 2 length bytes, decide, fetch the
; rest, stop short) was cut down on the wire to 3 bytes for an empty poll -
; and the SIPODEV/Hantick touchpad firmwares are exactly the kind of thing
; that wedges when an input read is shorter than the frame they prepared.
; rlen should not exceed the TX FIFO depth (see DwTxDepth) or the queue loop
; degrades to feeding the FIFO on the fly, which reintroduces pauses.
; in: addr = slave, wlen bytes at wbuf written first (0 = plain read),
;     rlen bytes read into buf.
; out: eax = 0 ok, IC_TX_ABRT_SOURCE on NAK/abort, -1 on timeout.
; Timeout check for the dw_read_block wait spots; expands inside the proc so
; [t0] and [XferTimeout] resolve in the caller's own frame.
macro BLOCK_TICK fail_label {
        invoke  GetTimerTicks
        sub     eax, [t0]
        cmp     eax, [XferTimeout]
        jae     fail_label
        pause
}

proc dw_read_block stdcall uses ebx esi edi, addr:dword, wbuf:dword, wlen:dword,         buf:dword, rlen:dword
locals
        rgot    dd ?
        t0      dd ?
endl
        mov     ebx, [I2cMmio]
        call    dw_ctrl_disable
        mov     eax, [addr]
        mov     [ebx+DwIcTar], eax
        mov     eax, [ebx+DwIcClrIntr]
        mov     dword [ebx+DwIcEnable], 1
        invoke  GetTimerTicks
        mov     [t0], eax
        mov     [rgot], 0
; Write phase: the register address bytes, no STOP - the read continues the
; same transaction after a repeated START.
        xor     ecx, ecx
.wr:
        cmp     ecx, [wlen]
        jae     .queue_reads
        test    dword [ebx+DwIcRawIntrStat], DW_INT_TX_ABRT
        jnz     .abort
        test    dword [ebx+DwIcStatus], DW_STAT_TFNF
        jz      .wr_wait
        mov     esi, [wbuf]
        movzx   edx, byte [esi+ecx]
        mov     [ebx+DwIcDataCmd], edx
        inc     ecx
        jmp     .wr
.wr_wait:
        BLOCK_TICK .timeout
        jmp     .wr
.queue_reads:
        xor     ecx, ecx
.rq:
        cmp     ecx, [rlen]
        jae     .drain
        test    dword [ebx+DwIcRawIntrStat], DW_INT_TX_ABRT
        jnz     .abort
        test    dword [ebx+DwIcStatus], DW_STAT_TFNF
        jz      .rq_wait
        mov     edx, DW_CMD_READ
        test    ecx, ecx
        jnz     .not_first
        cmp     [wlen], 0
        je      .not_first
        or      edx, DW_CMD_RESTART
.not_first:
        mov     eax, [rlen]
        dec     eax
        cmp     ecx, eax
        jne     @f
        or      edx, DW_CMD_STOP
@@:
        mov     [ebx+DwIcDataCmd], edx
        inc     ecx
; Drain opportunistically so a FIFO shallower than rlen cannot deadlock the
; queue loop (the RX side fills while the TX side is full).
        test    dword [ebx+DwIcStatus], DW_STAT_RFNE
        jz      .rq
        push    ecx
        mov     edx, [rgot]
        mov     eax, [ebx+DwIcDataCmd]
        mov     edi, [buf]
        mov     [edi+edx], al
        inc     edx
        mov     [rgot], edx
        pop     ecx
        jmp     .rq
.rq_wait:
        BLOCK_TICK .timeout
        jmp     .rq
.drain:
        test    dword [ebx+DwIcRawIntrStat], DW_INT_TX_ABRT
        jnz     .abort
        test    dword [ebx+DwIcStatus], DW_STAT_RFNE
        jz      .chk
        mov     edx, [rgot]
        mov     eax, [ebx+DwIcDataCmd]
        mov     edi, [buf]
        mov     [edi+edx], al
        inc     edx
        mov     [rgot], edx
        jmp     .drain
.chk:
        mov     eax, [rgot]
        cmp     eax, [rlen]
        jne     .wait
        test    dword [ebx+DwIcStatus], DW_STAT_MST_ACT
        jnz     .wait
        call    dw_ctrl_disable
        xor     eax, eax
        ret
.wait:
        BLOCK_TICK .timeout
        jmp     .drain
.abort:
        mov     esi, [ebx+DwIcTxAbrtSource]
        mov     eax, [ebx+DwIcClrTxAbrt]
        call    dw_ctrl_disable
        mov     eax, esi
        test    eax, eax
        jnz     @f
        or      eax, 0x80000000
@@:
        ret
.timeout:
        call    dw_ctrl_abort
        or      eax, -1
        ret
endp
