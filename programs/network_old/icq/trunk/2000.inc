; constants
FLAP_HEAD_SIZE 	= 6
SNAC_HEAD_SIZE 	= 10

;AUTH_MESSAGE 	= 0008h
;USER_ADDED_MESS = 000Ch
;AUTH_REQ_MESS 	= 0006h
;URL_MESS	= 0004h
;WEB_MESS	= 000dh
;EMAIL_MESS	= 000eh
;MASS_MESS_MASK 	= 8000h
;MRURL_MESS	= 8004h
;NORM_MESS	= 0001h
;MRNORM_MESS	= 8001h
;CONTACT_MESS	= 0013h
;MRCONTACT_MESS	= 8013h
;
;
;
;CAP_PRECAP	= "\x09\x46\x13"
;CAP_PRERTF	= "\x97\xb1\x27"
;CAP_POSCAP	= "\x4c\x7f\x11\xd1\x82\x22\x44\x45\x53\x54\x00\x00"
;CAP_POSRTF	= "\x24\x3c\x43\x34\xad\x22\xd6\xab\xf7\x3f\x14\x92"

;
;Fingerprinting Capabilities
;
;CAP_M2001	= "\x2e\x7a\x64"									"\x75"										"\xfa\xdf\x4d\xc8\x88\x6f\xea\x35\x95\xfd\xb6\xdf"
;CAP_M2001_2	= "\xa0\xe9\x3f"									"\x37"										"\x4f\xe9\xd3\x11\xbc\xd2\x00\x04\xac\x96\xdd\x96"	
;CAP_M2002	= "\x10\xcf\x40"									"\xd1"										"\x4f\xe9\xd3\x11\xbc\xd2\x00\x04\xac\x96\xdd\x96"
;CAP_MLITE	= "\x56\x3f\xc8"									"\x09"										"\x0b\x6f\x41\xbd\x9f\x79\x42\x26\x09\xdf\xa2\xf3"
;CAP_SIMICQ	= "\x97\xb1\x27"									"\x51"										"\x24\x3c\x43\x34\xad\x22\xd6\xab\xf7\x3f\x14\x48"
;CAP_MICQ	= "mICQ \xa9 R.K. \x00\x00\x00\x00"
;CAP_TRILL_NORM	= "\x97\xb1\x27"									"\x51"										"\x24\x3c\x43\x34\xad\x22\xd6\xab\xf7\x3f\x14\x09"
;CAP_TRILL_CRYPT= "\xf2\xe7\xc7"									"\xf4"										"\xfe\xad\x4d\xfb\xb2\x35\x36\x79\x8b\xdf\x00\x00"
;CAP_LICQ	= "\x09\x49\x13"	  

;
;DC Packet Types
;
;PEER_INIT		= 0ffh
;PEER_INITACK		= 01h
;PEER_MSG		= 02h
;PEER_INIT2		= 03h
;PEER_FILE_INIT		= 00h
;PEER_FILE_INIT_ACK	= 01h
;PEER_FILE_START		= 02h
;PEER_FILE_START_ACK	= 03h
;PEER_FILE_STOP		= 04h
;PEER_FILE_SPEED		= 05h
;PEER_FILE_DATA		= 06h


ICQ_PORT 	= 5190
;
; FLAP transport
;
FLAP_ID		= 02ah

struc FLAP_head
{
	.bId db FLAP_ID ;id byte 
	.bCh db ? ;channel
	.wSn dw ? ;seq number
	.wDs dw ? ;data size
}
;
; Channels ID
;

NEW_CONNECTION		= 01h
SNAC_DATA		= 02h
FLAP_ERROR		= 03h
CLOSE_CONNECTION	= 04h
KEEP_ALIVE		= 05h

;
; SNAC
; 
struc SNAC_head
{
	.wFid dw ?; Family id
	.wSid dw ?; subtype id
	.wDf  dw ?; SNAC flags
	.dRi  dd ?; SNAC Request id
}

;
;
; Familes/SNACs list
;

GENERIC_SN	= 0001h
LOCATION_SN	= 0002h
BUDDY_LIST_SN	= 0003h
ICBM_SN		= 0004h
PRIVACY_SN	= 0009h
BUDDY_ICONS_SN	= 0010h
SSI_SN		= 0013h
AUTH_REG_SN	= 0017h

;
; TLV
;
struc TLV_head 
{
 	.wTn dw ?; TLV type number
	.wLv dw ?; TLV length value
}

;
; userinfo block
;
struc UI_head
{
	.bUinLength    db 0        ; UIN/screenname length
	.bUin          db 11 dup 0 ; string
	.wWl           dw 0        ; Warning level
	.dUserClass    dd 0
	.dCreateTime   dd 0
	.dSignonTime   dd 0
	.wIdleTime     dw 0
	.dCreationTime dd 0
	.dUserStatus   dd 0
	.dIpAddress    dd 0
	.dOnlineTime   dd 0

}

;
;Roasting array
;
ROASTING_ARRAY db 0F3h, 026h, 081h, 0C4h, 039h, 086h, 0DBh, 092h, 071h, 0A3h, 0B9h, 0E6h, 053h, 07Ah, 095h, 07Ch

;
; Status flags
;
;
 
  STATUS_WEBAWARE   = 0x0001  	  ;Status webaware flag
  STATUS_SHOWIP     = 0x0002	  ;Status show ip flag
  STATUS_BIRTHDAY   = 0x0008 	  ;User birthday flag
  STATUS_WEBFRONT   = 0x0020 	  ;User active webfront flag
  STATUS_DCDISABLED = 0x0100	  ;Direct connection not supported
  STATUS_DCAUTH     = 0x1000	  ;Direct connection upon authorization
  STATUS_DCCONT     = 0x2000	  ;DC only with contact users

;
; Status
;

  STATUS_ONLINE    = 0x0000	  ;Status is online
  STATUS_AWAY      = 0x0001	  ;Status is away
  STATUS_DND  	   = 0x0002	  ;Status is no not disturb (DND)
  STATUS_NA  	   = 0x0004	  ;Status is not available (N/A)
  STATUS_OCCUPIED  = 0x0010	  ;Status is occupied (BISY)
  STATUS_FREE4CHAT = 0x0020 	  ;Status is free for chat
  STATUS_INVISIBLE = 0x0100 	  ;Status is invisible


