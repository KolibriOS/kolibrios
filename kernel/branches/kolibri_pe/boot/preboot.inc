;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


display_modechg     db	0 ; display mode change for text, yes/no (0 or 2)
			  ;
			  ; !! Important note !!
			  ;
			  ; Must be set to 2, to avoid two screenmode
			  ; changes within a very short period of time.

display_atboot	    db	0 ; show boot screen messages ( 2-no )

preboot_graph	    dw	0 ; graph mode
x_save		    dw	0 ; x
y_save		    dw	0 ; y
number_vm	    dw	0 ;
;pixel_save          dw  0 ; per to pixel
preboot_gprobe	    db	0 ; probe vesa3 videomodes (1-no, 2-yes)
preboot_vrrm	    db	2 ; use VRR_M (1-yes, 2- no)
preboot_dma	    db	0 ; use DMA for access to HDD (1-always, 2-only for read, 3-never)
preboot_device	    db	0 ; boot device
			  ; (1-floppy 2-harddisk 3-kernel restart 4-format ram disk)
			  ;!!!! 0 - autodetect !!!!
preboot_blogesc = 0       ; start immediately after bootlog
preboot_biosdisk    db	2 ; use V86 to access disks through BIOS (1-yes, 2-no)

;      if $>0x200
;ERROR: prebooting parameters must fit in first sector!!!
;      end if
hdsysimage	    db	'KOLIBRI IMG' ; load from
image_save	    db	'KOLIBRI IMG' ; save to
