;+---------------------------------+
;|     DEFINITION BLOCKS           |
;+---------------------------------+

t_block_0:
db 0,0,0,0
db 1,1,1,0
db 0,1,0,0
db 0,0,0,0
dd t_block_3

t_block_1:
db 0,1,0,0
db 1,1,0,0
db 0,1,0,0
db 0,0,0,0
dd t_block_0

t_block_2:
db 0,1,0,0
db 1,1,1,0
db 0,0,0,0
db 0,0,0,0
dd t_block_1

t_block_3:
db 0,1,0,0
db 0,1,1,0
db 0,1,0,0
db 0,0,0,0
dd t_block_2

;-----------------------------------

i_block_0:
db 0,1,0,0
db 0,1,0,0
db 0,1,0,0
db 0,1,0,0
dd i_block_1

i_block_1:
db 0,0,0,0
db 1,1,1,1
db 0,0,0,0
db 0,0,0,0
dd i_block_0
;-----------------------------------

q_block_0:
db 0,1,1,0
db 0,1,1,0
db 0,0,0,0
db 0,0,0,0
dd q_block_0

;-----------------------------------

s_block_0:
db 0,0,0,0
db 0,1,1,0
db 1,1,0,0
db 0,0,0,0
dd s_block_1

s_block_1:
db 1,0,0,0
db 1,1,0,0
db 0,1,0,0
db 0,0,0,0
dd s_block_0

;-----------------------------------

l_block_0:
db 0,0,0,0
db 1,1,1,0
db 1,0,0,0
db 0,0,0,0
dd l_block_3

l_block_1:
db 1,1,0,0
db 0,1,0,0
db 0,1,0,0
db 0,0,0,0
dd l_block_0

l_block_2:
db 0,0,1,0
db 1,1,1,0
db 0,0,0,0
db 0,0,0,0
dd l_block_1

l_block_3:
db 0,1,0,0
db 0,1,0,0
db 0,1,1,0
db 0,0,0,0
dd l_block_2

;-----------------------------------

g_block_0:

db 0,1,0,0
db 0,1,0,0
db 1,1,0,0
db 0,0,0,0
dd g_block_1

g_block_1:
db 0,0,0,0
db 1,1,1,0
db 0,0,1,0
db 0,0,0,0
dd g_block_2

g_block_2:
db 0,1,1,0
db 0,1,0,0
db 0,1,0,0
db 0,0,0,0
dd g_block_3

g_block_3:
db 1,0,0,0
db 1,1,1,0
db 0,0,0,0
db 0,0,0,0
dd g_block_0

;-----------------------------------

k_block_0:
db 0,0,0,0
db 1,1,0,0
db 0,1,1,0
db 0,0,0,0
dd k_block_1

k_block_1:
db 0,1,0,0
db 1,1,0,0
db 1,0,0,0
db 0,0,0,0
dd k_block_0

;-----------------------------------
;logo blocks
;-----------------------------------

tetris_t:
db 1,1,1,0
db 0,1,0,0
db 0,1,0,0
db 0,1,0,0
db 0,1,0,0

tetris_e:
db 1,1,1,0
db 1,0,0,0
db 1,1,0,0
db 1,0,0,0
db 1,1,1,0

tetris_r:
db 1,1,0,0
db 1,0,1,0
db 1,1,0,0
db 1,0,1,0
db 1,0,1,0

tetris_i:
db 0,1,0,0
db 0,0,0,0
db 0,1,0,0
db 0,1,0,0
db 0,1,0,0

tetris_s:
db 0,1,1,1
db 1,0,0,0
db 0,1,1,0
db 0,0,0,1
db 1,1,1,0

tetris_II:
db 1,1,1,1
db 0,1,1,0
db 0,1,1,0
db 0,1,1,0
db 1,1,1,1

;-----------------------------------
block_table:
    dd t_block_0    ; t
    dd t_block_1
    dd t_block_2
    dd t_block_3
    dd i_block_0    ; i
    dd i_block_1
    dd i_block_0
    dd i_block_1
    dd q_block_0    ; q
    dd q_block_0
    dd q_block_0
    dd q_block_0
    dd s_block_0    ; s
    dd s_block_1
    dd s_block_0
    dd s_block_1
    dd l_block_0    ; l
    dd l_block_1
    dd l_block_2
    dd l_block_3
    dd g_block_0    ; g
    dd g_block_1
    dd g_block_2
    dd g_block_3
    dd k_block_0    ; k
    dd k_block_1
    dd k_block_0
    dd k_block_1

color_table:
    dd 00000000h    ;black       0
    dd 00cccccch    ;white       1
    dd 00cc0000h    ;red         2
    dd 0000cc00h    ;green       3
    dd 000000cch    ;blue        4
    dd 00cccc00h    ;yellow      5
    dd 0000cccch    ;cyan        6
    dd 00cc00cch    ;pink        7