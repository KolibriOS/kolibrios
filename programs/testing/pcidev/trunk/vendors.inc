; AUTHOR: Victor Alberto Gil Hancco Laquita <vhanla@gmail.com>
; This list might content mistakes, plz report it
; There are 865 Lists of Vendors' Names
; Date: 8/14/2005
;
;  maybe this list is outdated...
; (the source was from 2004 list)

; changed 11/05/2006: + 3 vendors and no more label 'PREVIOUSVERSIONLIST'
; so the total quantity of vendors is 875 now (changes by Sergey Kuzmin)

; changed 16/08/2006: + 122 vendors
; so the total quantity of vendors is 987 now (changes by Sergey Kuzmin)

; changed ??/04/2007: 
; size optimezed 
; total quantity of vendors is 997 now (changes by Mihailov Ilia)

; changed ??/03/2010: 
; only few best-known vendors left in this file to reduce the file size
; the other list is in vendors.add (changes by Artem Jerdev)

; changed 15/01/2014: 
; typo fixes and new subclasses (changes by Sergey Kuzmin)

macro VendorID a,b {	; by mike.dld
	dw a
	dd _vtmp#a
	\iglobal
		_vtmp#a db b
		db	0
	\};endg
}

macro ClassID a,b,c,d {
	db a
	db b
	local tt

	if c eq
		db 0
		db 1
		tt = 'x'
	else
		db c
		db 0
		tt = c
	end if
	
	dd _ctmp#a#b#tt
	\iglobal
		_ctmp#a#b#tt db d
		db	0
	\};endg
}

;--------------------------------------
; to build the full vendors list, include vendors.add 
; to the bottom of this section
;
VendorsTab:
VendorID	0x0033, 'PARADYNE Corp.'
VendorID	0x003D, 'Lockheed Martin Corp'
VendorID	0x0E11, 'COMPAQ COMPUTER Corp.'
VendorID	0x1000, 'SYMBIOS LOGIC Inc./LSI Logic'
VendorID	0x1002, 'ATI TECHNOLOGIES Inc.'
VendorID	0x100B, 'NATIONAL SEMICONDUCTOR Corp.'
VendorID	0x100C, 'Tseng Labs'
VendorID	0x100E, 'Weitek'
VendorID	0x1010, 'VIDEO LOGIC Ltd.'
VendorID	0x1011, 'Digital Equipment Corp.'
VendorID	0x1013, 'Cirrus Logic'
VendorID	0x1014, 'IBM'
VendorID	0x101A, 'NCR Corp.'
VendorID	0x101E, 'AMERICAN MEGATRENDS'
VendorID	0x1020, 'HITACHI COMPUTER PRODUCTS'
VendorID	0x1021, 'OKI ELECTRIC INDUSTRY Co., Ltd.'
VendorID	0x1022, 'Advanced Micro Devices'
VendorID	0x1023, 'TRIDENT MICROSYSTEMS'
VendorID	0x1025, 'Acer Inc.'
VendorID	0x1028, 'DELL COMPUTER Corp.'
VendorID	0x102A, 'LSI LOGIC Corp.'
VendorID	0x102B, 'MATROX GRAPHICS'
VendorID	0x102F, 'TOSHIBA AMERICA'
VendorID	0x1033, 'NEC Corp.'
VendorID	0x1039, 'SILICON INTEGRATED SYSTEMS'
VendorID	0x103C, 'Hewlett-Packard Company'
VendorID	0x103F, 'SYNOPSYS/LOGIC MODELING GROUP'
VendorID	0x1042, 'Micron Electronics'
VendorID	0x1043, 'ASUSTEK COMPUTER'
VendorID	0x104A, 'STMicroelectronics'
VendorID	0x104B, 'Mylex/Buslogic'
VendorID	0x104C, 'TEXAS INSTRUMENTS'
VendorID	0x104D, 'SONY Corp.'
VendorID	0x104E, 'Oak Technology'
VendorID	0x1054, 'HITACHI'
VendorID	0x1055, 'Standard Microsystems Corp.'
VendorID	0x1057, 'Motorola'
VendorID	0x1067, 'MITSUBISHI ELECTRIC AMERICA'
VendorID	0x106B, 'APPLE COMPUTER Inc.'
VendorID	0x106C, 'Hyundai Electronics America'
VendorID	0x106D, 'SEQUENT COMPUTER SYSTEMS'
VendorID	0x1070, 'DAEWOO TELECOM Ltd.'
VendorID	0x1071, 'MITAC'
VendorID	0x1073, 'YAMAHA Corp.'
VendorID	0x1074, 'Nexgen Microsystems'
VendorID	0x1076, 'Chaintech Comp.'
VendorID	0x1077, 'QLOGIC  Corp.'
VendorID	0x1078, 'Cyrix Corp.'
VendorID	0x1079, 'I-BUS'
VendorID	0x107B, 'GATEWAY 2000'
VendorID	0x107D, 'Leadtek Research'
VendorID	0x107E, 'INTERPHASE Corp.'
VendorID	0x107F, 'Data Technology Corp.'
VendorID	0x1080, 'Cypress Semiconductor'
VendorID	0x108E, 'Sun Microsystems'
VendorID	0x1091, 'Intergraph Corp.'
VendorID	0x1092, 'Diamond Computer Systems'
VendorID	0x1093, 'National Instruments'
VendorID	0x1095, 'SILICON IMAGE (WAS CMD TECHNOLOGY Inc.)'
VendorID	0x10A9, 'SILICON GRAPHICS'
VendorID	0x10AC, 'HONEYWELL IAC'
VendorID	0x10AD, 'Winbond Systems Labs'
VendorID	0x10AF, 'MICRO COMPUTER SYSTEMS Inc.'
VendorID	0x10B5, 'PLX TECHNOLOGY.'
VendorID	0x10B6, 'MADGE NETWORKS'
VendorID	0x10B7, '3COM Corp, Networking Division'
VendorID	0x10B8, 'Standard Microsystems Corp.'
VendorID	0x10B9, 'Acer Labs Inc.'
VendorID	0x10BA, 'MITSUBISHI ELECTRIC Corp.'
VendorID	0x10BD, 'Surecom Technology'
VendorID	0x10C2, 'AUSPEX SYSTEMS Inc.'
VendorID	0x10C3, 'Samsung Semiconductors'
VendorID	0x10C4, 'Award Software International Inc.'
VendorID	0x10C8, 'NEOMAGIC Corp.'
VendorID	0x10CA, 'FUJITSU MICROELECTRONIC'
VendorID	0x10CB, 'OMRON Corp.'
VendorID	0x10CD, 'ADVANCED SYSTEM PRODUCTS'
VendorID	0x10CF, 'FUJITSU LIMITED'
VendorID	0x10DE, 'NVIDIA Corp.'
VendorID	0x10EC, 'REALTEK SEMICONDUCTOR Corp.'
VendorID	0x11AD, 'LITE-ON COMMUNICATIONS Inc.'
VendorID	0x10EE, 'XILINX'
VendorID	0x1102, 'Creative Labs'
VendorID	0x1105, 'SIGMA DESIGNS'
VendorID	0x1106, 'VIA TECHNOLOGIES'
VendorID	0x1114, 'Atmel Corp.'
VendorID	0x1131, 'PHILIPS SEMICONDUCTORS'
VendorID	0x1132, 'MITEL Corp.'
VendorID	0x1133, 'Eicon Networks Corp.'
VendorID	0x1134, 'MERCURY COMPUTER SYSTEMS'
VendorID	0x1135, 'FUJI XEROX Co., Ltd.'
VendorID	0x1137, 'CISCO SYSTEMS Inc.'
VendorID	0x1179, 'TOSHIBA AMERICA INFO SYSTEMS'
VendorID	0x1189, 'MATSUSHITA ELECTIC INDUSTRIAL Co., Ltd.'
VendorID	0x11AA, 'ACTEL'
VendorID	0x12D2, 'NVIDIA (WAS: STB,SGS THOMPSON)'
VendorID	0x11AB, 'GALILEO TECHNOLOGY Ltd./Marvell Semiconductor, Inc.'
VendorID	0x11B2, 'EASTMAN KODAK'
VendorID	0x11C1, 'LSI Corp.'
VendorID	0x11CA, 'LSI SYSTEMS'
VendorID	0x11D0, 'LOCKHEED MARTIN-Electronics & Communications'
VendorID	0x11D4, 'Analog Devices, Inc.'
VendorID	0x11D9, 'TOSHIBA TEC Corp.'
VendorID	0x11DA, 'NOVELL'
VendorID	0x11DB, 'Sega Enterprises Ltd.'
VendorID	0x119E, 'FUJITSU MICROELECTRONICS Ltd.'
VendorID	0x11A1, 'HAMAMATSU PHOTONICS K.K.'
VendorID	0x1217, 'O2Micro Inc.'
VendorID	0x1234, 'Technical Corp.'
VendorID	0x1250, 'Hitachi Microcomputer System Ltd.'
VendorID	0x122D, 'Aztech System Ltd.'
VendorID	0x1261, 'MATSUSHITA-KOTOBUKI ELECTRONICS'
VendorID	0x1260, 'INTERSIL Corp.'
VendorID	0x1274, 'Creative Labs (was Ensoniq, Malvern)'
VendorID	0x12BF, 'Fujifilm Microdevices'
VendorID	0x13B8, 'NOKIA TELECOMMUNICATIONS OY'
VendorID	0x13BD, 'SHARP Corp.'
VendorID	0x149A, 'ANDOR TECHNOLOGY Ltd.'
VendorID	0x149B, 'SEIKO INSTRUMENTS Inc.'
VendorID	0x14E4, 'Broadcom Corp.'
VendorID	0x1521, 'BELL Corp.'
VendorID	0x15AD, 'VMware Inc.'
VendorID	0x15AE, 'AMERSHAM PHARMACIA BIOTECH'
VendorID	0x1603, 'NOKIA WIRELESS BUSINESS COMMUNICATIONS'
VendorID	0x167B, 'ZyDAS Technology Corp.'
VendorID	0x168C, 'Atheros Communications Inc.'
VendorID	0x17F3, 'RDC Semiconductor Co., Ltd.'
VendorID	0x1814, 'Ralink Technology, Corp.'
VendorID	0x1969, 'Atheros Communications Inc.'
VendorID	0x197B, 'JMicron Technology Corp.'
VendorID	0x1AF4, 'Red Hat, Inc.'
VendorID	0x5333, 'S3 Graphics Co., Ltd.'
VendorID	0x6409, 'LOGITEC Corp.'
VendorID	0x8086, 'Intel Corp.'
VendorID	0x80EE, 'Oracle Corp.'
VendorID	0x8888, 'SILICON MAGIC Corp.'
VendorID	0x9004, 'Adaptec Inc.'
VendorID	0xA259, 'HEWLETT PACKARD'
VendorID	0xCA50, 'VARIAN AUSTRIALIA PTY Ltd.'
;include 'VENDORS.ADD'		; <-------- uncomment to insert the list of less known vendors
VendorID	0, 	'Unknown'	; <- terminator

;--------------------------------------
Classes:
; Class, Subclass, [Interface], Name
ClassID    0,    0,     , 'Prehistoric - misc'
ClassID    0,    1,     , 'Prehistoric - VGA'

ClassID    1,    0,     , 'Storage - SCSI'
ClassID    1,    1,     , 'Storage - IDE'	; intrface may be 0x80, 0x85 & 0x8A
ClassID    1,    1, 0x8A, 'Storage - IDE (prog. canals)'
ClassID    1,    2,     , 'Storage - Floppy'
ClassID    1,    3,     , 'Storage - IPI'
ClassID    1,    4,     , 'Storage - RAID'
ClassID    1,    5,     , 'Storage - ATA with DMA'
ClassID    1,    5, 0x20, 'Storage - ATA c. w/ single DMA'
ClassID    1,    5, 0x30, 'Storage - ATA c. w/ chained DMA'
ClassID    1,    6,    0, 'Storage - Serial ATA - vendor sp.'
ClassID    1,    6,    1, 'Storage - Serial ATA - AHCI 1.0'
ClassID    1,    7,     , 'Storage - SAS'
ClassID    1,    8,     , 'Storage - Non-Volatile MC'
ClassID    1, 0x80,     , 'Storage - misc'

ClassID    2,    0,     , 'Network - Ethernet'
ClassID    2,    1,     , 'Network - Token Ring'
ClassID    2,    2,     , 'Network - FDDI'
ClassID    2,    3,     , 'Network - ATM'
ClassID    2,    4,     , 'Network - ISDN'
ClassID    2,    5,     , 'Network - WorldFip'
ClassID    2,    6,     , 'Network - PICMG 2.14'
ClassID    2,    7,     , 'Network - InfiniBand c.'
ClassID    2, 0x80,     , 'Network - misc'

ClassID    3,    0,     , 'Display - VGA controller'
ClassID    3,    0,    0, 'Display - VGA-compatible c.'
ClassID    3,    0,    1, 'Display - 8514-compatible c.'
ClassID    3,    1,     , 'Display - XGA controller'
ClassID    3,    2,     , 'Display - 3D controller'
ClassID    3, 0x80,     , 'Display - misc'

ClassID    4,    0,     , 'Multimedia - Video'
ClassID    4,    1,     , 'Multimedia - Audio'
ClassID    4,    2,     , 'Multimedia - Computer telephony'
ClassID    4,    3,     , 'Multimedia - High Definition Audio'
ClassID    4, 0x80,     , 'Multimedia - misc'

ClassID    5,    0,     , 'Memory - RAM'
ClassID    5,    1,     , 'Memory - Flash memory'
ClassID    5, 0x80,     , 'Memory - misc'

ClassID    6,    0,     , 'Bridge - CPU/PCI'
ClassID    6,    1,     , 'Bridge - PCI/ISA'
ClassID    6,    2,     , 'Bridge - PCI/EISA'
ClassID    6,    3,     , 'Bridge - PCI/MCA'
ClassID    6,    4,     , 'Bridge - PCI/PCI'
ClassID    6,    4,    1, 'Bridge - Subtract.Decode PCI/PCI'
ClassID    6,    5,     , 'Bridge - PCMCIA'
ClassID    6,    6,     , 'Bridge - NuBus'
ClassID    6,    7,     , 'Bridge - CardBus'
ClassID    6,    8,     , 'Bridge - RACEway'
ClassID    6,    9,     , 'Bridge - Semi-transpar. PCI/PCI'
ClassID    6,    9, 0x40, 'Bridge - Semi-transp. PCI/PCI 1'
ClassID    6,    9, 0x80, 'Bridge - Semi-transp. PCI/PCI 2'
ClassID    6, 0x0a,     , 'Bridge - InfiniBand/PCI'
ClassID    6, 0x80,     , 'Bridge - misc'

ClassID    7,    0,     , 'Communication - Serial'
ClassID    7,    0,    0, 'Communication - Serial (XT)'
ClassID    7,    0,    1, 'Communication - Serial c.(16450)'
ClassID    7,    0,    2, 'Communication - Serial c.(16550)'
ClassID    7,    0,    3, 'Communication - Serial c.(16650)'
ClassID    7,    0,    4, 'Communication - Serial c.(16750)'
ClassID    7,    0,    5, 'Communication - Serial c.(16850)'
ClassID    7,    0,    6, 'Communication - Serial c.(16950)'
ClassID    7,    1,     , 'Communication - Parallel port'
ClassID    7,    1,    1, 'Communication - Bi-dir. par.port'
ClassID    7,    1,    2, 'Communication - ECP 1.X par.port'
ClassID    7,    1,    3, 'Communication - IEEE1284 c.'
ClassID    7,    1, 0xfe, 'Communication - IEEE1284 device'
ClassID    7,    2,     , 'Communication - Multiport Serial'
ClassID    7,    3,     , 'Communication - Modem'
ClassID    7,    3,    0, 'Communication - Generic modem'
ClassID    7,    3,    1, 'Communication - Hayes modem 16450'
ClassID    7,    3,    2, 'Communication - Hayes modem 16550'
ClassID    7,    3,    3, 'Communication - Hayes modem 16650'
ClassID    7,    3,    4, 'Communication - Hayes modem 16750'
ClassID    7,    4,     , 'Communication - GPIB'
ClassID    7,    5,     , 'Communication - Smart Card'
ClassID    7, 0x80,     , 'Communication - misc'

ClassID    8,    0,     , 'System - PIC'
ClassID    8,    0,    0, 'System - Generic 8259 PIC'
ClassID    8,    0,    1, 'System - ISA PIC'
ClassID    8,    0,    2, 'System - EISA PIC'
ClassID    8,    0, 0x10, 'System - I/O APIC interrupt c.'
ClassID    8,    0, 0x20, 'System - I/O(x) APIC interrupt c'
ClassID    8,    1,     , 'System - DMAC'
ClassID    8,    1,    0, 'System - Generic 8237 DMA c.'
ClassID    8,    1,    1, 'System - ISA DMA c.'
ClassID    8,    1,    2, 'System - EISA DMA c.'
ClassID    8,    2,     , 'System - Timer'
ClassID    8,    2,    0, 'System - 8254 system timer'
ClassID    8,    2,    1, 'System - ISA system timer'
ClassID    8,    2,    2, 'System - EISA (2 system timers)'
ClassID    8,    3,     , 'System - RTC'
ClassID    8,    3,    0, 'System - Generic RTC c.'
ClassID    8,    3,    1, 'System - ISA RTC c.'
ClassID    8,    4,     , 'System - PCI Hot-Plug'
ClassID    8,    5,     , 'System - SDHCI'
ClassID    8,    6,     , 'System - IOMMU'
ClassID    8, 0x80,     , 'System - misc'

ClassID    9,    0,     , 'Input - Keyboard'
ClassID    9,    1,     , 'Input - Digitazer (pen)'
ClassID    9,    2,     , 'Input - Mouse'
ClassID    9,    3,     , 'Input - Scanner'
ClassID    9,    4,     , 'Input - Gameport'
ClassID    9,    4,    0, 'Input - Gameport  c. (generic)'
ClassID    9,    4, 0x10, 'Input - Gameport c. (legacy)'
ClassID    9, 0x80,     , 'Input - misc'

ClassID 0x0a,    0,     , 'Docking - Generic station'
ClassID 0x0a, 0x80,     , 'Docking - misc'

ClassID 0x0b,    0,     , 'Processor - 386'
ClassID 0x0b,    1,     , 'Processor - 486'
ClassID 0x0b,    2,     , 'Processor - Pentium'
; ClassID 0x0b, 0x10,     , 'Processor - Alpha'		;
; ClassID 0x0b, 0x20,     , 'Processor - PowerPC'		; it`s realy need for PCIDEV???
; ClassID 0x0b, 0x30,     , 'Processor - MIPS'		;
ClassID 0x0b, 0x40,     , 'Processor - Co-processor'

ClassID 0x0c,    0,     , 'Serial Bus - FireWire'
ClassID 0x0c,    0,    0, 'Serial Bus - IEEE 1394(FireWire)'
ClassID 0x0c,    0, 0x10, 'Serial Bus - IEEE 1394(Open HCI)'
ClassID 0x0c,    1,     , 'Serial Bus - ACCESS.bus'
ClassID 0x0c,    2,     , 'Serial Bus - SSA'
ClassID 0x0c,    3,     , 'Serial Bus - USB/USB2'
ClassID 0x0c,    3,    0, 'Serial Bus - USB Universal HC'
ClassID 0x0c,    3, 0x10, 'Serial Bus - USB Open HC'
ClassID 0x0c,    3, 0x20, 'Serial Bus - USB2 Enhanced HC'
ClassID 0x0c,    3, 0x80, 'Serial Bus - USB w/o specific PI'
ClassID 0x0c,    3, 0xfe, 'Serial Bus - USB device (not HC)'
ClassID 0x0c,    4,     , 'Serial Bus - Fibre Channel'
ClassID 0x0c,    5,     , 'Serial Bus - SMBus'
ClassID 0x0c,    6,     , 'Serial Bus - InfiniBand'
ClassID 0x0c,    7,     , 'Serial Bus - IPMI'
ClassID 0x0c,    7,    0, 'Serial Bus - IPMI SMIC I.'
ClassID 0x0c,    7,    1, 'Serial Bus - IPMI Kybd CSI'
ClassID 0x0c,    7,    2, 'Serial Bus - IPMI BTI'
ClassID 0x0c,    8,     , 'Serial Bus - SERCOS'
ClassID 0x0c,    9,     , 'Serial Bus - CANbus'

ClassID 0x0d,    0,     , 'Wireless - iRDA'
ClassID 0x0d,    1,     , 'Wireless - Consumer IR'
ClassID 0x0d, 0x10,     , 'Wireless - RF'
ClassID 0x0d, 0x11,     , 'Wireless - Bluetooth'
ClassID 0x0d, 0x12,     , 'Wireless - Broadband'
ClassID 0x0d, 0x20,     , 'Wireless - Ethernet (802.11a)'
ClassID 0x0d, 0x21,     , 'Wireless - Ethernet (802.11b)'
ClassID 0x0d, 0x80,     , 'Wireless - misc'

ClassID 0x0e,    0,     , 'Intelligent I/O - c.(FIFO @ 40h)'
ClassID 0x0e,    0,    0, 'Intelligent I/O - I/O c. (I2O 1)'

ClassID 0x0f,    1,     , 'Satellite Communication - TV'
ClassID 0x0f,    2,     , 'Satellite Communication - Audio'
ClassID 0x0f,    3,     , 'Satellite Communication - Voice'
ClassID 0x0f,    4,     , 'Satellite Communication - Data'

ClassID 0x10,    0,     , 'En/Decryption - Network&comput.'
ClassID 0x10, 0x10,     , 'En/Decryption - Entertainment'	; 0x10, 0x01,, ???
ClassID 0x10, 0x80,     , 'En/Decryption - misc'

ClassID 0x11,    0,     , 'DSP - DPIO modules'
ClassID 0x11,    1,     , 'DSP - Performance counters'
ClassID 0x11, 0x10,     , 'DSP - Sync./Test/Measurement'
ClassID 0x11, 0x20,     , 'DSP - Management card'
ClassID 0x11, 0x80,     , 'DSP - misc'

ClassID 0xff, 0xff,     , 'Unknown'	; <- terminator
