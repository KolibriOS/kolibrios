; Performance tab: CPU, temperature when there is a sensor, memory and
; network history in rows, lines between them. A row: the icon, the title
; and the items on the left, the graph on the right.

GRAPH_STEP      = 4                     ; pixels per sample
INFO_W          = 256 + 2*PAD + ICON    ; the icon, the title, the items, PAD
NET_DEVICES     = 16                    ; of the kernel

struct G                                ; a graph
        title           dd ?
        hist            dd ?            ; dwords up to the top
        hist2           dd ?            ; 0, the network: received, sent
        info            dd ?            ; items left of the graph
        unit            dd ?            ; of the value in the title
        top             dd ?            ; -> the top of the scale
        on              dd ?            ; -> not 0 if shown
        icon            dd ?            ; before the title
ends

struct ITEM                             ; a line left of a graph
        text            dd ?
        get             dd ?            ; edi -> the text; out: edi after it
ends

struct NET_BYTES                        ; of all the devices
        rx              dq ?
        tx              dq ?
ends

perf_draw:                              ; eax = TAB.draw: with the frames,
        lea     esi, [eax - TAB.update] ; TAB.update: esi = 0, without
        mov     edi, shown              ; the graphs there is data for
        mov     eax, graphs

.shown:
        mov     edx, [eax + G.on]
        cmp     dword [edx], 0
        je      @f
        stosd
@@:
        add     eax, sizeof.G
        cmp     eax, graphs_end
        jb      .shown

        sub     edi, shown
        shr     edi, 2
        xchg    edi, [shown_n]
        cmp     edi, [shown_n]          ; one came or went: all again
        je      .draw
        test    esi, esi
        jz      draw_window

.draw:
        xor     ebp, ebp
.graph:
        call    graph_frame
        call    perf_graph
        inc     ebp
        cmp     ebp, [shown_n]
        jb      .graph
        ret

; The button of the bar.
perf_cpuid:
        mov     esi, sz_cpuid_path
        jmp     run_plain

; ebp = graph; out: ebx = x shl 16 + w, ecx = y shl 16 + h of its frame.
; Between the rows a line, PAD from both.
graph_frame:
        mov     eax, [area_h]
        dec     eax
        xor     edx, edx
        div     [shown_n]               ; from a row to the next one

        mov     ecx, ebp
        imul    ecx, eax
        add     ecx, AREA_Y + 1 + PAD
        shl     ecx, 16

        sub     eax, 2*PAD + 1
        mov     cx, ax

        mov     ebx, (3*PAD + 1 + INFO_W) shl 16
        mov     bx, word [area_w]
        sub     bx, 2 + 3*PAD + INFO_W
        ret

; ecx = y shl 16 of a frame: the line between its row and the one above
row_line:
        pusha
        shr     ecx, 16
        sub     ecx, PAD + 1            ; the middle of the gap above
        shl     ecx, 16
        inc     ecx

        mov     ebx, (PAD + 1) shl 16
        mov     bx, word [area_w]
        sub     bx, 2
        mcall   SF_DRAW_RECT, , , [sc.work_graph]
        popa
        ret

; ebp = graph, ebx and ecx = its frame, esi = 0: no frame
perf_graph:
        pusha
        test    esi, esi
        jz      .title
        mov     edx, [sc.work_light]
        call    frame_rect
        call    row_line                ; the first one on the panel border

.title:
        mov     ebp, [shown + ebp*4]
        mov     esi, [ebp + G.hist]
        mov     edx, [ebp + G.hist2]

        mov     edi, [hist_pos]         ; the last value
        mov     eax, [esi + edi*4]
        test    edx, edx
        jz      @f
        add     eax, [edx + edi*4]      ; the network: both ways
        mov     edi, text_buf
        call    fmt_rate
        jmp     .titled

@@:
        mov     edi, text_buf
        call    fmt_uint
        mov     eax, [ebp + G.unit]
        call    fmt_unit

.titled:
        call    info_lines              ; the lines left of it

        mov     eax, [ebp + G.top]      ; the top: 100 %, TjMax, the network:
        mov     eax, [eax]              ; the largest rate of the columns

        add     ebx, 1 shl 16 - 2       ; bars inside the frame, newest on the right
        add     ecx, 1 shl 16 - 2
        movzx   edi, bx

        test    edx, edx
        jz      .scaled
        call    net_scale

.scaled:
        mov     [scale], eax

        mov     ebp, [hist_pos]

.bar:                                   ; the network: received on the left,
        mov     eax, GRAPH_STEP         ; sent on the right, lighter
        cmp     edi, eax
        jae     @f
        mov     eax, edi

@@:
        mov     bx, ax
        sub     edi, eax
        pusha
        shl     edi, 16
        add     ebx, edi

        mov     edi, [sc.work_button]
        test    edx, edx
        jz      .last

        mov     eax, ebx
        shr     ax, 1
        sub     bx, ax
        call    .value
        shl     ebx, 16
        add     eax, ebx
        xchg    eax, ebx

        mov     esi, edx
        mov     eax, [sc.work_light]    ; sent: half way to the background
        mov     edx, edi
        call    mix
        xchg    eax, edi

.last:
        call    .value
        popa

        dec     ebp
        and     ebp, HIST_LEN - 1
        test    edi, edi
        jnz     .bar

        test    edx, edx                ; the network: its top at the top right
        jz      .critical
        mov     edi, text_buf
        mov     eax, [scale]
        call    fmt_rate
        mov     ebx, [area_w]
        shl     ebx, 16
        mov     bx, [esp + PUSHA_ECX + 2]       ; y of the frame
        add     ebx, ((-6 - INFO_W + ICON + PAD) shl 16) + 4
        call    info_value

@@:
        popa
        ret

.critical:                              ; the temperature: a line 10 below
        cmp     esi, hist_temp          ; the top
        jne     @b

        movzx   eax, cx
        imul    eax, 10
        div     [scale]                 ; edx = G.hist2 = 0 here
        shl     eax, 16
        add     ecx, eax
        mov     cx, 1

        mov     ebx, [esp + PUSHA_EBX]  ; the frame
        add     ebx, 1 shl 16 - 2
        mcall   SF_DRAW_RECT, , , [red]
        jmp     @b

; esi + ebp*4 -> value, edi = color: its bar at the bottom, the background
; above it, not one over the other, so it does not flicker
.value:
        pusha
        mov     eax, [esi + ebp*4]
        cmp     eax, [scale]            ; not over the top
        jb      @f
        mov     eax, [scale]

@@:
        movzx   esi, cx
        mul     esi
        div     [scale]
        xchg    ebp, eax
        sub     cx, bp
        mcall   SF_DRAW_RECT, , , [sc.work_light]

        movzx   edx, cx
        shl     edx, 16
        add     ecx, edx
        mov     cx, bp
        mcall   SF_DRAW_RECT, , , edi
        popa
        ret

; esi, edx -> the history received and sent, edi = the width of the graph;
; out: eax = its top, the largest rate shown up to 1, 2, 5, 10, 20, 50...
; bit/s while it fits
net_scale:
        push    edi
        mov     ebp, [hist_pos]
        xor     eax, eax

.max:
        cmp     eax, [esi + ebp*4]
        jae     @f
        mov     eax, [esi + ebp*4]
@@:
        cmp     eax, [edx + ebp*4]
        jae     @f
        mov     eax, [edx + ebp*4]

@@:
        dec     ebp
        and     ebp, HIST_LEN - 1
        sub     edi, GRAPH_STEP
        jg      .max
        pop     edi

        cmp     eax, 2000000000
        ja      .done

        xor     ebp, ebp
        inc     ebp
@@:
        cmp     ebp, eax
        jae     @f
        add     ebp, ebp
        cmp     ebp, eax
        jae     @f
        lea     ebp, [ebp*4 + ebp]
        shr     ebp, 1
        cmp     ebp, eax
        jae     @f
        add     ebp, ebp
        jmp     @b

@@:
        xchg    eax, ebp

.done:
        ret

; text_buf on the right of an item line at ebx = x shl 16 + y
info_value:
        pusha
        mov     ecx, [sc.work_text]
        mov     edx, text_buf
        mov     eax, edx
        call    text_width

        neg     eax
        add     eax, INFO_W - ICON - PAD
        shl     eax, 16
        add     ebx, eax

        call    draw_text
        popa
        ret

; ebp -> graph, ecx = y shl 16 + h of its frame, text_buf = its value:
; the title and the items a line each as fit, the label on the left, the
; value on the right. The ITEMs end with 0.
info_lines:
        pusha
        movzx   esi, cx                 ; the height left
        mov     ebx, ((2*PAD + 1) shl 16) + INFO_W
        mcall   SF_DRAW_RECT, , , [sc.work_light]

        mov     ebx, ecx                ; the icon, the lines after it
        shr     ebx, 16
        add     ebx, (2*PAD + 1) shl 16 - 1
        mov     eax, [ebp + G.icon]
        mov     edx, [sc.work_light]
        call    draw_icon
        add     ebx, ((ICON + PAD) shl 16) + 1

        mov     edx, [ebp + G.title]
        mov     ebp, [ebp + G.info]
        mov     ecx, [sc.work_text]
        jmp     .line

.item:
        mov     edx, [ebp + ITEM.text]
        test    edx, edx
        jz      .done

        mov     edi, text_buf
        pusha
        call    [ebp + ITEM.get]
        popa
        add     ebp, sizeof.ITEM

.line:
        sub     esi, 16
        jb      .done
        call    draw_text

        mov     eax, edx                ; the value in the rest of the line
        call    text_width

        pusha
        mov     ebx, INFO_W - 8 - ICON - PAD
        mov     edx, text_buf
        mov     edi, edx
        call    fmt_clip
        popa

        call    info_value
        add     ebx, 16
        jmp     .item

.done:
        popa
        ret

; Once a second: the bytes of the network devices, but the loopback.
net_sample:
        pusha
        mov     edi, net_sum
        xor     eax, eax
        mov     ecx, (net_last - net_sum) / 4   ; and net_up
        rep stosd

        mov     ebx, (NET_DEVICES - 1) shl 8

.device:
        push    ebx                     ; bl = SSF_DEVICE_TYPE
        mcall   SF_NETWORK_GET
        test    eax, eax                ; -1: none, 0: the loopback
        jle     .next

        inc     [net_up]
        mov     bl, SSF_RX_BYTE_COUNT
        mcall   SF_NETWORK_GET
        add     dword [net_sum.rx], eax
        adc     dword [net_sum.rx + 4], ebx

        mov     ebx, [esp]
        mov     bl, SSF_TX_BYTE_COUNT
        mcall   SF_NETWORK_GET
        add     dword [net_sum.tx], eax
        adc     dword [net_sum.tx + 4], ebx

.next:
        pop     ebx
        dec     bh
        jns     .device

        mov     ecx, [hist_pos]
        mov     esi, net_sum.rx
        mov     ebx, hist_rx
        call    .rate
        mov     ebx, hist_tx
        call    .rate

        or      [net_primed], -1
        popa
        ret

.rate:                                  ; since the last second, 0 at first
        mov     eax, [esi]
        mov     edx, [esi + 4]
        sub     eax, [edi]
        sbb     edx, [edi + 4]
        movsd                           ; the sum is the last one now
        movsd
        jae     @f
        xor     eax, eax                ; a device went away

@@:
        shl     eax, 3                  ; bits
        and     eax, [net_primed]
        mov     [ebx + ecx*4], eax
        ret

; The items of the lines left of the graphs: edi -> the text; out:
; edi after it.
get_freq:
        mcall   SF_SYSTEM, SSF_GET_CPU_FREQUENCY
        mov     ecx, 1000000
        call    fmt_quot
        mov     eax, sz_mhz
        jmp     fmt_unit

get_uptime:
        mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
        xor     edx, edx
        mov     ecx, 100
        div     ecx
        jmp     fmt_time

get_threads:
        mov     eax, [procs_end]
        sub     eax, procs
        mov     ecx, sizeof.PROC        ; then fmt_quot

; eax / ecx, edx = the remainder
fmt_quot:
        xor     edx, edx
        div     ecx
        jmp     fmt_uint

get_used:
        mov     eax, [ram_info.total]
        sub     eax, [ram_info.free]
        jmp     @f

get_free:
        mov     eax, [ram_info.free]
        jmp     @f

get_total:
        mov     eax, [ram_info.total]

@@:
        shl     eax, 2                  ; pages to KB
        jmp     fmt_size

get_rx:
        mov     esi, net_sum.rx
        mov     ebx, hist_rx
        jmp     @f

get_tx:
        mov     esi, net_sum.tx
        mov     ebx, hist_tx

@@:                                     ; the rate, then the total
        mov     eax, [hist_pos]
        mov     eax, [ebx + eax*4]
        call    fmt_rate
        mov     word [edi], " ("
        scasw

        mov     eax, [esi]
        mov     edx, [esi + 4]
        shrd    eax, edx, 10
        call    fmt_size
        mov     word [edi], ")"
        inc     edi
        ret

graphs:
        G       sz_col_cpu, hist_cpu, 0, cpu_info, sz_pct, pct_top, pct_top, 48
temp_graph G    sz_temp, hist_temp, 0, temp_info, sz_deg, tjmax, temp_on, 68
        G       sz_col_mem, hist_mem, 0, mem_info, sz_pct, pct_top, pct_top, 51
        G       sz_net, hist_rx, hist_tx, net_info, 0, pct_top, net_up, 12
graphs_end:
pct_top         dd      100
perf_bar:
        BAR     BAR_BUTTON, sz_cpuid_path + 5, sc.work_button   ; "CPUID"
        db      BAR_END
sz_cpuid_path   db      "/sys/CPUID", 0
cpu_info        ITEM    sz_freq, get_freq
                ITEM    sz_uptime, get_uptime
                ITEM    sz_threads, get_threads
                dd      0
mem_info        ITEM    sz_used, get_used
                ITEM    sz_col_free, get_free
                ITEM    sz_total, get_total
                dd      0
net_info        ITEM    sz_received, get_rx
                ITEM    sz_sent, get_tx
                dd      0
temp_info       ITEM    sz_crit, get_tjmax      ; Intel
no_info         dd      0

uglobal
align 4
shown           rd      4               ; -> graphs
shown_n         dd      ?
net_primed      dd      ?
scale           dd      ?               ; of the bars being drawn
net_sum         NET_BYTES
net_up          dd      ?               ; the devices, but the loopback
net_last        NET_BYTES
hist_rx         rd      HIST_LEN        ; bits a second
hist_tx         rd      HIST_LEN
endg
