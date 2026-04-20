; Brand
;======================
brand0:
    db 'n/a'
brand0len:

brand01:
    db '01 Celeron'
brand01len:

brand02:
    db '02 Pentium III'
brand02len:

brand03:
    db '03 Pentium III Xeon'
brand03len:

brand03d:
    db '03 Celeron'
brand03dlen:

brand04:
    db '04 Pentium III'
brand04len:

brand06:
    db '06 Pentium III M'
brand06len:

brand07:
    db '07 Celeron M'
brand07len:

brand08:
    db '08 Pentium 4'
brand08len:

brand09:
    db '09 Pentium 4'
brand09len:

brand0A:
    db '0A Celeron'
brand0Alen:

brand0B:
    db '0B Xeon'
brand0Blen:

brand0Bd:
    db '0B Xeon MP'
brand0Bdlen:

brand0C:
    db '0C Xeon MP'
brand0Clen:

brand0E:
    db '0E Pentium 4 M'
brand0Elen:

brand0Ed:
    db '0E Xeon'
brand0Edlen:

brand0F:
    db '10 Celeron M'
brand0Flen:

brand11:
    db '11 Mobile CPU'
brand11len:

brand12:
    db '12 Celeron M'
brand12len:

brand13:
    db '13 Celeron M'
brand13len:

brand14:
    db '14 Celeron'
brand14len:

brand15:
    db '15 Mobile CPU'
brand15len:

brand16:
    db '16 Pentium M'
brand16len:

brand17:
    db '17 Celeron M'
brand17len:
;======================

 ;features

FPU:
    db 'FPU:         ',0
FPUlen:

VME:
    db 'VME:         ',0
VMElen:

DE:
    db 'DE:         ',0
DElen:

PSE:
    db 'PSE:         ',0
PSElen:

TSC:
    db 'TSC:         ',0
TSClen:

MSR:
    db 'MSR:         ',0
MSRlen:

PAE:
    db 'PAE:         ',0
PAElen:

MCE:
    db 'MCE:         ',0
MCElen:

CX8:
    db 'CX8:         ',0
CX8len:

APIC:
    db 'APIC:         ',0
APIClen:

SEP:
    db 'SEP:         ',0
SEPlen:

MTRR:
    db 'MTRR:         ',0
MTRRlen:

PGE:
    db 'PGE:         ',0
PGElen:

MCA:
    db 'MCA:         ',0
MCAlen:

CMOV:
    db 'CMOV:         ',0
CMOVlen:

PAT:
    db 'PAT:         ',0
PATlen:

PSE36:
    db 'PSE36:         ',0
PSE36len:

PSNUM:
    db 'PSNUM:         ',0
PSNUMlen:

CLFLUSHn:
    db 'CLFLSH:         ',0
CLFLUSHnlen:

DTS:
    db 'DTS:         ',0
DTSlen:

ACPI:
    db 'ACPI:         ',0
ACPIlen:

MMX:
    db 'MMX:         ',0
MMXlen:

FXSR:
    db 'FXSR:         ',0
FXSRlen:

SSE:
    db 'SSE:         ',0
SSElen:

SSE2:
    db 'SSE2:         ',0
SSE2len:

SSn:
    db 'SS:         ',0
SSnlen:

TM:
    db 'TM:         ',0
TMlen:

IA64:
    db 'IA64:         ',0
IA64len:

PBE:
    db 'PBE:         ',0
PBElen:

Res:
    db 'Reserved',0
Reslen:

;Extended

SS3:
    db 'SSE3:         ',0
SS3len:

MON:
    db 'MON:          ',0
MONlen:

DS_CPL:
    db 'DS-CPL:       ',0
DS_CPLlen:

EST:
    db 'EST:         ',0
ESTlen:

TM2:
    db 'TM2:         ',0
TM2len:

CNXT_ID:
    db 'CNXT-ID:        ',0
CNXT_IDlen:

CX16:
    db 'CX16:           ',0
CX16len:

ETPRD:
    db 'xTPR/ETPRD:       ',0 ; xTPR
ETPRDlen:

;Extended2

SYS:
    db 'SYSCALL:            ',0
SYSlen:

MP:
    db 'MP:            ',0
MPlen:

NX:
    db 'NX/XD:            ',0
NXlen:

MMXPi:
    db 'MMX+:          ',0
MMXPilen:

MMXn:
    db 'MMX:           ',0
MMXnlen:

FXSRn:
    db 'FXSR:          ',0
FXSRnlen:

FFXSR:
    db 'FFXSR:           ',0
FFXSRlen:

TSCP:
    db 'TSCP:            ',0
TSCPlen:

LM:
    db 'EM64T/LM:             ',0 ; AA64
LMlen:

DNo:
    db '3DNow!+:         ',0
DNolen:

DN:
    db '3DNow!:          ',0
DNlen:

LAF:
    db 'LAHF:           ',0
LAFlen:

VMX:
    db 'VMX:          ',0
VMXlen:

SSSE3:
    db 'SSSE3:              ',0
SSSE3len:

DCA:
    db 'DCA:          ',0
DCAlen:


MCR8:
    db 'MCR8:              ',0
MCR8len:

SVM:
    db 'SVM:          ',0
SVMlen:

CMPL:
    db 'CMPL:          ',0
CMPLlen:
;================================
abrand00:
    db '8-bit Brand (no details)'
abrand00len:

abrand0:
    db 'AMD Opteron UP'
abrand0len:

abrand1:
    db 'AMD Opteron DP'
abrand1len:

abrand2:
    db 'AMD Opteron MP'
abrand2len:
;=====================================
multi0:
    db 'n/a'
multi0len:

;=====================================
clock0:
     db 'n/a'
clock0len:

