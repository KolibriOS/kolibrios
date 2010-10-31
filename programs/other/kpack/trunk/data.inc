;---------------------------------------------------------------------
;rb	0xD3C  ;unknown space area
params:
	rb 256
;---------------------------------------------------------------------	
color_table	rd 10
skinheight	rd 1

innamelen	rd 1
inname		rb 48
outnamelen	rd 1
outname		rb 48
pathlen		rd 1
path		rb 48
curedit_y	rd 1

message_mem	rb 80*20
message_cur_pos	rd 1

outsize		rd 1
infile		rd 1
outfile		rd 1
outfile1	rd 1
outfile2	rd 1
outfilebest	rd 1
inbuftmp	rd 1
workmem		rd 1
lzma_dictsize	rd 1
ct1		rb 256
ctn		rd 1
cti		rb 1
use_lzma	= 1

use_no_calltrick = 0
use_calltrick1	= 40h
use_calltrick2	= 80h

method		rb 1

;---------------------------------------------------------------------
align 4
fn70block:
fn70op		rd 1
fn70start	rd 1
fn70size	rd 1
fn70zero	rd 1
fn70dest	rd 1
fullname	rb 100

;---------------------------------------------------------------------
align 4
file_attr	rd 8
insize		rd 1       ; last qword in file_attr
		rd 1
;---------------------------------------------------------------------
align 4
	rb 4096
stacktop:
;---------------------------------------------------------------------
; Deconpress area
align 4
unpack.p	rb	0x1F36*4
unpack.code_:
		rd	1
unpack.range:
		rd	1
unpack.rep0	rd	1
unpack.rep1	rd	1
unpack.rep2	rd	1
unpack.rep3	rd	1
unpack.previousByte rb	1
;---------------------------------------------------------------------
; Compress area
align 4
_lenEncoder:
	rd 8451
;-----------------------------------------------------
_prices:
	rd 4384
	rd 17
;-----------------------------------------------------
_finished:		rb 1
_writeEndMark:		rb 1
_longestMatchWasFound:	rb 1
_previousByte:		rb 1
_longestMatchLength:	rd 1
;-----------------------------------------------------
g_FastPos:
	rb 1024
;-----------------------------------------------------
_posSlotPrices:
	rd 256
;-----------------------------------------------------
_isRep0Long:
	rd 192
;-----------------------------------------------------
distances:
	rd 274
;-----------------------------------------------------
_optimumCurrentIndex:	rd 1
_additionalOffset:	rd 1
;-----------------------------------------------------
_isRepG1:
	rd 12
;-----------------------------------------------------
_isMatch:
	rd 192
;-----------------------------------------------------
_alignPriceCount:	rd 1
_numLiteralContextBits:	rd 1
;-----------------------------------------------------
_literalEncoder:
	rd 114
;-----------------------------------------------------
nowPos64:
	rd 2
;-----------------------------------------------------
_distancesPrices:
	rd 512
;-----------------------------------------------------
_repDistances:
	rd 4
;-----------------------------------------------------
_posSlotEncoder:
	rd 1028
;-----------------------------------------------------
lastPosSlotFillingPos:
	rd 2
;-----------------------------------------------------
_numFastBytes:	rd 1
_posStateMask:	rd 1
;-----------------------------------------------------
_isRepG0:
	rd 12
;-----------------------------------------------------
_repMatchLenEncoder:
	rd 8451
	rd 4384
	rd 17
;-----------------------------------------------------
_isRepG2:
	rd 12
;-----------------------------------------------------
_dictionarySize:		rd 1
_numLiteralPosStateBits:	rd 1
_distTableSize:			rd 1
_optimumEndIndex:		rd 1
;-----------------------------------------------------
;static CState state
state.State:		rb 1
state.Prev1IsChar:	rb 1
state.Prev2:		rb 2
state.PosPrev2:		rd 1
state.BackPrev2:	rd 1
state.Price:		rd 1
state.PosPrev:		rd 1
state.BackPrev:		rd 1
state.Backs:
	rd 4
;----------------------------------------------------
	rd 40950
;-----------------------------------------------------
_alignPrices:
	rd 16
;-----------------------------------------------------
_isRep:
	rd 12
;-----------------------------------------------------
_posAlignEncoder:
	rd 256
;-----------------------------------------------------
i_01:	rd 1
;-----------------------------------------------------
_state:			rb 1
_cache:			rb 1
_state.Prev2:		rb 2
_posEncoders:		rd 1
_numPrevBits:		rd 1
_numPosBits:		rd 1
_posMask:		rd 1
_posStateBits:		rd 1
range:			rd 1
_cacheSize:		rd 1
_cyclicBufferSize:	rd 1
;-----------------------------------------------------
low:
	rd 2
;-----------------------------------------------------
Models:
	rd 512
;-----------------------------------------------------
_matchMaxLen:	rd 1
pack_pos:	rd 1
_cutValue:	rd 1
_hash:		rd 1
;-----------------------------------------------------
crc_table:
	rd 256
;-----------------------------------------------------
_buffer:	rd 1
_pos:		rd 1
_streamPos:	rd 1
pack_length:	rd 1
;---------------------------------------------------------------------
