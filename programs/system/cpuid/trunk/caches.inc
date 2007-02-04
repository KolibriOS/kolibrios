; Decoding cache L1,L2,L3 for Intel


decodecache:

comp0:
cmp al, 00h
je L000

;-------L2

comp1:
cmp al, 41h
je kk1


comp2:
cmp al, 42h
je kk2


comp3:
cmp al, 43h
je kk3


comp4:
cmp al, 44h
je kk4


comp5:
cmp al, 45h
je kk5


;------------L1 inst
comp6:
cmp al, 06h
je kk6

comp7:
cmp al, 08h
je kk7

comp8:
cmp al, 15h
je kk8


comp9:
cmp al, 77h
je kk9


comp10:
cmp al, 30h
je kk10

;---------------L1 data
comp11:
cmp al, 0Ah
je kk11
je Ld8

comp12:
cmp al, 66h
je kk12
je Ld8

comp13:
cmp al, 0Ch
je kk13
je Ld16

comp14:
cmp al, 10h
je kk14
je Ld16

comp15:
cmp al, 67h
je kk15
je Ld16

comp16:
cmp al, 2Ch
je kk16
je Ld32

comp17:
cmp al, 68h
je kk17
je Ld32

;-------L2
comp18:
cmp al, 39h
je kk18
je L128

comp19:
cmp al, 3Bh
je kk19
je L128

comp20:
cmp al, 79h
je kk20
je L128

comp21:
cmp al, 81h
je kk21
je L128

comp22:
cmp al, 3Ch
je kk22
je L256

comp23:
cmp al, 7Ah
je kk23
je L256

comp24:
cmp al, 7Eh
je kk24
je L256

comp25:
cmp al, 82h
je kk25
je L256

comp26:
cmp al, 7Bh
je kk26
je L512

comp27:
cmp al, 83h
je kk27
je L512

comp28:
cmp al, 86h
je kk28
je L512

comp29:
cmp al, 7Ch
je kk29
je L1024

comp30:
cmp al, 84h
je kk30
je L1024

comp31:
cmp al, 87h
je kk31
je L1024

comp32:
cmp al, 85h
je kk32
je L2048

;-----L1 Trace instr

comp33:
cmp al, 70h
je kk33
je Li12

comp34:
cmp al, 71h
je kk34
je Li16

comp35:
cmp al, 72h
je kk35
je Li32

;----New codes

comp36:
cmp al, 60h
je kk36
je Ld16

comp37:
cmp al, 78h
je kk37
je L1024

comp38:
cmp al, 7Dh
je kk38
je L2048

;---- L3
comp39:
cmp al, 22h
je kk39
je L305

comp40:
cmp al, 23h
je kk40
je L31

comp41:
cmp al, 25h
je kk41
je L32

comp42:
cmp al, 29h
je kk42
je L34

comp43:
cmp al, 88h
je kk43
je L32

comp44:
cmp al, 89h
je kk44
je L34

comp45:
cmp al, 8Ah
je kk45
je L38

comp46:
cmp al, 8Dh
je kk46
je L34

;============= v. 2.04
comp47:
cmp al, 73h
je kk47
je Li64

comp48:
cmp al, 1Ah
je kk48
je L96

comp49:
cmp al, 3Ah
je kk49
je L192

comp50:
cmp al, 3Dh
je kk50
je L384

comp51:
cmp al, 3Eh
je kk51
je L512

comp52:
cmp al, 7Fh
je kk52
je L512

comp53:
cmp al, 46h
je kk53
je L34

comp54:
cmp al, 47h
je kk54
je L38

comp55:
cmp al, 49h
je kk55
je L34

comp56:
cmp al, 4Ah
je kk56
je L36

comp57:
cmp al, 4Bh
je kk57
je L38

comp58:
cmp al, 4Ch
je kk58
je L312

comp59:
cmp al, 4Dh
je kk59
je L316

jne L000
;------------------

kk1:
mov [wayl2], 4
mov [linel2], 32
jmp L128

kk2:
mov [wayl2], 4
mov [linel2], 32
jmp L256

kk3:
mov [wayl2], 4
mov [linel2], 32
jmp L512

kk4:
mov [wayl2], 4
mov [linel2], 32
jmp L1024

kk5:
mov [wayl2], 4
mov [linel2], 32
jmp L2048
;;;;;;;;;;;;;;;;;
kk6:
mov [wayli], 4
mov [lineli], 32
jmp Li8

kk7:
mov [wayli], 4
mov [lineli], 32
jmp Li16

kk8:
mov [wayli], 4	     
mov [lineli], 32
jmp Li16

kk9:
mov [wayli], 4	  
mov [lineli], 64
jmp Li16

kk10:
mov [wayli], 8
mov [lineli], 64
jmp Li32
;;;;;;;;;;;;;;;;;;;

;---------------L1 data
kk11:
mov [wayld], 2
mov [lineld], 32
jmp Ld8

kk12:
mov [wayld], 4
mov [lineld], 64
jmp Ld8

kk13:
mov [wayld], 4
mov [lineld], 32
jmp Ld16

kk14:
mov [wayld], 4
mov [lineld], 32
jmp Ld16

kk15:
mov [wayld], 4
mov [lineld], 64
jmp Ld16

kk16:
mov [wayld], 8
mov [lineld], 64
jmp Ld32

kk17:
mov [wayld], 4
mov [lineld], 64
jmp Ld32
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kk18:
mov [wayl2], 4
mov [linel2], 64
jmp L128

kk19:
mov [wayl2], 2
mov [linel2], 64
jmp L128

kk20:
mov [wayl2], 8
mov [linel2], 64
jmp L128

kk21:
mov [wayl2], 8
mov [linel2], 32
jmp L128

kk22:
mov [wayl2], 4
mov [linel2], 64
jmp L256

kk23:
mov [wayl2], 8
mov [linel2], 64
jmp L256

kk24:
mov [wayl2], 8
mov [linel2], 128
jmp L256

kk25:
mov [wayl2], 4
mov [linel2], 32
jmp L256

kk26:
mov [wayl2], 8
mov [linel2], 64
jmp L512

kk27:
mov [wayl2], 8
mov [linel2], 32
jmp L512

kk28:
mov [wayl2], 4
mov [linel2], 64
jmp L512

kk29:
mov [wayl2], 8
mov [linel2], 64
jmp L1024

kk30:
mov [wayl2], 8
mov [linel2], 32
jmp L1024

kk31:
mov [wayl2], 8
mov [linel2], 64
jmp L1024

kk32:
mov [wayl2], 8
mov [linel2], 32
jmp L2048
;;;;;;;;;;;;;;;;;;;;;;;;;
kk33:
mov [wayli], 8
;mov [lineli], 32
jmp Li12

kk34:
mov [wayli], 8
;mov [lineli], 32
jmp Li16

kk35:
mov [wayli], 8
;mov [lineli], 32
jmp Li32
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kk36:
mov [wayld], 8
mov [lineld], 64
jmp Ld16

kk37:
mov [wayl2], 4
mov [linel2], 64
jmp L1024

kk38:
mov [wayl2], 8
mov [linel2], 64
jmp L2048
;;;;;;;;;;;;;;;;;;;;;;;;;
kk39:
mov [wayl3], 4
mov [linel3], 64
jmp L305

kk40:
mov [wayl3], 8
mov [linel3], 64
jmp L31

kk41:
mov [wayl3], 8
mov [linel3], 64
jmp L32

kk42:
mov [wayl3], 8
mov [linel3], 64
jmp  L34

kk43:
mov [wayl3], 4
mov [linel3], 64
jmp L32

kk44:
mov [wayl3], 4
mov [linel3], 64
jmp L34

kk45:
mov [wayl3], 4
mov [linel3], 64
jmp L38

kk46:
mov [wayl3], 12
mov [linel3], 128
jmp L33
;;;;;;;;;;;;;;;;;;;;;;
kk47:
mov [wayli], 8
;mov [lineli], 32
jmp Li64

kk48:
mov [wayl2], 6
mov [linel2], 64
jmp L96

kk49:
mov [wayl2], 6
mov [linel2], 64
jmp L192

kk50:
mov [wayl2], 6
mov [linel2], 64
jmp L384

kk51:
mov [wayl2], 4
mov [linel2], 64
jmp L512

kk52:
mov [wayl2], 2
mov [linel2], 64
jmp L512

kk53:
mov [wayl3], 4
mov [linel3], 64
jmp L34

kk54:
mov [wayl3], 8
mov [linel3], 64
jmp L38

kk55:
mov [wayl3], 16
mov [linel3], 64
jmp L34

kk56:
mov [wayl3], 12
mov [linel3], 64
jmp L36

kk57:
mov [wayl3], 16
mov [linel3], 64
jmp L38

kk58:
mov [wayl3], 12
mov [linel3], 64
jmp L312

kk59:
mov [wayl3], 16
mov [linel3], 64
jmp L316
;------------------
Li8:
mov [L1i], 8
jmp L000

Li12:
mov [L1i], 12
jmp L000

Li16:
mov [L1i], 16
jmp L000

Li32:
mov [L1i], 32
jmp L000

Li64:
mov [L1i], 64
jmp L000

Ld8:
mov [L1d], 8
jmp L000

Ld16:
mov [L1d], 16
jmp L000

Ld32:
mov [L1d], 32
jmp L000

L96:
mov [L2], 96
jmp L000

L128:
mov [L2], 128
jmp L000

L192:
mov [L2], 192
jmp L000

L256:
mov [L2], 256
jmp L000

L384:
mov [L2], 384
jmp L000

L512:
mov [L2], 512
jmp L000

L1024:
mov [L2], 1024
jmp L000

L2048:
mov [L2], 2048
jmp L000

L305:
mov [L3], 512
jmp L000

L31:
mov [L3], 1024
jmp L000

L32:
mov [L3], 2048
jmp L000

L33:
mov [L3], 3072
jmp L000

L34:
mov [L3], 4096
jmp L000

L36:
mov [L3], 6144
jmp L000

L38:
mov [L3], 8192
jmp L000

L312:
mov [L3], 12288
jmp L000

L316:
mov [L3], 16384
jmp L000



L000:
ret