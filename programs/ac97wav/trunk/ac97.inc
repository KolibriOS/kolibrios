; Vendor ids
INTEL_VID      =   0x8086
SIS_VID        =   0x1039
NVIDIA_VID     =   0x10DE
AMD_VID        =   0x1022

; Device ids
ICH_DID        =   0x2415
ICH0_DID       =   0x2425
ICH2_DID       =   0x2445
ICH3_DID       =   0x2485
ICH4_DID       =   0x24C5
ICH5_DID       =   0x24D5
MX440_DID      =   0x7195
SI7012_DID     =   0x7012
NFORCE_DID     =   0x01B1
NFORCE2_DID    =   0x006A
AMD8111_DID    =   0x764D
AMD768_DID     =   0x7445

NAMBAR_REG     =   0x10            ; native audio mixer BAR
 NAM_SIZE      =   256             ; 256 bytes required.

NABMBAR_REG    =   0x14            ; native audio bus mastering BAR
 NABM_SIZE     =   64              ; 64 bytes

IRQ_REG        =   0x3c            ; IRQ holder for PCI
INT_REG        =   0x3d            ; INT pin
ICH4_CFG_REG   =   0x41            ; ICH4 config register


; BUS master registers, accessed via NABMBAR+offset

; ICH supports 3 different types of register sets for three types of things
; it can do, thus:
;
; PCM in (for recording) aka PI
; PCM out (for playback) aka PO
; MIC in (for recording) aka MC

PI_BDBAR_REG            =     0       ; PCM in buffer descriptor BAR
PO_BDBAR_REG            =     10h     ; PCM out buffer descriptor BAR
MC_BDBAR_REG            =     20h     ; MIC in buffer descriptor BAR

; each buffer descriptor BAR holds a pointer which has entries to the buffer
; contents of the .WAV file we're going to play.  Each entry is 8 bytes long
; (more on that later) and can contain 32 entries total, so each BAR is
; 256 bytes in length, thus:

BDL_SIZE                =     32*8    ; Buffer Descriptor List size
INDEX_MASK              =     31      ; indexes must be 0-31



PI_CIV_REG              =     4       ; PCM in current Index value (RO)
PO_CIV_REG              =     14h     ; PCM out current Index value (RO)
MC_CIV_REG              =     24h     ; MIC in current Index value (RO)

;8bit read only
; each current index value is simply a pointer showing us which buffer
; (0-31) the codec is currently processing.  Once this counter hits 31, it
; wraps back to 0.
; this can be handy to know, as once it hits 31, we're almost out of data to
; play back or room to record!


PI_LVI_REG              =     5       ; PCM in Last Valid Index
PO_LVI_REG              =     15h     ; PCM out Last Valid Index
MC_LVI_REG              =     25h     ; MIC in Last Valid Index
;8bit read/write
; The Last Valid Index is a number (0-31) to let the codec know what buffer
; number to stop on after processing.  It could be very nasty to play audio
; from buffers that aren't filled with the audio we want to play.


PI_SR_REG               =     6       ; PCM in Status register
PO_SR_REG               =     16h     ; PCM out Status register
MC_SR_REG               =     26h     ; MIC in Status register
;16bit read/write
; status registers.  Bitfields follow:

FIFO_ERR                =     BIT4    ; FIFO Over/Underrun W1TC.

BCIS                    =     BIT3    ; buffer completion interrupt status.
                                        ; Set whenever the last sample in ANY
                                        ; buffer is finished.  Bit is only
                                        ; set when the Interrupt on Complete
                                        ; (BIT4 of control reg) is set.

LVBCI                   =     BIT2    ; Set whenever the codec has processed
                                        ; the last buffer in the buffer list.
                                        ; Will fire an interrupt if IOC bit is
                                        ; set. Probably set after the last
                                        ; sample in the last buffer is
                                        ; processed.  W1TC


CELV                    =     BIT1    ; Current buffer == last valid.
                                        ; Bit is RO and remains set until LVI is
                                        ; cleared.  Probably set up the start
                                        ; of processing for the last buffer.


DCH                     =     BIT0    ; DMA controller halted.
                                        ; set whenever audio stream is stopped
                                        ; or something else goes wrong.


PI_PICB_REG             =     8       ; PCM in position in current buffer(RO)
PO_PICB_REG             =     18h     ; PCM out position in current buffer(RO)
MC_PICB_REG             =     28h     ; MIC in position in current buffer (RO)
;16bit read only
; position in current buffer regs show the number of dwords left to be
; processed in the current buffer.
;





PI_PIV_REG              =     0ah     ; PCM in Prefected index value
PO_PIV_REG              =     1ah     ; PCM out Prefected index value
MC_PIV_REG              =     2ah     ; MIC in Prefected index value
;8bit, read only
; Prefetched index value register.
; tells which buffer number (0-31) has be prefetched.  I'd imagine this
; value follows the current index value fairly closely. (CIV+1)
;


PI_CR_REG               =     0bh     ; PCM in Control Register
PO_CR_REG               =     1bh     ; PCM out Control Register
MC_CR_REG               =     2bh     ; MIC in Control Register
; 8bit
; Control register *MUST* only be accessed as an 8bit value.
; Control register.  See bitfields below.
;


IOCE                    =     BIT4    ; interrupt on complete enable.
                                        ; set this bit if you want an intrtpt
                                        ; to fire whenever LVBCI is set.
FEIFE                   =     BIT3    ; set if you want an interrupt to fire
                                        ; whenever there is a FIFO (over or
                                        ; under) error.
LVBIE                   =     BIT2    ; last valid buffer interrupt enable.
                                        ; set if you want an interrupt to fire
                                        ; whenever the completion of the last
                                        ; valid buffer.
RR                      =     BIT1    ; reset registers.  Nukes all regs
                                        ; except bits 4:2 of this register.
                                        ; Only set this bit if BIT 0 is 0
RPBM                    =     BIT0    ; Run/Pause
                                        ; set this bit to start the codec!


GLOB_CNT_REG            =     2ch     ; Global control register
SEC_RES_EN              =     BIT5    ; secondary codec resume event
                                        ; interrupt enable.  Not used here.
PRI_RES_EN              =     BIT4    ; ditto for primary. Not used here.
ACLINK_OFF              =     BIT3    ; Turn off the AC97 link
ACWARM_RESET            =     BIT2    ; Awaken the AC97 link from sleep.
                                        ; registers preserved, bit self clears
ACCOLD_RESET            =     BIT1    ; Reset everything in the AC97 and
                                        ; reset all registers.  Not self clearin
;g

GPIIE                   =     BIT0    ; GPI Interrupt enable.
                                        ; set if you want an interrupt to
                                        ; fire upon ANY of the bits in the
                                        ; GPI (general pursose inputs?) not used
;.

GLOB_STS_REG            =     30h     ; Global Status register (RO)

MD3                     =     BIT17   ; modem powerdown status (yawn)
AD3                     =     BIT16   ; Audio powerdown status (yawn)
RD_COMPLETE_STS         =     BIT15   ; Codec read timed out. 0=normal
BIT3SLOT12              =     BIT14   ; shadowed status of bit 3 in slot 12
BIT2SLOT12              =     BIT13   ; shadowed status of bit 2 in slot 12
BIT1SLOT12              =     BIT12   ; shadowed status of bit 1 in slot 12
SEC_RESUME_STS          =     BIT11   ; secondary codec has resumed (and irqed)
PRI_RESUME_STS          =     BIT10   ; primary codec has resumed (and irqed)
SEC_CODEC_RDY           =     BIT9    ; secondary codec is ready for action
PRI_CODEC_RDY           =     BIT8    ; Primary codec is ready for action
                                        ; software must check these bits before
                                        ; starting the codec!
MIC_IN_IRQ              =     BIT7    ; MIC in caused an interrupt
PCM_OUT_IRQ             =     BIT6    ; One of the PCM out channels IRQed
PCM_IN_IRQ              =     BIT5    ; One of the PCM in channels IRQed
MODEM_OUT_IRQ           =     BIT2    ; modem out channel IRQed
MODEM_IN_IRQ            =     BIT1    ; modem in channel IRQed
GPI_STS_CHANGE          =     BIT0    ; set whenever GPI's have changed.
                                        ; BIT0 of slot 12 also reflects this.


ACC_SEMA_REG            =     34h     ; Codec write semiphore register
CODEC_BUSY              =     BIT0    ; codec register I/O is happening
                                        ; self clearing



;
; Buffer Descriptors List
; As stated earlier, each buffer descriptor list is a set of (up to) 32
; descriptors, each 8 bytes in length.  Bytes 0-3 of a descriptor entry point
; to a chunk of memory to either play from or record to.  Bytes 4-7 of an
; entry describe various control things detailed below.
;
; Buffer pointers must always be aligned on a Dword boundry.
;
;

IOC                     =     BIT31   ; Fire an interrupt whenever this
                                        ; buffer is complete.

BUP                     =     BIT30   ; Buffer Underrun Policy.
                                        ; if this buffer is the last buffer
                                        ; in a playback, fill the remaining
                                        ; samples with 0 (silence) or not.
                                        ; It's a good idea to set this to 1
                                        ; for the last buffer in playback,
                                        ; otherwise you're likely to get a lot
                                        ; of noise at the end of the sound.

;
; Bits 15:0 contain the length of the buffer, in number of samples, which
; are 16 bits each, coupled in left and right pairs, or 32bits each.
; Luckily for us, that's the same format as .wav files.
;
; A value of FFFF is 65536 samples.  Running at 44.1Khz, that's just about
; 1.5 seconds of sample time.  FFFF * 32bits is 1FFFFh bytes or 128k of data.
;
; A value of 0 in these bits means play no samples.
;


;*****************************************************************************
;* AC97 Codec registers include (based on Jeff Leyda AC97 wav player SDK :-)
;*****************************************************************************

; Not all codecs are created =al. Refer to the spec for your specific codec.
; All registers are 16bits wide.  Access to codec registers over the AC97 link
; is defined by the OEM.
; Secondary codec's are accessed by ORing in BIT7 of all register accesses.


; each codec/mixer register is 16bits

CODEC_RESET_REG                 =     00      ; reset codec
CODEC_MASTER_VOL_REG            =     02      ; master volume
CODEC_HP_VOL_REG                =     04      ; headphone volume
CODEC_MASTER_MONO_VOL_REG       =     06      ; master mono volume
CODEC_MASTER_TONE_REG           =     08      ; master tone (R+L)
CODEC_PCBEEP_VOL_REG            =     0ah     ; PC beep volume
CODEC_PHONE_VOL_REG             =     0ch     ; phone volume
CODEC_MIC_VOL_REG               =     0eh     ; MIC volume
CODEC_LINE_IN_VOL_REG           =     10h     ; line input volume
CODEC_CD_VOL_REG                =     12h     ; CD volume
CODEC_VID_VOL_REG               =     14h     ; video volume
CODEC_AUX_VOL_REG               =     16h     ; aux volume
CODEC_PCM_OUT_REG               =     18h     ; PCM output volume
CODEC_RECORD_SELECT_REG         =     1ah     ; record select input
CODEC_RECORD_VOL_REG            =     1ch     ; record volume
CODEC_RECORD_MIC_VOL_REG        =     1eh     ; record mic volume
CODEC_GP_REG                    =     20h     ; general purpose
CODEC_3D_CONTROL_REG            =     22h     ; 3D control
; 24h is reserved
CODEC_POWER_CTRL_REG            =     26h     ; powerdown control
CODEC_EXT_AUDIO_REG             =     28h     ; extended audio
CODEC_EXT_AUDIO_CTRL_REG        =     2ah     ; extended audio control
CODEC_PCM_FRONT_DACRATE_REG     =     2ch     ; PCM out sample rate
CODEC_PCM_SURND_DACRATE_REG     =     2eh     ; surround sound sample rate
CODEC_PCM_LFE_DACRATE_REG       =     30h     ; LFE sample rate
CODEC_LR_ADCRATE_REG            =     32h     ; PCM in sample rate
CODEC_MIC_ADCRATE_REG           =     34h     ; mic in sample rate


; registers 36-7a are reserved on the ICH

CODEC_VENDORID1_REG             =     7ch     ; codec vendor ID 1
CODEC_VENDORID2_REG             =     7eh     ; codec vendor ID 2


; When 2 codecs are present in the system, use BIT7 to access the 2nd
; set of registers, ie 80h-feh

SECONDARY_CODEC                 =     BIT7    ; 80-8f registers for 2nda