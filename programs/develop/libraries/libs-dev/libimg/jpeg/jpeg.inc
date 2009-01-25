;;================================================================================================;;
;;//// jpeg.inc //// (c) diamond, 2008-2009 //////////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; Lesser General Public License as published by the Free Software Foundation, either version 2.1 ;;
;; of the License, or (at your option) any later version.                                         ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; Lesser General Public License for more details.                                                ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU Lesser General Public License along with Libs-Dev.  ;;
;; If not, see <http://www.gnu.org/licenses/>.                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;

struct jpeg.work	; working area for JPEG handling
image			dd	?
; progressive JPEG?
progressive		db	?
; one component in the scan?
not_interleaved		db	?
; Adobe YCCK file?
adobe_ycck		db	?
			rb	1
; parameters for progressive scan
ScanStart		db	?
ScanEnd			db	?
ApproxPosLow		db	?
ApproxPosHigh		db	?
; restart interval
restart_interval	dd	?
decoded_MCUs		dd	?

_esp			dd	?

; components information, up to 4 components
; db ComponentIdentifier, db V, db H, db VFactor, db HFactor, db QuantizationTable
components		rb	4*6
max_v			db	?
max_h			db	?
cur_rst_marker		db	?
			db	?
huffman_bits		dd	?
block_width	dd	?
block_height	dd	?
block_delta_x	dd	?
block_delta_y	dd	?
cur_block_dx	dd	?
cur_block_dy	dd	?
x_num_blocks	dd	?
y_num_blocks	dd	?
delta_x		dd	?
delta_y		dd	?
pixel_size	dd	?
line_size	dd	?
cur_x		dd	?
cur_y		dd	?
max_x		dd	?
max_y		dd	?
cur_out_ptr	dd	?
dct_buffer	dd	?
dct_buffer_size	dd	?
;ns			dd	?
; +0: db V, db H, db VFactor, db HFactor, dd HIncrement, dd VIncrement,
; +12: dd QuantizationTable, dd DCTable, dd ACTable,
; +24: dd width/HFactor, dd width/HFactor-8k, dd HFactor+1-(width%HFactor),
; +36: dd height/VFactor, dd height/VFactor-8m, dd VFactor+1-(height%VFactor),
; +48: dw DCPrediction, db ?, db (0 for Y, 80h for Cb,Cr), dd ComponentOffset
cur_components		rb	4*56
cur_components_end	dd	?
; Fourier coefficients
dct_coeff		rw	64
; Temporary space for IDCT
idct_tmp_area		rd	64
; decoded block 8*8
decoded_data		rb	8*8
; up to 4 quantization tables
quant_tables		rd	4*64
quant_tables_defined	rb	4

; Huffman tables
dc_huffman_defined	rb	4
ac_huffman_defined	rb	4
; up to 4 DC Huffman tables
;dc_huffman		rd	4*256*2
; up to 4 AC Huffman tables
;ac_huffman		rd	4*256*2
max_hufftable_size = (256 + (9+128)*16)*2
dc_huffman		rb	4*max_hufftable_size
ac_huffman		rb	4*max_hufftable_size

ends
