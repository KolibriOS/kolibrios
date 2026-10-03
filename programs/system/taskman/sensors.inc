; Temperature on the Performance tab: the sensor of the CPU (Intel from
; Core, AMD from K8, Hygon, VIA C7, Nano and Zhaoxin; the rules are those
; of Linux coretemp, k8temp, k10temp and via-cputemp), else the first one
; of a monitoring chip of the board (hwmon.inc).

MSR_THERM_STATUS = 0x19C                ; Intel: the core, the package,
MSR_PKG_THERM   = 0x1B1                 ; TjMax in bits 16-23
MSR_TEMP_TARGET = 0x1A2
MSR_VIA_TEMP    = 0x1423                ; Nano and Zhaoxin, C7
MSR_C7_TEMP     = 0x1169

AMD_NB          = 0xC300                ; PCI 00:18.3, then its registers:
K8_THERMTRIP    = 0xC3E4                ; K8, the others to Family 16h
NB_TEMP         = 0xC3A4
VENDOR_AMD      = 0x1022
VENDOR_HYGON    = 0x1D94
SMN_ZEN_TEMP    = 0x59800               ; the address, the index register
SMN_ZEN_INDEX   = 0x60                  ; on the root complex, 00:00.0
SMN_F15_TEMP    = 0xD8200CA4            ; Family 15h from model 60h
SMN_F15_INDEX   = 0xB8

; The sensor of the CPU, else the chip; the temperature graph if any.
sensors_init:
        pusha
        call    cpu_sensor
        cmp     [cpu_temp], chip_temp
        je      .chip

        mcall   SF_THREAD_CONTROL, SSF_CREATE_THREAD, msr_probe, probe_stack_top
        mov     esi, 20                 ; up to 0.2 s

@@:
        cmp     [probe], 2
        je      @f
        mcall   SF_SLEEP, 1
        dec     esi
        jnz     @b

@@:
        cmp     [probe], 0
        jne     @f
        mov     [cpu_temp], chip_temp   ; the kernel killed the probe

@@:
        cmp     [tjmax], 0
        jne     .chip
        mov     byte [tjmax], 100

.chip:
        cmp     [cpu_temp], chip_temp   ; the chip for a CPU without a sensor
        jne     .temp
        call    hwm_init
        test    eax, eax
        jz      @f

.temp:
        inc     [temp_on]               ; a graph for any sensor

@@:
        cmp     [cpu_temp], intel_temp  ; TjMax of Intel only
        je      @f
        mov     [temp_graph.info], no_info

@@:
        popa
        ret

; cpu_temp and what it reads, by the vendor and the model
cpu_sensor:
        pushfd                          ; CPUID if the ID flag can be changed
        pop     eax
        xor     eax, 1 shl 21
        push    eax
        popfd
        pushfd
        pop     ecx
        cmp     eax, ecx
        jne     .done

        xor     eax, eax
        cpuid
        mov     esi, eax                ; the last leaf
        mov     edi, ebx                ; vendor

        mov     eax, 1
        cpuid
        mov     ebp, eax                ; signature

        movzx   ecx, ah
        and     cl, 0xF                 ; family
        movzx   edx, al
        shr     edx, 4                  ; model

        cmp     cl, 6
        jb      @f
        shr     eax, 12
        and     al, 0xF0
        or      dl, al
        cmp     cl, 0xF
        jne     @f
        movzx   eax, ah
        add     ecx, eax

@@:
        cmp     edi, "Genu"
        je      intel_sensor
        cmp     edi, "  Sh"             ; C7, Nano and Zhaoxin, the probe
        je      via_sensor              ; drops the others
        cmp     edi, "Cent"
        je      via_sensor
        cmp     edi, "Auth"
        je      amd_sensor
        cmp     edi, "Hygo"
        je      amd_sensor

.done:
        ret

; ecx = family, edx = model, esi = the last leaf of CPUID, ebp = signature
intel_sensor:
        cmp     esi, 6
        jb      .done
        push    ecx edx
        mov     eax, 6
        cpuid
        pop     edx ecx

        test    al, 1                   ; digital thermal sensor
        jz      .done
        mov     [cpu_temp], intel_temp
        test    al, 1 shl 6             ; the package one
        jz      @f
        mov     [therm_msr], MSR_PKG_THERM
        mov     [therm_valid], 1 shl 31 ; it has no "reading valid" bit

@@:
        cmp     ecx, 6                  ; TjMax: 100 before Nehalem,
        jne     .done                   ; then from the CPU but on Atoms
        cmp     edx, 0x1A
        jb      .done
        mov     esi, .atoms
@@:
        lodsw
        test    al, al
        jz      @f                      ; 0: msr_probe reads it
        cmp     al, dl
        jne     @b
@@:
        mov     byte [tjmax], ah

        cmp     dl, 0x1C                ; Pineview: stepping 10
        jne     .done
        and     ebp, 0xF
        cmp     ebp, 10
        jne     .done
        mov     byte [tjmax], 100

.done:
        ret

; TjMax of Atoms without MSR_TEMPERATURE_TARGET, and Xeon 7400
.atoms          db      0x1C, 90, 0x1D, 100, 0x26, 90, 0x27, 90, 0x35, 90, 0x36, 100, 0, 0

; ecx = family, edx = model
via_sensor:
        mov     eax, MSR_VIA_TEMP
        cmp     ecx, 6
        ja      @f
        cmp     dl, 0xF
        jae     @f
        mov     ax, MSR_C7_TEMP

@@:
        mov     [therm_msr], eax
        mov     [cpu_temp], via_temp
        ret

; ecx = family, edx = model
amd_sensor:
        cmp     ecx, 0xF
        jb      .done
        ja      .k10

        push    edx                     ; K8 with a thermal sensor
        mov     eax, 0x80000007
        cpuid
        test    dl, 1
        pop     edx
        jz      .done

        cmp     dl, 0x69                ; revision G desktops read lower
        jb      @f
        xchg    eax, edx
        mov     edi, .k8_mobile
        mov     ecx, 3
        repne scasb
        je      @f
        mov     [k8_zero], 49 - 21
@@:
        mov     esi, k8_temp
        jmp     .nb

.k10:
        mov     eax, SMN_ZEN_TEMP       ; Zen
        mov     [smn_index], SMN_ZEN_INDEX
        cmp     ecx, 0x17
        jae     .smn

        mov     eax, SMN_F15_TEMP
        mov     [smn_index], SMN_F15_INDEX
        cmp     ecx, 0x15
        jne     @f
        cmp     edx, 0x60
        jae     .smn

@@:
        cmp     ecx, 0x16
        ja      .done
        mov     esi, amd_nb_temp

.nb:
        mov     ecx, AMD_NB
        jmp     .pci

.smn:
        mov     [smn_addr], eax
        xor     ecx, ecx
        mov     esi, amd_smn_temp

.pci:                                   ; esi = the procedure, if the
        call    pci_read                ; device is AMD or Hygon
        cmp     ax, VENDOR_AMD
        je      @f
        cmp     ax, VENDOR_HYGON
        jne     .done
@@:
        mov     [cpu_temp], esi

.done:
        ret

.k8_mobile      db      0x6C, 0x7C, 0xC1

; ecx = device shl 8 + register on bus 0; out: eax
pci_read:                               ; the callers keep ebx
        mcall   SF_PCI, SSF_READ_DWORD
        ret

; out: eax = temperature of the CPU, CF = 1 if unknown
via_temp:
        call    read_therm
        and     eax, 0xFFFFFF
        ret

; out: eax = the low dword of [therm_msr]
read_therm:
        mcall   SF_SYS_MISC, SSF_READ_MSR, , [therm_msr]
        ret

intel_temp:
        call    read_therm
        or      eax, [therm_valid]      ; the reading is valid
        jns     no_temp

        shr     eax, 16
        and     eax, 0x7F
        neg     eax
        add     eax, [tjmax]
        clc
        ret

k8_temp:
        mov     ecx, K8_THERMTRIP
        call    pci_read

        shr     eax, 16
        movzx   eax, al
        sub     eax, [k8_zero]
        clc
        ret

amd_smn_temp:
        mcall   SF_PCI, SSF_WRITE_DWORD, [smn_index], [smn_addr]
        add     ecx, 4                  ; the data after the index
        call    pci_read

        test    eax, 1 shl 19           ; the range is -49..206
        jz      amd_temp
        or      eax, 3 shl 16
        jmp     amd_temp

amd_nb_temp:
        mov     ecx, NB_TEMP
        call    pci_read                ; then amd_temp

; eax = the reported temperature of AMD
amd_temp:
        mov     edx, eax
        shr     eax, 24                 ; 1/8 of a degree from bit 21
        and     edx, 3 shl 16
        cmp     edx, 3 shl 16
        jne     @f
        sub     eax, 49

@@:
        clc
        ret

chip_temp:                              ; none in the CPU: the first of the chip
        mov     eax, [hwm_temp]
        jmp     @b

no_temp:
        stc
        ret

; A thread reads the MSRs first: if one is not there, the kernel kills
; the thread, not the program.
msr_probe:
        call    [cpu_temp]
        inc     [probe]

        cmp     [tjmax], 0
        jne     @f
        mcall   SF_SYS_MISC, SSF_READ_MSR, , MSR_TEMP_TARGET
        shr     eax, 16
        test    al, al                  ; 0 in some virtual machines
        jz      @f
        mov     byte [tjmax], al

@@:
        inc     [probe]
        mcall   SF_TERMINATE_PROCESS

; Once a second: the sensors and the temperature for the graph
sensors_read:
        pusha
        xor     eax, eax
        call    [hwm_read]
        cmp     eax, 127                ; 127 and more: no sensor
        jb      @f
        xor     eax, eax
@@:
        mov     [hwm_temp], eax

        call    [cpu_temp]
        jnc     .got
        mov     eax, [last_temp]        ; not valid this time

.got:
        mov     [last_temp], eax
        mov     edi, last_temp          ; not below 0
        mov     ecx, 1000
        call    clamp

        mov     ecx, [hist_pos]
        mov     [hist_temp + ecx*4], eax
        popa
        ret

; An ITEM of temp_info
get_tjmax:
        mov     eax, [tjmax]
        call    fmt_uint                ; then deg_unit

deg_unit:
        mov     eax, sz_deg
        jmp     fmt_unit

cpu_temp        dd      chip_temp       ; the procedure
therm_msr       dd      MSR_THERM_STATUS
therm_valid     dd      0
tjmax           dd      100
k8_zero         dd      49

uglobal
align 4
temp_on         dd      ?               ; a graph of the temperature
smn_addr        dd      ?
smn_index       dd      ?
last_temp       dd      ?
probe           dd      ?               ; 1: the sensor is read, 2: done
                rb      256
probe_stack_top:
hist_temp       rd      HIST_LEN
endg
