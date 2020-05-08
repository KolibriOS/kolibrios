; Decoding cache L1,L2,L3 for Intel

decodecache32:

call decodecache

 

decodecache24:

shr eax, 8

call decodecache

shr eax, 8

call decodecache

shr eax, 8


decodecache:

comp0:
cmp al, 00h
je L000


comp6:
cmp al, 06h
je kk6

comp7:
cmp al, 08h
je kk7

comp61:  ;+1
cmp al, 09h
je kk61

comp11:
cmp al, 0Ah
je kk11
je Ld8


comp13:
cmp al, 0Ch
je kk13
je Ld16

comp62:  ;+1
cmp al, 0Dh
je kk15

comp63:  ;+1
cmp al, 0Eh
je kk63

comp14:
cmp al, 10h
je kk14
je Ld16


comp8:
cmp al, 15h
je kk8

comp48:
cmp al, 1Ah
je kk48
je L96

comp64:   ;+1
cmp al, 21h
je kk23


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

comp16:
cmp al, 2Ch
je kk16
je Ld32

comp10:
cmp al, 30h
je kk10

comp18:
cmp al, 39h
je kk18
je L128

comp49:
cmp al, 3Ah
je kk49
je L192

comp19:
cmp al, 3Bh
je kk19
je L128

comp22:
cmp al, 3Ch
je kk22
je L256

comp50:
cmp al, 3Dh
je kk50
je L384

comp51:
cmp al, 3Eh
je kk51
je L512

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


comp53:
cmp al, 46h
je kk53
je L34

comp54:
cmp al, 47h
je kk54
je L38

comp65:              ;+1
cmp al, 48h
je kk26


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

comp60:
cmp al, 4Eh
je kk60
je L6144

comp36:
cmp al, 60h
je kk36
je Ld16

comp12:
cmp al, 66h
je kk12
je Ld8

comp15:
cmp al, 67h
je kk15
je Ld16

comp17:
cmp al, 68h
je kk17
je Ld32

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


comp47:
cmp al, 73h
je kk47
je Li64


comp9:
cmp al, 77h
je kk9



comp37:
cmp al, 78h
je kk37
je L1024



comp20:
cmp al, 79h
je kk20
je L128

comp23:
cmp al, 7Ah
je kk23
je L256


comp26:
cmp al, 7Bh
je kk26
je L512

comp29:
cmp al, 7Ch
je kk29
je L1024

comp38:
cmp al, 7Dh
je kk38
je L2048

comp24:
cmp al, 7Eh
je kk24
je L256

comp52:
cmp al, 7Fh
je kk52
je L512


comp66:
cmp al, 80h
je kk21


comp21:
cmp al, 81h
je kk21
je L128


comp25:
cmp al, 82h
je kk25
je L256


comp27:
cmp al, 83h
je kk27
je L512


comp30:
cmp al, 84h
je kk30
je L1024

comp32:
cmp al, 85h
je kk32
je L2048

comp28:
cmp al, 86h
je kk28
je L512

comp31:
cmp al, 87h
je kk31
je L1024

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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;comp67:               ;+1
;cmp al, 208          ; D0h
;je kk39

;comp68:               ;+1
;cmp al, 209         ; D1h
;je kk68

;comp69:               ;+1
;cmp al, 210         ; D2h
;je kk43

;comp70:               ;+1
;cmp al, 214          ; D6h
;je kk40

;comp71:               ;+1
;cmp al, 215         ; D7h
;je kk41

;comp72:               ;+1
;cmp al, 216          ;D8h
;je kk42

;comp73:               ;+1
;cmp al, 220          ; DCh
;je kk73

;comp74:               ;+1
;cmp al, 221         ;DDh
;je kk74

;comp75:               ;+1
;cmp al, 222        ;DEh
;je kk56

;comp76:               ;+1
;cmp al, 226        ;E2h
;je kk76

;comp77:               ;+1
;cmp al, 227        ;E3h
;je kk55

;comp78:               ;+1
;cmp al, 228        ;E4h
;je kk57

;comp79:               ;+1
;cmp al, 234         ;EAh
;je kk79

;comp80:               ;+1
;cmp al, 235         ;EBh
;je kk80

;comp81:               ;+1
;cmp al, 236         ;ECh
;je kk81

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
comp67:               ;+1
cmp al, 0D0h
je kk39

comp68:               ;+1
cmp al, 0D1h
je kk68

comp69:               ;+1
cmp al, 0D2h
je kk43

comp70:               ;+1
cmp al, 0D6h
je kk40

comp71:               ;+1
cmp al, 0D7h
je kk41

comp72:               ;+1
cmp al, 0D8h
je kk42

comp73:               ;+1
cmp al, 0DCh
je kk73

comp74:               ;+1
cmp al, 0DDh
je kk74

comp75:               ;+1
cmp al, 0DEh
je kk56

comp76:               ;+1
cmp al, 0E2h
je kk76

comp77:               ;+1
cmp al, 0E3h
je kk55

comp78:               ;+1
cmp al, 0E4h
je kk57

comp79:               ;+1
cmp al, 0EAh
je kk79

comp80:               ;+1
cmp al, 0EBh
je kk80

comp81:               ;+1
cmp al, 0ECh
je kk81
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
jne L000
;------------------


;;;;;;;;;;;;;;;;; L1 instr
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

kk61:
mov [wayli], 4	  ;+1
mov [lineli], 64
jmp Li32


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

kk47:
mov [wayli], 8
;mov [lineli], 32
jmp Li64

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

kk63:
mov [wayld], 6
mov [lineld], 64
jmp Ld24

kk36:
mov [wayld], 8
mov [lineld], 64
jmp Ld16

;;;;;;;;;;;;;;;;;;;;;;;;;;;; L2
kk19:
mov [wayl2], 2
mov [linel2], 64
jmp L128

kk52:
mov [wayl2], 2
mov [linel2], 64
jmp L512

kk1:
mov [wayl2], 4
mov [linel2], 32
jmp L128

kk2:
mov [wayl2], 4
mov [linel2], 32
jmp L256

kk25:
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

kk18:
mov [wayl2], 4
mov [linel2], 64
jmp L128

kk22:
mov [wayl2], 4
mov [linel2], 64
jmp L256

kk28:
mov [wayl2], 4
mov [linel2], 64
jmp L512

kk51:
mov [wayl2], 4
mov [linel2], 64
jmp L512

kk37:
mov [wayl2], 4
mov [linel2], 64
jmp L1024

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

kk21:
mov [wayl2], 8
mov [linel2], 32
jmp L128

kk27:
mov [wayl2], 8
mov [linel2], 32
jmp L512

kk30:
mov [wayl2], 8
mov [linel2], 32
jmp L1024

kk32:
mov [wayl2], 8
mov [linel2], 32
jmp L2048

kk20:
mov [wayl2], 8
mov [linel2], 64
jmp L128

kk23:
mov [wayl2], 8
mov [linel2], 64
jmp L256

kk26:
mov [wayl2], 8
mov [linel2], 64
jmp L512

kk29:
mov [wayl2], 8
mov [linel2], 64
jmp L1024

kk31:
mov [wayl2], 8
mov [linel2], 64
jmp L1024

kk38:
mov [wayl2], 8
mov [linel2], 64
jmp L2048

kk24:
mov [wayl2], 8
mov [linel2], 128
jmp L256

kk65:
mov [wayl2], 12
mov [linel2], 64
jmp L3072

kk60:
mov [wayl2], 24
mov [linel2], 64
jmp L6144
;;;;;;;;;;;;;;;;;;;;;;;;; L3
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

kk53:
mov [wayl3], 4
mov [linel3], 64
jmp L34

kk68:
mov [wayl3], 4
mov [linel3], 64
jmp L31

kk54:
mov [wayl3], 8
mov [linel3], 64
jmp L38

kk74:
mov [wayl3], 12
mov [linel3], 64
jmp L33

kk73:
mov [wayl3], 12
mov [linel3], 64
jmp L315

kk56:
mov [wayl3], 12
mov [linel3], 64
jmp L36

kk58:
mov [wayl3], 12
mov [linel3], 64
jmp L312

kk46:
mov [wayl3], 12
mov [linel3], 128
jmp L33

kk76:
mov [wayl3], 16
mov [linel3], 64
jmp L32

kk55:
mov [wayl3], 16
mov [linel3], 64
jmp L34

kk57:
mov [wayl3], 16
mov [linel3], 64
jmp L38

kk59:
mov [wayl3], 16
mov [linel3], 64
jmp L316

kk79:
mov [wayl3], 24
mov [linel3], 64
jmp L312

kk80:
mov [wayl3], 24
mov [linel3], 64
jmp L318

kk81:
mov [wayl3], 24
mov [linel3], 64
jmp L324

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

Ld24:
mov [L1d], 24
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

L3072:
mov [L2], 3072
jmp L000

L6144:
mov [L2], 6144
jmp L000

L305:
mov [L3], 512
jmp L000

L31:
mov [L3], 1024
jmp L000

L315:
mov [L3], 1536
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

L318:
mov [L3], 18432
jmp L000

L324:
mov [L3], 24576
jmp L000

L000:
ret