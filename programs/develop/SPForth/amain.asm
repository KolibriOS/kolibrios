

reserve EQU Mreserve-main_task



amain:	
	MOV	[ByeLevel],ESP
        mov edi, main_task
	call	cfa_INIT
	ret

TIB_SIZE EQU 1027
PAD_SIZE EQU 1027

include 'img.asm'

cfa_AHEADER 0,"BYE",_BYE
	MOV	ESP,[ByeLevel]
	RET


cfa_AHEADER 0,"_SLITERAL-CODE",_SLITERALminusCODE
	LEA	EBP, [EBP-8]
	MOV	[EBP+4], EAX
	POP	EBX
	MOVZX	EAX, BYTE [EBX]
	LEA	EBX, [EBX+1]
	MOV	[EBP], EBX
	LEA	EBX, [EBX+EAX]
	LEA	EBX, [EBX+1]
	JMP	EBX



cfa_AHEADER 0,"_CONSTANT-CODE",_CONSTANTminusCODE         
	LEA	EBP,[EBP-4]
	MOV	[EBP],EAX
	POP	EAX
	MOV	EAX,[EAX]
	RET   

cfa_AHEADER 0,"_CREATE-CODE",_CREATEminusCODE
 DB 083H ,0EDH ,4 
 DB 089H ,045H ,0 
 DB 058H 
 DB 0C3H 

cfa_AHEADER 0,"_TOVALUE-CODE",_TOVALUEminusCODE
 DB 05BH
 DB 08DH ,05BH ,0F7H
 DB 089H ,03 
 DB 08BH ,045H ,00 
 DB 08DH ,06DH ,04 
 DB 0C3H

cfa_AHEADER 0,"TOVALUE-CODE",TOVALUEminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__TOVALUEminusCODE
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"VECT-CODE",VECTminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__VECTminusCODE
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"_VECT-CODE",_VECTminusCODE
 DB 05BH
 DB 0FFH ,023H

cfa_AHEADER 0,"_USER-VALUE-CODE", _USERminusVALUEminusCODE
	LEA	EBP,[EBP-4]
	MOV	[EBP],EAX
	POP	EAX
	MOV	EAX,[EAX]
	LEA	EAX,[EDI+EAX]
	MOV	EAX,[EAX]
	RET

cfa_AHEADER 0,"USER-VALUE-CODE",USERminusVALUEminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__USERminusVALUEminusCODE
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"_TOUSER-VALUE-CODE",_TOUSERminusVALUEminusCODE
 DB 05BH 
 DB 083H ,0EBH ,09 
 DB 08BH ,01BH 
 DB 03 ,0DFH
 DB 089H ,03 
 DB 08BH ,045H ,00 
 DB 083H ,0C5H ,04 
 DB 0C3H

cfa_AHEADER 0,"TOUSER-VALUE-CODE",TOUSERminusVALUEminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__TOUSERminusVALUEminusCODE
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"_USER-CODE", _USERminusCODE
	LEA	EBP,[EBP-4]
	MOV	[EBP],EAX
	POP	EAX
	MOV	EAX,[EAX]
	LEA	EAX,[EDI+EAX]
	RET

cfa_AHEADER 0,"DOES>A",DOESgreatA
 call cfa__USERminusCODE
 DD T_DOESgreatA-main_task

cfa_AHEADER 0,"USER-CODE",USERminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__USERminusCODE
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"CREATE-CODE",CREATEminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__CREATEminusCODE
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"CONSTANT-CODE",CONSTANTminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__CONSTANTminusCODE
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"_CLITERAL-CODE",_CLITERALminusCODE
 DB 083H ,0EDH ,04 
 DB 089H ,045H ,00 
 DB 058H 
 DB 0FH ,0B6H ,018H
 DB 08DH ,05CH ,03 ,02 
 DB 0FFH ,0E3H 

cfa_AHEADER 0,"CLITERAL-CODE",CLITERALminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__CLITERALminusCODE
 call cfa__TOVALUEminusCODE 


cfa_AHEADER 0,'BASE',BASE
 call cfa__USERminusCODE
 DD T_BASE-main_task

cfa_AHEADER 0,'PAD',PAD
 call cfa__USERminusCODE
 DD T_PAD-main_task

cfa_AHEADER 0,'LAST',LAST
 call cfa__CREATEminusCODE 
 DD LastNFA

 cfa_AHEADER 0,'DP',DP
 call cfa__CONSTANTminusCODE
 DD CP_P ; DP_BUFF

cfa_AHEADER 0,'''DROP_V',ticDROP_V
 call cfa__CONSTANTminusCODE+00H
 DD cfa_DROP
 call cfa__TOVALUEminusCODE+00H

cfa_AHEADER 0,'''DUP_V',ticDUP_V

 call cfa__CONSTANTminusCODE
 DD cfa_DUP
 call cfa__TOVALUEminusCODE+00H

cfa_AHEADER 0,'CONTEXT',_CONTEXT
 call cfa__USERminusVALUEminusCODE
 DD T_CONTEXT-main_task
 call cfa__TOUSERminusVALUEminusCODE

cfa_AHEADER 0,'>IN',greatIN
 call cfa__USERminusCODE
 DD T_greatIN-main_task

cfa_AHEADER 0,'>OUT',greatOUT
 call cfa__CREATEminusCODE 
TO_OUT   dd 0

cfa_AHEADER 0,'CURFILE',CURFILE
 call cfa__USERminusCODE
 DD T_CURFILE-main_task

cfa_AHEADER 0,'S0',S0
 call cfa__USERminusCODE
 DD T_S0-main_task

cfa_AHEADER 0,'R0',R0
 call cfa__USERminusCODE
 DD T_R0-main_task

cfa_AHEADER 0,'SOURCE-ID',SOURCEminusID
 call cfa__USERminusVALUEminusCODE
 DD T_SOURCEminusID-main_task
 call cfa__TOUSERminusVALUEminusCODE

cfa_AHEADER 0,'TIB',TIB
 call cfa__USERminusVALUEminusCODE
 DD T_TIB-main_task
 call cfa__TOUSERminusVALUEminusCODE

cfa_AHEADER 0,'#TIN',nTIB
 call cfa__USERminusCODE
 DD T_nTIB-main_task

cfa_AHEADER 0,'CURSTR',CURSTR
 call cfa__USERminusCODE
 DD T_CURSTR-main_task

cfa_AHEADER 0,'SLITERAL-CODE',SLITERALminusCODE
 call cfa__CONSTANTminusCODE 
 DD cfa__SLITERALminusCODE
 call cfa__TOVALUEminusCODE 

cfa_AHEADER 0,'USER-OFFS',USERminusOFFS
 call cfa__CREATEminusCODE 
 DD reserve+MUSEROFFS

cfa_AHEADER 0,'HANDLER',HANDLER
 call cfa__USERminusCODE
 DD T_HANDLER-main_task

cfa_AHEADER 0,'STATE',STATE
 call cfa__USERminusCODE
 DD T_STATE-main_task

cfa_AHEADER 0,'CURRENT',CURRENT
 call cfa__USERminusCODE
 DD T_CURRENT-main_task

cfa_AHEADER 0,'W-CNT',WminusCNT
 call cfa__USERminusCODE
 DD T_WminusCNT-main_task

cfa_AHEADER 0,'S-O',SminusO
 call cfa__USERminusCODE
 DD T_SminusO-main_task

cfa_AHEADER 0,'ER-U',ERminusU
 call cfa__USERminusCODE
 DD T_ERminusU-main_task

cfa_AHEADER 0,'ER-A',ERminusA
 call cfa__USERminusCODE
 DD T_ERminusA-main_task

cfa_AHEADER 0,'FORTH-WORDLIST',FORTHminusWORDLIST
 call cfa__CONSTANTminusCODE 
 DD T_FORTH+4
 call cfa__TOVALUEminusCODE 

cfa_AHEADER 0,'VOC-LIST',VOCminusLIST
 call cfa__CREATEminusCODE 
 DD T_FORTH

cfa_AHEADER 0,'WARNING',WARNING
 call cfa__USERminusCODE
 DD T_WARNING-main_task

;cfa_AHEADER 0,'LAST_KEY',LAST_KEY
; call cfa__CREATEminusCODE 
;	DD last_key

cfa_AHEADER 0,"BLK",BLK
 call cfa__USERminusCODE
 DD T_BLK-main_task



;cfa_AHEADER 0,"UZERO",UZERO_M
; call cfa__CONSTANTminusCODE 
; DD UZERO

cfa_AHEADER 0,"UPP",UPP_M
 call cfa__CONSTANTminusCODE 
 DD UPP

cfa_AHEADER 0,"ULAST",ULAST_M
 call cfa__CONSTANTminusCODE 
 DD ULAST

cfa_AHEADER 0,"SPP",SPP_M
 call cfa__CONSTANTminusCODE 
 DD SPP
cfa_AHEADER 0,"TIBB",TIBB_M
 call cfa__CONSTANTminusCODE 
 DD TIBB

cfa_AHEADER 0,"#TIB",NTIB_M
 call cfa__CONSTANTminusCODE 
 DD NTIB_P

cfa_AHEADER 0,"sbuf",screen_buf_M
 call cfa__CONSTANTminusCODE 
 DD screen_buf

cfa_AHEADER 0,"cursor",cursor_M
 call cfa__CONSTANTminusCODE 
 DD cursor

cfa_AHEADER 0,"FINFO",FINFO
 call cfa__CREATEminusCODE 
 DD FINFO

cfa_AHEADER 0,"ROWH",ROWH
 call cfa__CONSTANTminusCODE 
 DD ROWH
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"ROWW",ROWW
 call cfa__CONSTANTminusCODE 
 DD 6
 call cfa__TOVALUEminusCODE

cfa_AHEADER 0,"MEMS",MEMS
 call cfa__CONSTANTminusCODE 
 DD MEMS

cfa_AHEADER 0,"draw_window",draw_window
	PUSH EDI
	CALL draw_window
	POP EDI
	RET

; cfa_AHEADER 0,'MEBLK',MEBLK
; call cfa__USERminusCODE
; DD BLK_P-main_task

main_task:
T_R0:
	DD 0
T_S0:
	DD SPP ;STACK0
T_WARNING:
	DD -1
T_STATE:
	DD 0
T_BLK:
	DD 0
T_CURFILE:
	DD 0
T_HANDLER:
	DD 0
T_HLD:
	DD 5
T_BASE:
	DD 0AH
	DD PAD_SIZE DUP (0)
T_PAD:
	DD PAD_SIZE DUP (0)
T_ERminusA:
	DD 0
T_ERminusU:
	DD 0
T_DOESgreatA:
	DD 0
;T_ALIGNminusBYTES:
	DD 0
T_nTIB:
	DD 0
T_greatIN:
	DD 5
T_TIB:
	DD T_ATIB
T_ATIB:
	DD TIB_SIZE DUP (0)
T_SOURCEminusID:
	DD 0
T_CURSTR:
	DD 0
T_WBWminusNFA:
	DD 0
T_WBWminusOFFS:
	DD 0
T_CURRENT:
	DD T_FORTH+4
T_SminusO:
	DD T_FORTH+4,T_FORTH+4
	DD 16 DUP (0)
T_CONTEXT:
	DD T_SminusO
T_greatOUT:
	DD 0
T_WminusCNT:
	DD 0
T_NNN:
	DD 0
Mreserve:
	DD MUSEROFFS DUP (0)
UPP:

BASE_P    	DD	BASEE
TEMP_P     	DD	0			
INN_P     	DD	0			
NTIB_P    	DD	0			
TIB_P     	DD	TIBB			
;TEVAL_P   	DD	INTER			
HLD_P     	DD	0			
CNTXT_P 	DD	0	
CP_P      	DD	CTOP
;LAST_P      DD  LASTN                   ;LAST
EMIT_PROC_P      DD  -1  ; EMITPROC
reg_struc_P      DD  0
; BLK_P		DD  FILE_B
; fi_struc_P      DD  FINFO
workarea_P      DD  os_work

	DD 1000 DUP (0)

T_FORTH:
	DD 0			; для VOC-LIST
	DD LastNFA		; адрес последнего имени !!!!!!!!
	DD 0			; предок
	DD 0			; класс

ByeLevel DD 0
