driverlist:
db  'RTL8139',0         ; Realtek 8139
dd  0x813910ec
dd  0x813810ec
dd  0x12111113
dd  0x13601500
dd  0x13604033
dd  0x13001186
dd  0x13401186
dd  0xab0613d1
dd  0xa1171259
dd  0xa11e1259
dd  0xab0614ea
dd  0xab0714ea
dd  0x123411db
dd  0x91301432
dd  0x101202ac
dd  0x0106018a
dd  0x1211126c
dd  0x81391743
dd  0x8139021b
dd  0x0 ; end

db  'RTL8029',0         ; Realtek 8029 (PCI NE2000)
dd  0x802910ec
dd  0x0

db  'I8255X',0          ; Intel eepro100
dd 0x10298086
dd 0x10308086
;dd 0x10318086
;dd 0x10328086
;dd 0x10338086
;dd 0x10348086
;dd 0x10388086
dd 0x10398086
;dd 0x103A8086
;dd 0x103B8086
;dd 0x103C8086
;dd 0x103D8086
;dd 0x103E8086
;dd 0x10508086
;dd 0x10518086
;dd 0x10528086
;dd 0x10538086
;dd 0x10548086
;dd 0x10558086
;dd 0x10568086
;dd 0x10578086
;dd 0x10598086
;dd 0x10648086
;dd 0x10658086
;dd 0x10668086
;dd 0x10678086
dd 0x10688086
;dd 0x10698086
;dd 0x106A8086
;dd 0x106B8086
;dd 0x10918086
;dd 0x10928086
;dd 0x10938086
;dd 0x10948086
;dd 0x10958086
;dd 0x10fe8086
dd 0x12098086
dd 0x12298086
dd 0x24498086
;dd 0x24598086
;dd 0x245D8086
;dd 0x27DC8086
dd  0x0

db  'RTL8169',0         ; Realtek 8169
dd  0x816710ec
dd  0x816810ec
dd  0x816910ec
dd  0x011616ec
dd  0x43001186
dd  0x813610ec
dd  0x0

db  '3C59X',0           ; 3Com 59x
dd  0x590010b7
dd  0x592010b7
dd  0x597010b7
dd  0x595010b7
dd  0x595110b7
dd  0x595210b7
dd  0x900010b7
dd  0x900110b7
dd  0x900410b7
dd  0x900510b7
dd  0x900610b7
dd  0x900A10b7
dd  0x905010b7
dd  0x905110b7
dd  0x905510b7
dd  0x905810b7
dd  0x905A10b7
dd  0x920010b7
dd  0x980010b7
dd  0x980510b7
dd  0x764610b7
dd  0x505510b7
dd  0x605510b7
dd  0x605610b7
dd  0x5b5710b7
dd  0x505710b7
dd  0x515710b7
dd  0x525710b7
dd  0x656010b7
dd  0x656210b7
dd  0x656410b7
dd  0x450010b7
dd  0x0

db  'SIS900',0
dd  0x09001039
dd  0x70161039
dd  0x0

db  'PCNET32',0
dd  0x20001022
dd  0x26251022
dd  0x20011022
dd  0x0

db  'FORCEDETH',0       ; nVidia nForce
dd  0x006610de
dd  0x01c310de
dd  0x00D610de
dd  0x008610de
dd  0x008c10de
dd  0x00e610de
dd  0x00df10de
dd  0x005610de
dd  0x005710de
dd  0x003710de
dd  0x003810de
dd  0x026810de
dd  0x026910de
dd  0x037210de
dd  0x037310de
dd  0x03e510de
dd  0x03e610de
dd  0x03ee10de
dd  0x03ef10de
dd  0x045010de
dd  0x045110de
dd  0x045210de
dd  0x045310de
dd  0x054c10de
dd  0x054d10de
dd  0x054e10de
dd  0x054f10de
dd  0x07dc10de
dd  0x07dd10de
dd  0x07de10de
dd  0x07df10de
dd  0x076010de
dd  0x076110de
dd  0x076210de
dd  0x076310de
dd  0x0ab010de
dd  0x0ab110de
dd  0x0ab210de
dd  0x0ab310de
dd  0x0d7d10de
dd  0x0

db  'MTD80X',0          ; Myson / Surecom
dd  0x08031516
dd  0x08001516
dd  0x08911516
dd  0x0

db  'DEC21X4X',0
dd  0x00021011
dd  0x00091011
dd  0x00141011
dd  0x00191011
dd  0x000211AD
dd  0x051210D9
dd  0x053110D9
dd  0x1400125B
dd  0x09851317
dd  0x0

db  'R6040',0           ; eBox integrated network
dd  0x604017F3
dd  0x0

db  'I8254X',0
dd  0x10008086          ; 82542 (Fiber)
dd  0x10018086          ; 82543GC (Fiber)
dd  0x10048086          ; 82543GC (Copper)
dd  0x10088086          ; 82544EI (Copper)
dd  0x10098086          ; 82544EI (Fiber)
dd  0x100A8086          ; 82540EM
dd  0x100C8086          ; 82544GC (Copper)
dd  0x100D8086          ; 82544GC (LOM)
dd  0x100E8086          ; 82540EM
dd  0x100F8086          ; 82545EM (Copper)
dd  0x10108086          ; 82546EB (Copper)
dd  0x10118086          ; 82545EM (Fiber)
dd  0x10128086          ; 82546EB (Fiber)
dd  0x10138086          ; 82541EI
dd  0x10148086          ; 82541ER
dd  0x10158086          ; 82540EM (LOM)
dd  0x10168086          ; 82540EP (Mobile)
dd  0x10178086          ; 82540EP
dd  0x10188086          ; 82541EI
dd  0x10198086          ; 82547EI
dd  0x101a8086          ; 82547EI (Mobile)
dd  0x101d8086          ; 82546EB
dd  0x101e8086          ; 82540EP (Mobile)
dd  0x10268086          ; 82545GM
dd  0x10278086          ; 82545GM
dd  0x10288086          ; 82545GM
dd  0x105b8086          ; 82546GB (Copper)
dd  0x10758086          ; 82547GI
dd  0x10768086          ; 82541GI
dd  0x10778086          ; 82541GI
dd  0x10788086          ; 82541ER
dd  0x10798086          ; 82546GB
dd  0x107a8086          ; 82546GB
dd  0x107b8086          ; 82546GB
dd  0x107c8086          ; 82541PI
dd  0x10b58086          ; 82546GB (Copper)
dd  0x11078086          ; 82544EI
dd  0x11128086          ; 82544GC
dd  0x15A18086          ; I218-V
dd  0x0

db  'RHINE', 0          ; VIA Rhine
dd  0x30431106
dd  0x61001106
dd  0x30651106
dd  0x31061106
dd  0x30531106
dd  0x0


db  'AR81XX', 0
dd  0x10631969          ; AR8131
;dd  0x10621969          ; AR8132
;dd  0x10731969          ; AR8151 V1
;dd  0x10831969          ; AR8151 V2
;dd  0x20601969          ; AR8152 V1
dd  0x20621969          ; AR8152 V2
dd  0x10911969          ; AR8161
;dd  0xe0911969          ; E2200
;dd  0xe0a11969          ; E2400
;dd  0xe0b11969          ; E2500
dd  0x10901969          ; AR8162
dd  0x10A11969          ; AR8171 (QCA8171)
dd  0x10A01969          ; AR8172 (QCA8172)


dd  0x0

dd  0x0 ; driverlist end