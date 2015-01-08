;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;flags------------------------------------------------------------
DEBUG        equ 1 ;show messages at debug board
use_cli_sti  equ 1 ;driver become more stable (theoretically)

;constants--------------------------------------------------------
API_VERSION     equ 0  ;debug

sb_irq_num      equ 5 ;default values for SB16, may be overrided by autodetect
sb_dma_num      equ 5 ;default values for SB16, may be overrided by autodetect

small_buffer    equ 32768
full_buffer     equ 65536
sb_buffer_size  equ small_buffer  ; FIX ring buffer overlapped events issue;  full_buffer

__supported_buffer_sizes fix <small_buffer, full_buffer>

if ~(sb_buffer_size in __supported_buffer_sizes)
display 13,10,'unsupported buffer size was selected, check config.inc',13,10
stop
end if

sb_out_rate     equ 48000
;time constant for cards older than SB16
sb_tc           equ (256-(1000000/(sb_out_rate*2)))

SRV_GETVERSION        equ  0
DEV_PLAY              equ  1
DEV_STOP              equ  2
DEV_CALLBACK          equ  3
DEV_SET_BUFF          equ  4
DEV_NOTIFY            equ  5
DEV_SET_MASTERVOL     equ  6
DEV_GET_MASTERVOL     equ  7
DEV_GET_INFO          equ  8

DRV_ENTRY             equ  1
DRV_EXIT              equ -1

