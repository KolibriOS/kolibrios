;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	BS_OEMName	db	'KOLIBRI '	; db 8
	BPB_BytsPerSec	dw	512		; bytes per sector
	BPB_SecPerClus	db	2		; sectors per cluster
	BPB_RsvdSecCnt	dw	1		; number of reserver sectors
	BPB_NumFATs	db	2		; count of FAT data structures
	BPB_RootEntCnt	dw	240		; count of 32-byte dir. entries (240*32 = 15 sectors)
	BPB_TotSec16	dw	5760		; count of sectors on the volume (5760 for 2.88 mbytes disk)
	BPB_Media	db	0f0h		; f0 - used for removable media
	BPB_FATSz16	dw	9		; count of sectors by one copy of FAT
	BPB_SecPerTrk	dw	36		; sectors per track
	BPB_NumHeads	dw	2		; number of heads
	BPB_HiddSec	dd	0		; count of hidden sectors
	BPB_TotSec32	dd	0		; count of sectors on the volume (if > 65535)
	BS_DrvNum	db	0		; int 13h drive number
	BS_Reserved	db	0		; reserved
	BS_BootSig	db	29h		; Extended boot signature
	BS_VolID	dd	0		; Volume serial number
	BS_VolLab	db	'KOLIBRI    '	; Volume label (db 11)
	BS_FilSysType	db	'FAT12   '	; file system type (db 8)
