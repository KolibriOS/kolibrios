; Universal Interface for Intel High Definition Audio Codec

; nodes
AC_NODE_ROOT		equ 0x00

; function group types
AC_GRP_AUDIO_FUNCTION	equ 0x01
AC_GRP_MODEM_FUNCTION	equ 0x02

; widget types
AC_WID_AUD_OUT		equ 0x0 ; Audio Out
AC_WID_AUD_IN		equ 0x1 ; Audio In
AC_WID_AUD_MIX		equ 0x2 ; Audio Mixer
AC_WID_AUD_SEL		equ 0x3 ; Audio Selector
AC_WID_PIN		equ 0x4 ; Pin Complex
AC_WID_POWER		equ 0x5 ; Power
AC_WID_VOL_KNB		equ 0x6 ; Volume Knob
AC_WID_BEEP		equ 0x7 ; Beep Generator
AC_WID_VENDOR		equ 0x0F    ; Vendor specific

; GET verbs
AC_VERB_GET_STREAM_FORMAT		equ 0x0A00
AC_VERB_GET_AMP_GAIN_MUTE		equ 0x0B00
AC_VERB_GET_PROC_COEF			equ 0x0C00
AC_VERB_GET_COEF_INDEX			equ 0x0D00
AC_VERB_PARAMETERS			equ 0x0F00
AC_VERB_GET_CONNECT_SEL 		equ 0x0F01
AC_VERB_GET_CONNECT_LIST		equ 0x0F02
AC_VERB_GET_PROC_STATE			equ 0x0F03
AC_VERB_GET_SDI_SELECT			equ 0x0F04
AC_VERB_GET_POWER_STATE 		equ 0x0F05
AC_VERB_GET_CONV			equ 0x0F06
AC_VERB_GET_PIN_WIDGET_CONTROL		equ 0x0F07
AC_VERB_GET_UNSOLICITED_RESPONSE	equ 0x0F08
AC_VERB_GET_PIN_SENSE			equ 0x0F09
AC_VERB_GET_BEEP_CONTROL		equ 0x0F0A
AC_VERB_GET_EAPD_BTLENABLE		equ 0x0F0C
AC_VERB_GET_DIGI_CONVERT_1		equ 0x0F0D
AC_VERB_GET_DIGI_CONVERT_2		equ 0x0F0E ; unused
AC_VERB_GET_VOLUME_KNOB_CONTROL 	equ 0x0F0F
; f10-f1a: GPIO
AC_VERB_GET_GPIO_DATA			equ 0x0F15
AC_VERB_GET_GPIO_MASK			equ 0x0F16
AC_VERB_GET_GPIO_DIRECTION		equ 0x0F17
AC_VERB_GET_GPIO_WAKE_MASK		equ 0x0F18
AC_VERB_GET_GPIO_UNSOLICITED_RSP_MASK	equ 0x0F19
AC_VERB_GET_GPIO_STICKY_MASK		equ 0x0F1A
AC_VERB_GET_CONFIG_DEFAULT		equ 0x0F1C
; f20: AFG/MFG
AC_VERB_GET_SUBSYSTEM_ID		equ 0x0F20
AC_VERB_GET_CVT_CHAN_COUNT		equ 0x0F2D
AC_VERB_GET_HDMI_DIP_SIZE		equ 0x0F2E
AC_VERB_GET_HDMI_ELDD			equ 0x0F2F
AC_VERB_GET_HDMI_DIP_INDEX		equ 0x0F30
AC_VERB_GET_HDMI_DIP_DATA		equ 0x0F31
AC_VERB_GET_HDMI_DIP_XMIT		equ 0x0F32
AC_VERB_GET_HDMI_CP_CTRL		equ 0x0F33
AC_VERB_GET_HDMI_CHAN_SLOT		equ 0x0F34

; SET verbs

AC_VERB_SET_STREAM_FORMAT		equ 0x200
AC_VERB_SET_AMP_GAIN_MUTE		equ 0x300
AC_VERB_SET_PROC_COEF			equ 0x400
AC_VERB_SET_COEF_INDEX			equ 0x500
AC_VERB_SET_CONNECT_SEL 		equ 0x701
AC_VERB_SET_PROC_STATE			equ 0x703
AC_VERB_SET_SDI_SELECT			equ 0x704
AC_VERB_SET_POWER_STATE 		equ 0x705
AC_VERB_SET_CHANNEL_STREAMID		equ 0x706
AC_VERB_SET_PIN_WIDGET_CONTROL		equ 0x707
AC_VERB_SET_UNSOLICITED_ENABLE		equ 0x708
AC_VERB_SET_PIN_SENSE			equ 0x709
AC_VERB_SET_BEEP_CONTROL		equ 0x70A
AC_VERB_SET_EAPD_BTLENABLE		equ 0x70C
AC_VERB_SET_DIGI_CONVERT_1		equ 0x70D
AC_VERB_SET_DIGI_CONVERT_2		equ 0x70E
AC_VERB_SET_VOLUME_KNOB_CONTROL 	equ 0x70F
AC_VERB_SET_GPIO_DATA			equ 0x715
AC_VERB_SET_GPIO_MASK			equ 0x716
AC_VERB_SET_GPIO_DIRECTION		equ 0x717
AC_VERB_SET_GPIO_WAKE_MASK		equ 0x718
AC_VERB_SET_GPIO_UNSOLICITED_RSP_MASK	equ 0x719
AC_VERB_SET_GPIO_STICKY_MASK		equ 0x71A
AC_VERB_SET_CONFIG_DEFAULT_BYTES_0	equ 0x71C
AC_VERB_SET_CONFIG_DEFAULT_BYTES_1	equ 0x71D
AC_VERB_SET_CONFIG_DEFAULT_BYTES_2	equ 0x71E
AC_VERB_SET_CONFIG_DEFAULT_BYTES_3	equ 0x71F
AC_VERB_SET_EAPD			       equ 0x788
AC_VERB_SET_CODEC_RESET 		equ 0x7FF
AC_VERB_SET_CVT_CHAN_COUNT		equ 0x72D
AC_VERB_SET_HDMI_DIP_INDEX		equ 0x730
AC_VERB_SET_HDMI_DIP_DATA		equ 0x731
AC_VERB_SET_HDMI_DIP_XMIT		equ 0x732
AC_VERB_SET_HDMI_CP_CTRL		equ 0x733
AC_VERB_SET_HDMI_CHAN_SLOT		equ 0x734

; Parameter IDs

AC_PAR_VENDOR_ID		equ 0x00
AC_PAR_SUBSYSTEM_ID		equ 0x01
AC_PAR_REV_ID			equ 0x02
AC_PAR_NODE_COUNT		equ 0x04
AC_PAR_FUNCTION_TYPE		equ 0x05
AC_PAR_AUDIO_FG_CAP		equ 0x08
AC_PAR_AUDIO_WIDGET_CAP 	equ 0x09
AC_PAR_PCM			equ 0x0A
AC_PAR_STREAM			equ 0x0B
AC_PAR_PIN_CAP			equ 0x0C
AC_PAR_AMP_IN_CAP		equ 0x0D
AC_PAR_CONNLIST_LEN		equ 0x0E
AC_PAR_POWER_STATE		equ 0x0F
AC_PAR_PROC_CAP 		equ 0x10
AC_PAR_GPIO_CAP 		equ 0x11
AC_PAR_AMP_OUT_CAP		equ 0x12
AC_PAR_VOL_KNB_CAP		equ 0x13
AC_PAR_HDMI_LPCM_CAP		equ 0x20

; AC_VERB_PARAMETERS results (32bit)


; Function Group Type
AC_FGT_TYPE			equ (0xFF shl 0)
AC_FGT_TYPE_SHIFT		equ 0
AC_FGT_UNSOL_CAP		equ (1 shl 8)

; Audio Function Group Capabilities
AC_AFG_OUT_DELAY		equ (0xF shl 0)
AC_AFG_IN_DELAY 		equ (0xF shl 8)
AC_AFG_BEEP_GEN 		equ (1 shl 16)

; Audio Widget Capabilities
AC_WCAP_STEREO			equ (1 shl 0)  ; stereo I/O
AC_WCAP_IN_AMP			equ (1 shl 1)  ; AMP-in present
AC_WCAP_OUT_AMP 		equ (1 shl 2)  ; AMP-out present
AC_WCAP_AMP_OVRD		equ (1 shl 3)  ; AMP-parameter override
AC_WCAP_FORMAT_OVRD		equ (1 shl 4)  ; format override
AC_WCAP_STRIPE			equ (1 shl 5)  ; stripe
AC_WCAP_PROC_WID		equ (1 shl 6)  ; Proc Widget
AC_WCAP_UNSOL_CAP		equ (1 shl 7)  ; Unsol capable
AC_WCAP_CONN_LIST		equ (1 shl 8)  ; connection list
AC_WCAP_DIGITAL 		equ (1 shl 9)  ; digital I/O
AC_WCAP_POWER			equ (1 shl 10) ; power control
AC_WCAP_LR_SWAP 		equ (1 shl 11) ; L/R swap
AC_WCAP_CP_CAPS 		equ (1 shl 12) ; content protection
AC_WCAP_CHAN_CNT_EXT		equ (7 shl 13) ; channel count ext
AC_WCAP_DELAY			equ (0xF shl 16)
AC_WCAP_DELAY_SHIFT		equ 16
AC_WCAP_TYPE			equ (0xF shl 20)
AC_WCAP_TYPE_SHIFT		equ 20

; supported PCM rates and bits
AC_SUPPCM_RATES 		equ (0xFFF  shl  0)
AC_SUPPCM_BITS_8		equ (1 shl 16)
AC_SUPPCM_BITS_16		equ (1 shl 17)
AC_SUPPCM_BITS_20		equ (1 shl 18)
AC_SUPPCM_BITS_24		equ (1 shl 19)
AC_SUPPCM_BITS_32		equ (1 shl 20)

; supported PCM stream format
AC_SUPFMT_PCM			equ (1 shl 0)
AC_SUPFMT_FLOAT32		equ (1 shl 1)
AC_SUPFMT_AC3			equ (1 shl 2)

; GP I/O count
AC_GPIO_IO_COUNT		equ (0xFF shl 0)
AC_GPIO_O_COUNT 		equ (0xFF shl 8)
AC_GPIO_O_COUNT_SHIFT		equ 8
AC_GPIO_I_COUNT 		equ (0xFF shl 16)
AC_GPIO_I_COUNT_SHIFT		equ 16
AC_GPIO_UNSOLICITED		equ (1 shl 30)
AC_GPIO_WAKE			equ (1 shl 31)

; Converter stream, channel
AC_CONV_CHANNEL 		equ (0xF shl 0)
AC_CONV_STREAM			equ (0xF shl 4)
AC_CONV_STREAM_SHIFT		equ 4

; Input converter SDI select
AC_SDI_SELECT			equ (0xF shl 0)

; Unsolicited response control
AC_UNSOL_TAG			equ (0x3F shl 0)
AC_UNSOL_ENABLED		equ (1 shl 7)
AC_USRSP_EN			equ AC_UNSOL_ENABLED

; Unsolicited responses
AC_UNSOL_RES_TAG		equ (0x3F shl 26)
AC_UNSOL_RES_TAG_SHIFT		equ 26
AC_UNSOL_RES_SUBTAG		equ (0x1F shl 21)
AC_UNSOL_RES_SUBTAG_SHIFT	equ 21
AC_UNSOL_RES_ELDV		equ (1 shl 1)  ; ELD Data valid (for HDMI)
AC_UNSOL_RES_PD 		equ (1 shl 0)  ; pinsense detect
AC_UNSOL_RES_CP_STATE		equ (1 shl 1)  ; content protection
AC_UNSOL_RES_CP_READY		equ (1 shl 0)  ; content protection

; Pin widget capabilies
AC_PINCAP_IMP_SENSE		equ (1 shl 0)  ; impedance sense capable
AC_PINCAP_TRIG_REQ		equ (1 shl 1)  ; trigger required
AC_PINCAP_PRES_DETECT		equ (1 shl 2)  ; presence detect capable
AC_PINCAP_HP_DRV		equ (1 shl 3)  ; headphone drive capable
AC_PINCAP_OUT			equ (1 shl 4)  ; output capable
AC_PINCAP_IN			equ (1 shl 5)  ; input capable
AC_PINCAP_BALANCE		equ (1 shl 6)  ; balanced I/O capable
; Note: This LR_SWAP pincap is defined in the Realtek ALC883 specification,
;       but is marked reserved in the Intel HDA specification.

AC_PINCAP_LR_SWAP		equ (1 shl 7)  ; L/R swap
; Note: The same bit as LR_SWAP is newly defined as HDMI capability
;       in HD-audio specification

AC_PINCAP_HDMI			equ (1 shl 7)  ; HDMI pin
AC_PINCAP_DP			equ (1 shl 24) ; DisplayPort pin, can
						 ; coexist with AC_PINCAP_HDMI

AC_PINCAP_VREF			equ (0x37 shl 8)
AC_PINCAP_VREF_SHIFT		equ 8
AC_PINCAP_EAPD			equ (1 shl 16) ; EAPD capable
AC_PINCAP_HBR			equ (1 shl 27) ; High Bit Rate
; Vref status (used in pin cap)
AC_PINCAP_VREF_HIZ		equ (1 shl 0)  ; Hi-Z
AC_PINCAP_VREF_50		equ (1 shl 1)  ; 50%
AC_PINCAP_VREF_GRD		equ (1 shl 2)  ; ground
AC_PINCAP_VREF_80		equ (1 shl 4)  ; 80%
AC_PINCAP_VREF_100		equ (1 shl 5)  ; 100%

; Amplifier capabilities
AC_AMPCAP_OFFSET		equ (0x7F shl 0)  ; 0dB offset
AC_AMPCAP_OFFSET_SHIFT		equ 0
AC_AMPCAP_NUM_STEPS		equ (0x7F shl 8)  ; number of steps
AC_AMPCAP_NUM_STEPS_SHIFT	equ 8
AC_AMPCAP_STEP_SIZE		equ (0x7F shl 16) ; step size 0-32dB
						    ; in 0.25dB
AC_AMPCAP_STEP_SIZE_SHIFT	equ 16
AC_AMPCAP_MUTE			equ (1 shl 31)	  ; mute capable
AC_AMPCAP_MUTE_SHIFT		equ 31

; Connection list
AC_CLIST_LENGTH 		equ (0x7F shl 0)
AC_CLIST_LONG			equ (1 shl 7)

; Supported power status
AC_PWRST_D0SUP			equ (1 shl 0)
AC_PWRST_D1SUP			equ (1 shl 1)
AC_PWRST_D2SUP			equ (1 shl 2)
AC_PWRST_D3SUP			equ (1 shl 3)
AC_PWRST_D3COLDSUP		equ (1 shl 4)
AC_PWRST_S3D3COLDSUP		equ (1 shl 29)
AC_PWRST_CLKSTOP		equ (1 shl 30)
AC_PWRST_EPSS			equ (1 shl 31)

; Power state values
AC_PWRST_SETTING		equ (0xF shl 0)
AC_PWRST_ACTUAL 		equ (0xF shl 4)
AC_PWRST_ACTUAL_SHIFT		equ 4
AC_PWRST_D0			equ 0x00
AC_PWRST_D1			equ 0x01
AC_PWRST_D2			equ 0x02
AC_PWRST_D3			equ 0x03

; Processing capabilies
AC_PCAP_BENIGN			equ (1 shl 0)
AC_PCAP_NUM_COEF		equ (0xFF shl 8)
AC_PCAP_NUM_COEF_SHIFT		equ 8

; Volume knobs capabilities
AC_KNBCAP_NUM_STEPS		equ (0x7F shl 0)
AC_KNBCAP_DELTA 		equ (1 shl 7)

; HDMI LPCM capabilities
AC_LPCMCAP_48K_CP_CHNS		equ (0x0F shl 0) ; max channels w/ CP-on
AC_LPCMCAP_48K_NO_CHNS		equ (0x0F shl 4) ; max channels w/o CP-on
AC_LPCMCAP_48K_20BIT		equ (1 shl 8)  ; 20b bitrate supported
AC_LPCMCAP_48K_24BIT		equ (1 shl 9)  ; 24b bitrate supported
AC_LPCMCAP_96K_CP_CHNS		equ (0x0F shl 10) ; max channels w/ CP-on
AC_LPCMCAP_96K_NO_CHNS		equ (0x0F shl 14) ; max channels w/o CP-on
AC_LPCMCAP_96K_20BIT		equ (1 shl 18) ; 20b bitrate supported
AC_LPCMCAP_96K_24BIT		equ (1 shl 19) ; 24b bitrate supported
AC_LPCMCAP_192K_CP_CHNS 	equ (0x0F shl 20) ; max channels w/ CP-on
AC_LPCMCAP_192K_NO_CHNS 	equ (0x0F shl 24) ; max channels w/o CP-on
AC_LPCMCAP_192K_20BIT		equ (1 shl 28) ; 20b bitrate supported
AC_LPCMCAP_192K_24BIT		equ (1 shl 29) ; 24b bitrate supported
AC_LPCMCAP_44K			equ (1 shl 30) ; 44.1kHz support
AC_LPCMCAP_44K_MS		equ (1 shl 31) ; 44.1kHz-multiplies support


; Control Parameters

; Amp gain/mute
AC_AMP_MUTE			equ (1 shl 7)
AC_AMP_GAIN			equ (0x7F)
AC_AMP_GET_INDEX		equ (0xF shl 0)

AC_AMP_GET_LEFT 		equ (1 shl 13)
AC_AMP_GET_RIGHT		equ (0 shl 13)
AC_AMP_GET_OUTPUT		equ (1 shl 15)
AC_AMP_GET_INPUT		equ (0 shl 15)

AC_AMP_SET_INDEX		equ (0xF shl 8)
AC_AMP_SET_INDEX_SHIFT		equ 8
AC_AMP_SET_RIGHT		equ (1 shl 12)
AC_AMP_SET_LEFT 		equ (1 shl 13)
AC_AMP_SET_INPUT		equ (1 shl 14)
AC_AMP_SET_OUTPUT		equ (1 shl 15)

; DIGITAL1 bits
AC_DIG1_ENABLE			equ (1 shl 0)
AC_DIG1_V			equ (1 shl 1)
AC_DIG1_VCFG			equ (1 shl 2)
AC_DIG1_EMPHASIS		equ (1 shl 3)
AC_DIG1_COPYRIGHT		equ (1 shl 4)
AC_DIG1_NONAUDIO		equ (1 shl 5)
AC_DIG1_PROFESSIONAL		equ (1 shl 6)
AC_DIG1_LEVEL			equ (1 shl 7)

; DIGITAL2 bits
AC_DIG2_CC		     equ (0x7F shl 0)

; Pin widget control - 8bit
AC_PINCTL_VREFEN	     equ (0x7 shl 0)
AC_PINCTL_VREF_HIZ	     equ 0	 ; Hi-Z
AC_PINCTL_VREF_50	     equ 1	 ; 50%
AC_PINCTL_VREF_GRD	     equ 2	 ; ground
AC_PINCTL_VREF_80	     equ 4	 ; 80%
AC_PINCTL_VREF_100	     equ 5	 ; 100%
AC_PINCTL_IN_EN 	     equ (1 shl 5)
AC_PINCTL_OUT_EN	     equ (1 shl 6)
AC_PINCTL_HP_EN 	     equ (1 shl 7)

; Pin sense - 32bit
AC_PINSENSE_IMPEDANCE_MASK   equ (0x7FFFFFFF)
AC_PINSENSE_PRESENCE	     equ (1 shl 31)
AC_PINSENSE_ELDV	     equ (1 shl 30) ; ELD valid (HDMI)

; EAPD/BTL enable - 32bit
AC_EAPDBTL_BALANCED	     equ (1 shl 0)
AC_EAPDBTL_EAPD 	     equ (1 shl 1)
AC_EAPDBTL_LR_SWAP	     equ (1 shl 2)

; HDMI ELD data
AC_ELDD_ELD_VALID	     equ (1 shl 31)
AC_ELDD_ELD_DATA	     equ 0xFF

; HDMI DIP size
AC_DIPSIZE_ELD_BUF	     equ (1 shl 3) ; ELD buf size of packet size
AC_DIPSIZE_PACK_IDX	     equ (0x07 shl 0) ; packet index

; HDMI DIP index
AC_DIPIDX_PACK_IDX	     equ (0x07 shl 5) ; packet idnex
AC_DIPIDX_BYTE_IDX	     equ (0x1F shl 0) ; byte index

; HDMI DIP xmit (transmit) control
AC_DIPXMIT_MASK 	     equ (0x3 shl 6)
AC_DIPXMIT_DISABLE	     equ (0x0 shl 6) ; disable xmit
AC_DIPXMIT_ONCE 	     equ (0x2 shl 6) ; xmit once then disable
AC_DIPXMIT_BEST 	     equ (0x3 shl 6) ; best effort

; HDMI content protection (CP) control
AC_CPCTRL_CES		     equ (1 shl 9) ; current encryption state
AC_CPCTRL_READY 	     equ (1 shl 8) ; ready bit
AC_CPCTRL_SUBTAG	     equ (0x1F shl 3) ; subtag for unsol-resp
AC_CPCTRL_STATE 	     equ (3 shl 0) ; current CP request state

; Converter channel <-> HDMI slot mapping
AC_CVTMAP_HDMI_SLOT	     equ (0xF shl 0) ; HDMI slot number
AC_CVTMAP_CHAN		     equ (0xF shl 4) ; converter channel number

; configuration default - 32bit
AC_DEFCFG_SEQUENCE	     equ (0xF shl 0)
AC_DEFCFG_DEF_ASSOC	     equ (0xF shl 4)
AC_DEFCFG_ASSOC_SHIFT	     equ 4
AC_DEFCFG_MISC		     equ (0xF shl 8)
AC_DEFCFG_MISC_SHIFT	     equ 8
AC_DEFCFG_MISC_NO_PRESENCE   equ (1   shl 0)
AC_DEFCFG_COLOR 	     equ (0xF shl 12)
AC_DEFCFG_COLOR_SHIFT	     equ 12
AC_DEFCFG_CONN_TYPE	     equ (0xF shl 16)
AC_DEFCFG_CONN_TYPE_SHIFT    equ 16
AC_DEFCFG_DEVICE	     equ (0xF shl 20)
AC_DEFCFG_DEVICE_SHIFT	     equ 20
AC_DEFCFG_LOCATION	     equ (0x3F shl 24)
AC_DEFCFG_LOCATION_SHIFT     equ 24
AC_DEFCFG_PORT_CONN	     equ (0x3 shl 30)
AC_DEFCFG_PORT_CONN_SHIFT    equ 30

; device device types (0x0-0xf)
AC_JACK_LINE_OUT	     equ 0x0
AC_JACK_SPEAKER 	     equ 0x1
AC_JACK_HP_OUT		     equ 0x2
AC_JACK_CD		     equ 0x3
AC_JACK_SPDIF_OUT	     equ 0x4
AC_JACK_DIG_OTHER_OUT	     equ 0x5
AC_JACK_MODEM_LINE_SIDE      equ 0x6
AC_JACK_MODEM_HAND_SIDE      equ 0x7
AC_JACK_LINE_IN 	     equ 0x8
AC_JACK_AUX		     equ 0x9
AC_JACK_MIC_IN		     equ 0xA
AC_JACK_TELEPHONY	     equ 0xB
AC_JACK_SPDIF_IN	     equ 0xC
AC_JACK_DIG_OTHER_IN	     equ 0xD
AC_JACK_OTHER		     equ 0xF

; jack connection types (0x0-0xf)
AC_JACK_CONN_UNKNOWN	     equ 0x0
AC_JACK_CONN_1_8	     equ 0x1
AC_JACK_CONN_1_4	     equ 0x2
AC_JACK_CONN_ATAPI	     equ 0x3
AC_JACK_CONN_RCA	     equ 0x4
AC_JACK_CONN_OPTICAL	     equ 0x5
AC_JACK_CONN_OTHER_DIGITAL   equ 0x6
AC_JACK_CONN_OTHER_ANALOG    equ 0x7
AC_JACK_CONN_DIN	     equ 0x8
AC_JACK_CONN_XLR	     equ 0x9
AC_JACK_CONN_RJ11	     equ 0xA
AC_JACK_CONN_COMB	     equ 0xB
AC_JACK_CONN_OTHER	     equ 0xF

; jack colors (0x0-0xf)
AC_JACK_COLOR_UNKNOWN	equ 0x0
AC_JACK_COLOR_BLACK	equ 0x1
AC_JACK_COLOR_GREY	equ 0x2
AC_JACK_COLOR_BLUE	equ 0x3
AC_JACK_COLOR_GREEN	equ 0x4
AC_JACK_COLOR_RED	equ 0x5
AC_JACK_COLOR_ORANGE	equ 0x6
AC_JACK_COLOR_YELLOW	equ 0x7
AC_JACK_COLOR_PURPLE	equ 0x8
AC_JACK_COLOR_PINK	equ 0x9
AC_JACK_COLOR_WHITE	equ 0xE
AC_JACK_COLOR_OTHER	equ 0xF

; Jack location (0x0-0x3f)
; common case
AC_JACK_LOC_NONE	equ 0
AC_JACK_LOC_REAR	equ 1
AC_JACK_LOC_FRONT	equ 2
AC_JACK_LOC_LEFT	equ 3
AC_JACK_LOC_RIGHT	equ 4
AC_JACK_LOC_TOP 	equ 5
AC_JACK_LOC_BOTTOM	equ 6

; bits 4-5
AC_JACK_LOC_EXTERNAL	equ 0x00
AC_JACK_LOC_INTERNAL	equ 0x10
AC_JACK_LOC_SEPARATE	equ 0x20
AC_JACK_LOC_OTHER	equ 0x30

; external on primary chasis
AC_JACK_LOC_REAR_PANEL	equ 0x07
AC_JACK_LOC_DRIVE_BAY	equ 0x08
; internal
AC_JACK_LOC_RISER	equ 0x17
AC_JACK_LOC_HDMI	equ 0x18
AC_JACK_LOC_ATAPI	equ 0x19
; others
AC_JACK_LOC_MOBILE_IN	equ 0x37
AC_JACK_LOC_MOBILE_OUT	equ 0x38

; Port connectivity (0-3)
AC_JACK_PORT_COMPLEX	equ 0
AC_JACK_PORT_NONE	equ 1
AC_JACK_PORT_FIXED	equ 2
AC_JACK_PORT_BOTH	equ 3

; max. connections to a widget
HDA_MAX_CONNECTIONS	equ 32

; max. codec address
HDA_MAX_CODEC_ADDRESS	equ 0x0f

; max number of PCM devics per card
HDA_MAX_PCMS		equ 10


; Structures

; direction
HDA_INPUT	   equ 0x0
HDA_OUTPUT	   equ 0x1


struc HDA_VERB
{
	.nid	   dw ?
	.verb	   dd ?
	.param	   dd ?
}

virtual at 0
   HDA_VERB  HDA_VERB
end virtual

; the struct for codec->pin_configs
struc HDA_PINCFG
{
	.nid	   dw ?
	.reserved  dw ? ;Asper + for align purposes
	.cfg	   dd ?
	.sizeof:
}

virtual at 0
   HDA_PINCFG  HDA_PINCFG
end virtual


;Asper [   this part is from "hda_local.h"

;(...)
; amp value bits
HDA_AMP_MUTE	   equ 0x80
HDA_AMP_UNMUTE	   equ 0x00
HDA_AMP_VOLMASK    equ 0x7F
;(...)


; unsolicited event handler
HDA_UNSOL_QUEUE_SIZE	equ 64

;struc  HDA_BUS_UNSOLICITED
;{
;        ; ring buffer
;        .queue:
;            times HDA_UNSOL_QUEUE_SIZE*2 dd ?
;        .rp                              dd ?
;        .wp                              dd ?
;
;        ; workqueue
;        .work                            dd ?;struct work_struct work;
;        .bus                             dd ?  ;struct hda_bus ;bus
;};

; Helper for automatic ping configuration
AUTO_PIN_MIC		 equ 0
AUTO_PIN_FRONT_MIC	 equ 1
AUTO_PIN_LINE		 equ 2
AUTO_PIN_FRONT_LINE	 equ 3
AUTO_PIN_CD		 equ 4
AUTO_PIN_AUX		 equ 5
AUTO_PIN_LAST		 equ 6


AUTO_PIN_LINE_OUT	 equ 0
AUTO_PIN_SPEAKER_OUT	 equ 1
AUTO_PIN_HP_OUT 	 equ 2


;extern const char *auto_pin_cfg_labels[AUTO_PIN_LAST];

AUTO_CFG_MAX_OUTS	 equ 5

;struc AUTO_PIN_CFG
;{
;        .line_outs       dd ?
;        ; sorted in the order of FrontSurrCLFESide
;        .line_out_pins   times AUTO_CFG_MAX_OUTS  dw ?
;        .speaker_outs    dd ?
;        .speaker_pins    times AUTO_CFG_MAX_OUTS  dw ?
;        .hp_outs         dd ?
;        .line_out_type   dd ?   ; AUTO_PIN_XXX_OUT
;        .hp_pins         times AUTO_CFG_MAX_OUTS  dw ?
;        .input_pins      times AUTO_PIN_LAST      dw ?
;        .dig_outs        dd ?
;        .dig_out_pins    times 2 dd ?
;        .dig_in_pin      dw ?
;        .mono_out_pin    dw ?
;        .dig_out_type    times 2 dd ?  ; HDA_PCM_TYPE_XXX
;        .dig_in_type     dd ?  ; HDA_PCM_TYPE_XXX
;}

;#define get_defcfg_connect(cfg) \
;        ((cfg & AC_DEFCFG_PORT_CONN) >> AC_DEFCFG_PORT_CONN_SHIFT)
;#define get_defcfg_association(cfg) \
;        ((cfg & AC_DEFCFG_DEF_ASSOC) >> AC_DEFCFG_ASSOC_SHIFT)
;#define get_defcfg_location(cfg) \
;        ((cfg & AC_DEFCFG_LOCATION) >> AC_DEFCFG_LOCATION_SHIFT)
;#define get_defcfg_sequence(cfg) \
;        (cfg & AC_DEFCFG_SEQUENCE)
;#define get_defcfg_device(cfg) \
;        ((cfg & AC_DEFCFG_DEVICE) >> AC_DEFCFG_DEVICE_SHIFT)

; amp values
;AMP_IN_MUTE(idx)        (0x7080 | ((idx)<<8))
;AMP_IN_UNMUTE(idx)      (0x7000 | ((idx)<<8))
AMP_OUT_MUTE		equ 0xb080
AMP_OUT_UNMUTE		equ 0xb000
AMP_OUT_ZERO		equ 0xb000
; pinctl values
PIN_IN			equ (AC_PINCTL_IN_EN)
PIN_VREFHIZ		equ (AC_PINCTL_IN_EN or AC_PINCTL_VREF_HIZ)
PIN_VREF50		equ (AC_PINCTL_IN_EN or AC_PINCTL_VREF_50)
PIN_VREFGRD		equ (AC_PINCTL_IN_EN or AC_PINCTL_VREF_GRD)
PIN_VREF80		equ (AC_PINCTL_IN_EN or AC_PINCTL_VREF_80)
PIN_VREF100		equ (AC_PINCTL_IN_EN or AC_PINCTL_VREF_100)
PIN_OUT 		equ (AC_PINCTL_OUT_EN)
PIN_HP			equ (AC_PINCTL_OUT_EN or AC_PINCTL_HP_EN)
PIN_HP_AMP		equ (AC_PINCTL_HP_EN)


; get widget capabilities
proc  get_wcaps stdcall, nid:dword
	push	ebx ecx edx
	xor	eax, eax
	movzx	ebx, [codec.start_nid]
	movzx	ecx, [codec.num_nodes]
	mov	edx, [nid]

	cmp	edx, ebx
	jl	.out

	add	ecx, ebx
	cmp	edx, ecx
	jge	.out

	sub	edx, ebx
	shl	edx, 2
	add	edx, [codec.wcaps]
	mov	eax, [edx]
.out:
	pop	edx ecx ebx
	ret
endp

; get the widget type from widget capability bits
proc  get_wcaps_type stdcall, wcaps:dword
	mov	eax, [wcaps]
	and	eax, AC_WCAP_TYPE
	shr	eax, AC_WCAP_TYPE_SHIFT
	ret
endp

;static inline unsigned int get_wcaps_channels(u32 wcaps)
proc  get_wcaps_channels stdcall, wcaps:dword
	mov	eax, [wcaps]
	and	eax, AC_WCAP_CHAN_CNT_EXT
	shr	eax, 13
	shl	eax, 1
	or	eax, 1
	inc	eax
	ret
endp

;Asper ]
