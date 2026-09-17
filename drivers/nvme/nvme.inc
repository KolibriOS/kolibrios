;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

define LOG2 bsr

; NVMe Controller Versions
VS100 = 0x00010000 ; (v1.0.0)
VS110 = 0x00010100 ; (v1.1.0)
VS120 = 0x00010200 ; (V1.2.0)
VS121 = 0x00010201 ; (v1.2.1)
VS130 = 0x00010300 ; (v1.3.0)
VS140 = 0x00010400 ; (v1.4.0)

NVM_MPS = 0                   ; Memory Page Size (2 ^ (12 + MPS))
NVM_ASQS = 64                 ; Admin Submission Queue Size
NVM_ACQS = NVM_ASQS           ; Admin Completion Queue Size
LAST_QUEUE_ID = 1             ; Index of the last queue
SQ_ENTRIES = NVM_ASQS         ; I/O and Admin Submission Queue Size
CQ_ENTRIES = NVM_ACQS         ; I/O and Admin Completion Queue Size
PAGE_SIZE = 4096 shl NVM_MPS  ; Use 4KiB pages
SUPPORTED_LBADS = 9           ; KolibriOS only supports LBADS of 512, later on we may remove this restriction
SQ_ALLOC_SIZE = 0x1000
CQ_ALLOC_SIZE = 0x1000
QUEUE_ALLOC_SIZE = SQ_ALLOC_SIZE + CQ_ALLOC_SIZE

; Upper bound on the number of memory pages a single I/O command may span. A PRP
; list holds PAGE_SIZE/8 entries, so staying well below that keeps every transfer
; describable by one non-chained PRP list.
MAX_PRP_PAGES = 256

PCI_MAX_CAPS = 48 ; a config space holds at most 48 capabilities

; How long to wait on the controller, in milliseconds.
CTRL_POLL_MS = 10
SHUTDOWN_WAIT_MS = 5000
IO_TIMEOUT_MS = 30000 ; how long a single I/O command may take
POLL_SPINS = 64 ; lock checks between two looks at the completion queue
IO_BUSY_ROUNDS = 20000 ; rounds to spend spinning before falling back to sleeping

MSIXCAP_CID = 0x11
MSIXCAP_MXE = 1 shl 15 ; MSI-X Enable bit
MSICAP_CID = 0x05
MSICAP_MSIE = 1 ; MSI Enable bit

ADMIN_QUEUE = 0 ; Admin Queue ID

IEN_ON = 2
IEN_OFF = 0

; Opcodes for NVM commands
NVM_CMD_FLUSH = 0x00 
NVM_CMD_WRITE = 0x01
NVM_CMD_READ = 0x02
NVM_CMD_WRITE_UNCORRECTABLE = 0x04
NVM_CMD_COMPARE = 0x05
NVM_CMD_WRITE_ZEROES = 0x08
NVM_CMD_DATASET_MANAGEMENT = 0x09
NVM_CMD_VERIFY = 0x0C
NVM_CMD_RESERVATION_REG = 0x0D
NVM_CMD_RESERVATION_REPORT = 0x0E
NVM_CMD_RESERVATION_ACQUIRE = 0x11
NVM_CMD_RESERVATION_RELEASE = 0x15
NVM_CMD_COPY = 0x19

; Opcodes for admin commands (Page 94 of NVMe 1.4 spec)
ADM_CMD_DEL_IO_SUBMISSION_QUEUE = 0x00
ADM_CMD_CRE_IO_SUBMISSION_QUEUE = 0x01
ADM_CMD_GET_LOG_PAGE = 0x02
ADM_CMD_DEL_IO_COMPLETION_QUEUE = 0x04
ADM_CMD_CRE_IO_COMPLETION_QUEUE = 0x05
ADM_CMD_IDENTIFY = 0x06
ADM_CMD_ABORT = 0x08
ADM_CMD_SET_FEATURES = 0x09
ADM_CMD_GET_FEATURES = 0x0A

; fuse (fused operation): In a fused operation, a complex command is created by 'fusing' together
;                         two simpler commands. This field specifies whether this command is part
;                         of a fused operation, and if so, which command it is in the sequence:
;                         00b -> Normal operation
;                         01b -> Fused operation, first command
;                         10b -> Fused operation, second command
;                         11b -> Reserved
NO_FUSE = 0
FUSE_OP_FIRST_CMD = 1 shl 8
FUSE_OP_SECOND_CMD = 2 shl 8

; sel (PRP or SGL for data transfer): This field specifies whether PRPs or SGLs are used for any
;                                     data transfer associated with the command. PRPs shall be
;                                     used for all Admin commands for NVMe over PCIe implementations.
;                                     SGLs shall be used for all Admin and I/O commands for NVMe over
;                                     Fabrics implementations (i.e., field set to 01b):
;                                     00b -> PRPs are used for this transfer
;                                     01b -> SGLs are used for this transfer, MPTR will contain address of 
;                                            a single contiguous physical buffer that is byte aligned
;                                     10b -> SGLs are used for this transfer. MPTR will contain address of
;                                            an SGL segment containing exactly one SGL descriptor that is
;                                            QWORD aligned
;                                     11b -> Reserved
SEL_PRP = 0
SEL_SGL = 1 shl 14

; Controller or Namespace Structure (CNS) specifies the information to be returned to the host.
CNS_IDNS   = 0x0  ; Namespace data structure (NSID)
CNS_IDCS   = 0x1  ; Controller data structure
CNS_ANIDL  = 0x2  ; Active namespace ID list (NSID)
CNS_NIDL   = 0x3  ; Namespace identification descriptor list (NSID)
CNS_NVM_SL = 0x4  ; NVM Set List 

; Optional Admin Command Support (OACS) values
OACS_SEC_SEN_RECV_SUPPORTED = 1 shl 0
OACS_FMT_NVM_SUPPORTED = 1 shl 1
OACS_FIRM_COMDL_SUPPORTED = 1 shl 2
OACS_NSMAN_SUPPORTED = 1 shl 3

; scope is all attached namespaces or all namespaces in NVM subsystem
NSID_BROADCAST = 0xFFFFFFFF

NSSRC_RESET = 0x4E564D65 ; "NVMe" (initiates a NVMe subsystem reset)

; NVMe Capabilities
CAP_MQES = 0xffff
CAP_CQR = 1 shl 16
CAP_AMS = (1 shl 17) or (1 shl 18)
CAP_TO = 0xff000000
CAP_DSTRD = 1 or (1 shl 1) or (1 shl 2) or (1 shl 3)
CAP_NSSRS = 1 shl 4
CAP_CSS_NVM_CMDSET = 1 shl 5
CAP_CSS_NOIO = 1 shl 12
CAP_BPS = 1 shl 13
CAP_CPS_COSCOP = 1 shl 15
CAP_CPS_DOSCOP = 1 shl 16
CAP_CPS_NVMSCOP = CAP_CPS_COSCOP or CAP_CPS_DOSCOP
CAP_MPSMIN = 0xf shl 16 ; CAP bits 51:48
CAP_MPSMAX = 0xf shl 20 ; CAP bits 55:52
CAP_PMRS = 1 shl 24
CAP_CMBS = 1 shl 25
CAP_NSSS = 1 shl 27
CAP_CRMS_CRWMS = 1 shl 28
CAP_CRMS_CRIMS = 1 shl 29

; Controller Configuration Bits
CC_EN = 1
CC_CSS = (1 shl 4) or (1 shl 5) or (1 shl 6)
CC_MPS = (1 shl 7) or (1 shl 8) or (1 shl 9) or (1 shl 10)
CC_AMS = (1 shl 11) or (1 shl 12) or (1 shl 13)
CC_SHN = (1 shl 14) or (1 shl 15)
CC_IOSQES = (1 shl 16) or (1 shl 17) or (1 shl 18) or (1 shl 19)
CC_IOCQES = (1 shl 20) or (1 shl 21) or (1 shl 22) or (1 shl 23)
CC_CRIME = 1 shl 24

CC_SHN_NORMAL_SHUTDOWN = 1 shl 14
CC_SHN_ABRUPT_SHUTDOWN = 1 shl 15

CC_DEFAULT_IOSQES = LOG2 sizeof.SQ_ENTRY shl 16
CC_DEFAULT_IOCQES = LOG2 sizeof.CQ_ENTRY shl 20

; Completion Queue Entry Status Field Values
CQ_PHASE_TAG = 1 shl 0
CQ_STATUS_SC = 0xfe
CQ_STATUS_SCT = (1 shl 9) or (1 shl 10) or (1 shl 11)
CQ_STATUS_CRD = (1 shl 12) or (1 shl 13)
CQ_STATUS_M = 1 shl 14
CQ_STATUS_DNR = 1 shl 15

; Completion Queue Entry Status Field - Status Code Type Values
CQ_STATUS_SCT_GCS   = 0x0 ; Generic Command Status
CQ_STATUS_SCT_CSS   = 0x1 ; Command Specific Status
CQ_STATUS_SCT_MADIE = 0x2 ; Media and Data Integrity Errors
CQ_STATUS_SCT_PRS   = 0x3 ; Path Related Status

; Completion Queue Entry Status Field - Status Code Generic Command Values
CQ_STATUS_SC_GCS_SUCCESS = 0x00 ; Successful Completion
CQ_STATUS_SC_GCS_ICOP    = 0x01 ; Invalid Command Opcode
CQ_STATUS_SC_GCS_IFIC    = 0x02 ; Invalid Field in Command
CQ_STATUS_SC_GCS_CIDC    = 0x03 ; Command ID Conflict
CQ_STATUS_SC_GCS_DTE     = 0x04 ; Data Transfer Error
CQ_STATUS_SC_GCS_CAPLN   = 0x05 ; Commands Aborted due to Power Loss Notification
CQ_STATUS_SC_GCS_INERR   = 0x06 ; Internal Error
CQ_STATUS_SC_GCS_CAR     = 0x07 ; Command Abort Requested
CQ_STATUS_SC_GCS_CASQD   = 0x08 ; Command Aborted due to SQ Deletion
CQ_STATUS_SC_GCS_CAFFC   = 0x09 ; Command Aborted due to Failed Fused Command
CQ_STATUS_SC_GCS_CAMFC   = 0x0A ; Command Aborted due to Missing Fused Command
CQ_STATUS_SC_GCS_INNOF   = 0x0B ; Invalid Namespace or Format
CQ_STATUS_SC_GCS_CSE	 = 0x0C ; Command Sequence Error
CQ_STATUS_SC_GCS_INSGL   = 0x0D ; Invalid SGL Segment Descriptor
CQ_STATUS_SC_GCS_INNSGL  = 0x0E ; Invalid Number of SGL Descriptors
CQ_STATUS_SC_GCS_OPDEN   = 0x15 ; Operation Denied
CQ_STATUS_SC_GCS_NSIWP   = 0x20 ; Namespace is Write Protected
CQ_STATUS_SC_GCS_CINT    = 0x21 ; Command Interrupted
CQ_STATUS_SC_GCS_TTE     = 0x22 ; Transient Transport Error

; Completion Queue Entry Status Field - Status Code Media and Data Integrity Errors
CQ_STATUS_SC_MADIE_WF    = 0x80 ; Write Fault
CQ_STATUS_SC_MADIE_URE   = 0x81 ; Unrecovered Read Error
CQ_STATUS_SC_MADIE_ACDEN = 0x86 ; Access Denied
CQ_STATUS_SC_MADIE_DOULB = 0x87 ; Deallocated or Unwritten Logical Block

; Controller Status (CSTS) Values
CSTS_RDY = 1
CSTS_CFS = 1 shl 1
CSTS_SHST = (1 shl 2) or (1 shl 3)
CSTS_NSSRO = 1 shl 4
CSTS_PP = 1 shl 5
CSTS_SHST_SHUTDOWN_OCCURRING = 1 shl 2
CSTS_SHST_SHUTDOWN_COMPLETE = 1 shl 3

; Admin Queue Attributes (AQA) Values
AQA_ASQS = 0xfff
AQA_ACQS = 0xfff shl 16

; CDW10.SEL Values (Page 115 of NVMe 1.4 specification)
CDW10_SEL_CURRENT = 000b
CDW10_SEL_DEFAULT = 001b
CDW10_SEL_SAVED = 010b
CDW10_SEL_SUPPORTED_CAPABILITIES = 011b

; Feature Identifiers (FID) Values (Page 206 of NVMe 1.4 specification)
; Used in Get/Set Features Commands
FID_ARBITRATION = 0x01
FID_POWER_MANAGEMENT = 0x02
FID_LBA_RANGE_TYPE = 0x03
FID_TEMPERATURE_THRESHOLD = 0x04
FID_ERROR_RECOVERY = 0x05
FID_VOLATILE_WRITE_CACHE = 0x06
FID_NUMBER_OF_QUEUES = 0x07
FID_INTERRUPT_COALESCING = 0x08
FID_INTERRUPT_VECTOR_CONFIGURATION = 0x09
FID_WRITE_ATOMICITY_NORMAL = 0x0A
FID_ASYNCHRONOUS_EVENT_CONFIGURATION = 0x0B
FID_AUTONOMOUS_POWER_STATE_TRANSITION = 0x0C
FID_HOST_MEMORY_BUFFER = 0x0D
FID_TIMESTAMP = 0x0E
FID_KEEP_ALIVE_TIMER = 0x0F
FID_HOST_CONTROLLED_THERMAL_MANAGEMENT = 0x10
FID_NON_OPERATIONAL_POWER_STATE_CONFIG = 0x11
FID_READ_RECOVERY_LEVEL_CONFIG = 0x12
FID_PREDICTABLE_LATENCY_MODE_CONFIG = 0x13
FID_PREDICTABLE_LATENCY_MODE_WINDOW = 0x14
FID_LBA_STATUS_INFORMATION_REPORT_INTERVAL = 0x15
FID_HOST_BEHAVIOR_SUPPORT = 0x16
FID_SANITIZE_CONFIG = 0x17
FID_ENDURANCE_GROUP_EVENT_CONFIGURATION = 0x18
; NVM Command Set Specific - FID
FID_SOFTWARE_PROGRESS_MARKER = 0x80
FID_HOST_IDENTIFIER = 0x81
FID_RESERVATION_NOTIFICATION_MASK = 0x82
FID_RESERVATION_PERSISTENCE = 0x83
FID_NAMESPACE_WRITE_PROTECTION_CONFIG = 0x84

; Get Log Page - Log Page Identifiers (Page 118-119 of NVMe 1.4 specification)
LID_ERROR_INFORMATION = 0x01
LID_SMARTHEALTH_INFORMATION = 0x02
LID_FIRMWARE_SLOT_INFORMATION = 0x03
LID_CHANGED_NAMESPACE_LIST = 0x04
LID_COMMANDS_SUPPORTED_AND_EFFECTS = 0x05
LID_DEVICE_SELF_TEST = 0x06
LID_TELEMETRY_HOST_INITIATED = 0x07
LID_TELEMETRY_CONTROLLER_INITIATED = 0x08
LID_ENDURANCE_GROUP_INFORMATION = 0x09
LID_PREDICTABLE_LATENCY_PER_NVM_SET = 0x0A
LID_PREDICTABLE_LATENCY_EVENT_AGGREGATE = 0x0B
LID_ASYMMETRIC_NAMESPACE_ACCESS = 0x0C
LID_PERSISTENT_EVENT_LOG = 0x0D
LID_LBA_STATUS_INFORMATION = 0x0E
LID_ENDURANCE_GROUP_EVENT_AGGREGATE = 0x0F
; I/O Command Set Specific - Log Page Identifiers
LID_RESERVATION_NOTIFICATION = 0x80
LID_SANITIZE_STATUS = 0x81

; Controller Type Values
CNTRLTYPE_IO_CONTROLLER = 0x1
CNTRLTYPE_DISCOVERY_CONTROLLER = 0x2
CNTRLTYPE_ADMIN_CONTROLLER = 0x3

struct NVME_MMIO 
	CAP      dq ? ; Controller Capabilities
	VS       dd ? ; Version
	INTMS    dd ? ; Interrupt Mask Set
	INTMC    dd ? ; Interrupt Mask Clear
	CC       dd ? ; Controller Configuration
	         rd 1 ; Reserved
	CSTS     dd ? ; Controller Status
	NSSR     dd ? ; NVM Subsystem Reset
	AQA      dd ? ; Admin Queue Attributes
	ASQ      dq ? ; Admin Submission Queue Base Address
	ACQ      dq ? ; Admin Completion Queue Base Address
	CMBLOC   dd ? ; Controller Memory Buffer Location
	CMBSZ    dd ? ; Controller Memory Buffer Size
	BPINFO   dd ? ; Boot Partition Information
	BPRSEL   dd ? ; Boot Partition Read Select
	BPMBL    dq ? ; Boot Partition Memory Buffer Location
	CMBMSC   dd ? ; Controller Memory Buffer Memory Space
	CMBSTS   dd ? ; Controller Memory Buffer Status
	    	 rb 3492 ; Reserved
	PMRCAP   dd ? ; Persistent Memory Capabilities
	PMRCTL   dd ? ; Persistent Memory Region Control
	PMRSTS   dd ? ; Persistent Memory Region Status
	PMREBS   dd ? ; Persistent Memory Region Elasticity Buffer Size
	PMRSWTP  dd ? ; Persistent Memory Region Sustained Write Throughput
	PMRMSC   dq ? ; Persistent Memory Region Controller Memory Space Control
	    	 rb 484 ; Reserved
	SQ0TDBL  dd ? ; Submission Queue 0 Tail Doorbell (Admin)
ends


; Submission Queue Entry (64 bytes)
struct SQ_ENTRY
	cdw0 	    	dd ?
	nsid 	    	dd ?
	cdw2            dd ?
	cdw3            dd ?
	mptr	    	dq ?
	prp1	    	dq ?
	prp2            dq ?
	cdw10	    	dd ?
	cdw11	    	dd ?
	cdw12	    	dd ?
	cdw13	    	dd ?
	cdw14	    	dd ?
	cdw15	    	dd ?
ends

; Completion Queue Entry (16 bytes) - See page 77 of the NVMe 1.4 spec
struct CQ_ENTRY 
	cdw0 	    	dd ?
	    		rd 1 ; reserved
	sqhd            dw ?
	sqid            dw ?
	cid             dw ?
	status 	    	dw ?
ends

struct NSINFO
	capacity 	dq ?
	size 	    	dq ?
	nsid 	    	dd ?
	pci 	    	dd ?
	lbads 	    	db ?
	features 	db ?
ends

struct pcidev 
	bus 	    	db ?
	devfn 	    	db ?
	ipin 	    	db ?
	iline 	        db ?
	num 	    	dd ?
	io_addr 	dd ?
	queue_entries 	dd ?
	version 	dd ?
	nsid 	    	dd ?
	spinlock 	dd ?
	nsinfo 	    	dd ?
	cqlock 	    	dd ? ; held by whoever is draining the completion queues
	last_status 	dd ? ; status field of the last completion, 0 when all went well
	nn 	    	dd ?
	max_sectors 	dd ? ; biggest transfer a single I/O command may carry
	dstrd 	    	db ?
	mdts 	    	db ? ; Maximum Data Transfer Size, in 2^(12+MPSMIN) byte units
	    		rb 2 ; align
ends
TOTAL_PCIDEVS = 4
TOTAL_PCIDEVS_MALLOC_SZ = TOTAL_PCIDEVS * sizeof.pcidev

struct NVMQCMD
	cid 		dd ?
	mutex_ptr 	MUTEX
ends

struct NVM_QUEUE_ENTRY
	tail 	    	dw ?
	head 	    	dw ?
	sq_ptr 	    	dd ?
	cq_ptr 	    	dd ?
	cmd_ptr 	dd ?
	phase 	    	db ? ; phase tag the next completion of this queue will carry
	    		rb 15 ; pad to a power of two, the queue index is shifted by LOG2 of it
ends

; Identify Controller Data Structure
struct IDENTC

	vid	   dw ?
	ssvid      dw ?
	sn         dt ?, ?
	mn         rt 4
	fr         dq ?
	rab        db ?
	ieee       db ?, ?, ?
	cmic       db ?
	mdts       db ?
	cntlid     dw ?
	ver        dd ?
	rtd3r      dd ?
	rtd3e      dd ?
	oaes 	   dd ?
	ctratt     dd ?
	rrls       dw ?
	           rb 9       ; reserved
	cntrltype  db ?
	fguid      dq ?, ?
	crdt1      dw ?
	crdt2      dw ?
	crdt3      dw ?
	           rb 106     ; reserved
	           rb 16      ; reserved (NVMMI)
	oacs 	   dw ?
	acl 	   db ?
	aerl 	   db ?
	frmw 	   db ?
	lpa 	   db ?
	elpe 	   db ?
	npss 	   db ?
	avscc 	   db ?
	apsta 	   db ?
	wctemp 	   dw ?
	cctemp 	   dw ?
	mtfa 	   dw ?
	hmpre 	   dd ?
	hmmin 	   dd ?
	tnvmcap    dq ?, ?
	unvmcap    dq ?, ?
	rpmbs 	   dd ?
	edstt 	   dw ?
	dsto 	   db ?
	fwug 	   db ?
	kas 	   dw ?
	hctma 	   dw ?
	mntmt 	   dw ?
	mxtmt 	   dw ?
	sanicap    dd ?
	hmminds    dd ?
	hmmaxd     dw ?
	nsetidmax  dw ?
	endgidmax  dw ?
	anatt      db ?
	anacap 	   db ?
	anagrpmax  dd ?
	nanagrpid  dd ?
	pels 	   dd ?
	    	   rb 156
	sqes  	   db ?
	cqes 	   db ?
	maxcmd 	   dw ?
	nn         dd ?
	oncs 	   dw ?
	fuses  	   dw ?
	fna 	   db ?
	vwc 	   db ?
	awun 	   dw ?
	awupf 	   dw ?
	nvscc  	   db ?
	nwpc 	   db ?
	acwu 	   dw ?
	    	   rb 2
	sgls 	   dd ?
	mnan 	   dd ?
	    	   rb 224
	subnqn 	   rq 32
	    	   rb 768
	    	   rb 256
	psd0 	   rq 4
	psd1	   rq 4
	psd2  	   rq 4
	psd3  	   rq 4
	psd4 	   rq 4
	psd5 	   rq 4
	psd6       rq 4
	psd7  	   rq 4
	psd8  	   rq 4
	psd9 	   rq 4
	psd10 	   rq 4
	psd11 	   rq 4
	psd12 	   rq 4
	psd13      rq 4
	psd14 	   rq 4
	psd15  	   rq 4
	psd16 	   rq 4
	psd17  	   rq 4
	psd18  	   rq 4
	psd19  	   rq 4
	psd20  	   rq 4
	psd21 	   rq 4
	psd22 	   rq 4
	psd23 	   rq 4
	psd24  	   rq 4
	psd25 	   rq 4
	psd26 	   rq 4
	psd27 	   rq 4
	psd28 	   rq 4
	psd29 	   rq 4
	psd30 	   rq 4
	psd31 	   rq 4
	    	   rb 1024
ends

; Identify Namespace Data Structure
struct IDENTN
	nsze 	   dq ?
	ncap 	   dq ?
	nuse  	   dq ?
	nsfeat     db ?
	nlbaf 	   db ?
	flbas 	   db ?
	mc 	   db ?
	dpc 	   db ?
	dps 	   db ?
	nmic 	   db ?
	rescap 	   db ?
	fpi 	   db ?
	dlfeat 	   db ?
	nawun 	   dw ?
	nawupf 	   dw ?
	nacwu 	   dw ?
	nabsn      dw ?
	nabo 	   dw ?
	nabspf 	   dw ?
	noiob 	   dw ?
	nvmcap 	   dq ?
	           dq ?
	npwg 	   dw ?
	npwa 	   dw ?
	npdg 	   dw ?
	npda 	   dw ?
	nows 	   dw ?
	           rb 18
	anagrpid   dd ?
	           rb 3
	nsattr 	   db ?
	nvmsetid   dw ?
	endgid 	   dw ?
	nguid 	   dq ?
	           dq ?
	eui64 	   dq ?
	lbaf0	   dd ?
	lbaf1 	   dd ? 
	lbaf2 	   dd ?
	lbaf3	   dd ?
	lbaf4	   dd ?
	lbaf5	   dd ?
	lbaf6	   dd ?
	lbaf7	   dd ?
	lbaf8	   dd ?
	lbaf9	   dd ?
	lbaf10	   dd ?
	lbaf11	   dd ?
	lbaf12	   dd ?
	lbaf13	   dd ?
	lbaf14	   dd ?
	lbaf15	   dd ?
	           rb 3904
ends

; Namespace Granularity List (CNS 16h - Page 199 of NVMe specification 1.4)
struct NSGRANLS 
	
	nga 	   dd ?
	nod 	   db ?
	    	   rb 27 ; reserved
	ngd0 	   dq ?, ?
	ngd1       dq ?, ?
	ngd2       dq ?, ?
	ngd3       dq ?, ?
	ngd4       dq ?, ?
	ngd5       dq ?, ?
	ngd6       dq ?, ?
	ngd7       dq ?, ?
	ngd8       dq ?, ?
	ngd9       dq ?, ?
	ngd10      dq ?, ?
	ngd11      dq ?, ?
	ngd12      dq ?, ?
	ngd13      dq ?, ?
	ngd14      dq ?, ?
	ngd15      dq ?, ?

ends

assert NVM_ASQS = NVM_ACQS
assert SQ_ENTRIES = NVM_ASQS
assert CQ_ENTRIES = NVM_ACQS
assert NVM_MPS = 0
assert PAGE_SIZE = 0x1000
assert sizeof.NVME_MMIO = 4096
assert sizeof.SQ_ENTRY = 64
assert sizeof.CQ_ENTRY = 16
assert sizeof.IDENTC = 4096
assert sizeof.IDENTN = 4096
assert sizeof.NSGRANLS = 288
assert sizeof.NVMQCMD = 16
assert sizeof.NVM_QUEUE_ENTRY = 32

; vim: syntax=fasm
