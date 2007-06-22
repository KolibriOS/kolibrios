;=============================================================================
;
; MGB - Menuet Graphics Benchmark 0.3
; Compile with FASM
;
;=============================================================================
;
; Original author and copyrights holder:
;     Mikhail Lisovin a.k.a. Mihasik
;     lisovin@26.ru
;
; Disassembled with IDA 5.0.0.879:
;     http://www.datarescue.com/
; With use of 'ida.int' and 'kloader.ldw':
;     Eugene Grechnikov a.k.a. diamond
;     diamondz@land.ru
;     http://diamondz.land.ru/
;
; Modified for version 0.3:
;     Mike Semenako a.k.a mike.dld
;     mike.dld@gmail.com
;     http://www.mikedld.com/
;
;=============================================================================

org 0x0
use32

  db 'MENUET01'
  dd 1
  dd start
  dd APP_MEM_END
  dd 0x200000
  dd 0x07FFF0
  dd 0
  dd 0

include '..\..\..\macros.inc'
;__CPU_type	equ	p6  ; charge it

include 'proc32.inc'

; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		start			; DATA XREF: seg000:off_Co

; FUNCTION CHUNK AT 000000CF SIZE 00000147 BYTES

		mov	ecx, -1
		call	subGetThreadInfo
		mov	edx, [dwMainPID]
		mov	ecx, eax

loc_36: 				; CODE XREF: start+20j
		call	subGetThreadInfo

loc_3B:
		cmp	edx, [dwMainPID]
		jz	loc_46
		dec	ecx
		jnz	loc_36

loc_46: 				; CODE XREF: start+1Dj
		mov	[dwMainWndSlot], ecx

locRedrawEvent: 			; CODE XREF: start+3Cj
		call	subDrawMainWindow

locWaitForEvent:			; CODE XREF: start+6Cj start:loc_B3j ...
		mcall	23, 20		; Kolibri - WAIT FOR EVENT WITH TIMEOUT
					; ebx = timeout
					; Return: eax = event
		cmp	eax, 1
		jz	locRedrawEvent

loc_62:
		cmp	eax, 2
		jz	locKeyEvent
		cmp	eax, 3

loc_6A:
		jz	locButtonEvent

loc_70:
		;btr     word[wFlags], 2
		;jnb     loc_87
		;mov     eax, 58
		;mov     ebx, stFileInfoBlock
		;int     0x40             ; Kolibri - ACCESS TO FILE SYSTEM (obsolete)
					; ebx -> fileinfo struc

loc_87: 				; CODE XREF: start+55j
		btr	word[wFlags], 1
		jnb	locWaitForEvent
		call	subDelay100ms
		mcall	0x12, 3, [dwMainWndSlot]	; Kolibri - ACTIVATE WINDOW
					; ecx = slot
		call	subDelay100ms

loc_AE:
		call	subDrawBars

loc_B3:
		jmp	locWaitForEvent
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subGetThreadInfo	; CODE XREF: start+5p start:loc_36p
		mcall	9, APP_MEM_END	; Kolibri - GET THREAD INFO
					; ebx -> buffer, ecx = slot (-1 for self)
					; Return: eax = maximum slot
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subDelay100ms		; CODE XREF: start+6Ep start+85p
		mcall	5, 0x0a 	; Kolibri - DELAY
					; ebx = time (in 1/100th of second)
		retn
endp

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
; START OF FUNCTION CHUNK FOR start

locKeyEvent:				; CODE XREF: start+41j
		mcall	2		; Kolibri - GET KEY CODE
					; Return: ah = keycode
		cmp	ah, 't'
		jz	locActionTest
		cmp	ah, 'c'
		jz	locActionComment
		cmp	ah, 'p'
		jz	locActionPattern
		cmp	ah, 'o'
		jz	locActionOpen
		cmp	ah, 's'
		jz	locActionSave
		jmp	locWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locButtonEvent: 			; CODE XREF: start:loc_6Aj
		mcall	0x11		; Kolibri - GET PRESSED BUTTON
					; Return: ah = button ID
		cmp	ah, 1
		jnz	locNotClose
		mcall	-1		; Kolibri - FINISH EXECUTION

locNotClose:				; CODE XREF: start+E6j
		cmp	ah, 2
		jnz	locNotTest

locActionTest:				; CODE XREF: start+B5j
		bts	[wFlags], 0
		jb	locWaitForEvent
		mcall	5, 50		; Kolibri - DELAY
					; ebx = time (in 1/100th of second)

loc_132:
		mcall	51, 1, subTestWndProc, 0x17FFF0; Kolibri - CREATE THREAD
					; ebx = 1 - unique subfunction
					; ecx = starting eip
					; edx = starting esp
		jmp	locWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locNotTest:				; CODE XREF: start+F2j
		cmp	ah, 3
		jnz	locNotComment

locActionComment:			; CODE XREF: start+BAj
		bts	[wFlags], 0
		jb	locWaitForEvent
		mov	[dwBufferPtr], aComment1 ; "Current                               "...
		mov	[dwEditLabel], aComment ; "Comment"
		mov	[dwEditLabelLen], 7
		call	subCreateOpenSaveDlg
		jmp	locWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locNotComment:				; CODE XREF: start+12Cj
		cmp	ah, 4
		jnz	locNotPattern

locActionPattern:			; CODE XREF: start+BFj
		call	subSavePattern
		call	subDrawBars
		jmp	locWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locNotPattern:				; CODE XREF: start+167j
		cmp	ah, 5
		jnz	locNotOpen

locActionOpen:				; CODE XREF: start+C8j
		bts	[wFlags], 0
		jb	locWaitForEvent
		mov	[dwBufferPtr], aPatternPath ; "/rd/1/pattern.mgb                             "...
		mov	[dwEditLabel], aOpenFile	; "Open file"
		mov	[dwEditLabelLen], 9
		call	subCreateOpenSaveDlg
		jmp	locWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locNotOpen:				; CODE XREF: start+17Bj
		cmp	ah, 6
		jnz	locWaitForEvent

locActionSave:				; CODE XREF: start+D1j
		bts	[wFlags], 0
		jb	locWaitForEvent
		mov	[dwBufferPtr], aPatternPath ; "/rd/1/pattern.mgb                             "...
		mov	[dwEditLabel], aSaveAs ;        "Save as..."
		mov	[dwEditLabelLen], 0Ah
		call	subCreateOpenSaveDlg
		jmp	locWaitForEvent
; END OF FUNCTION CHUNK FOR start

; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subCreateOpenSaveDlg	  ; CODE XREF: start+15Ap start+1A9p ...
		mcall	51, 1, subOpenSaveDlgProc, 0x19FFF0; Kolibri - CREATE THREAD
					; ebx = 1 - unique subfunction
					; ecx = starting eip
					; edx = starting esp
		retn
endp

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

subTestWndProc: 			; DATA XREF: start+118o

		mov	esi,results_table+8
  .next_test:	call	subInitTestTimer

align 4
	@@:	push	esi
		call	dword[esi]
		pop	esi
		call	subIfTimeElapsed
		jb	@b
		mov	[esi-8],edi

		add	esi,TEST_REC_SIZE
		cmp	dword[esi],0
		jnz	.next_test
macro unused {
		call	subInitTestTimer

locDrawNextWindow:			; CODE XREF: seg000:00000241j
		mov	ecx, 4F0190h
		call	testDrawWindow
		call	subIfTimeElapsed
		jb	locDrawNextWindow
		mov	[results_table+TEST_REC_SIZE*0], edi
		call	subInitTestTimer

locDrawNextBar: 			; CODE XREF: seg000:00000269j
		mcall	0x0d, 0x0A0064, 0x1E00FA, 0x6A73D0; Kolibri - DRAW RECTANGLE
					; ebx = [xstart]*65536+[xsize], ecx = [ystart]*65536+[ysize]
					; edx = 0xRRGGBB or 0x80RRGGBB for gradient
		call	subIfTimeElapsed
		jb	locDrawNextBar
		mov	[results_table+TEST_REC_SIZE*1], edi
		call	subInitTestTimer

locDrawNextLine:			; CODE XREF: seg000:00000291j
		; Kolibri - DRAW LINE
		mcall	38, 0x8C008C, 0x1E017C, 0x1090207F
					; ebx = [xstart]*65536+[xend], ecx = [ystart]*65536+[yend]
					; edx = 0x00RRGGBB - color or 0x01****** - inversed line
		call	subIfTimeElapsed
		jb	locDrawNextLine
		mov	[results_table+TEST_REC_SIZE*2], edi
		call	subInitTestTimer

locDrawNextText1:			; CODE XREF: seg000:000002BEj
		; Kolibri - DRAW STRING
		mcall	4, 0x0A012C, 0x0AA66, aTestText, 34
					; ebx = [xstart]*65536+[ystart]
					; ecx = 0xX0RRGGBB, edx -> string
		call	subIfTimeElapsed
		jb	locDrawNextText1
		mov	[results_table+TEST_REC_SIZE*3], edi
		call	subInitTestTimer

locDrawNextText2:			; CODE XREF: seg000:000002EBj
		; Kolibri - DRAW STRING
		mcall	4, 0x0A015E, 0x10E7B850, aTestText, 34
					; ebx = [xstart]*65536+[ystart]
					; ecx = 0xX0RRGGBB, edx -> string
		call	subIfTimeElapsed
		jb	locDrawNextText2
		mov	[results_table+TEST_REC_SIZE*4], edi
		call	subInitTestTimer

locDrawNextNumber:			; CODE XREF: seg000:00000318j
					; Kolibri - DRAW NUMBER
					; bl = 0/1 - ecx is number/pointer
					; bh = 0/1/2 - dec/hex/bin
					; highword(ebx) = number of digits
					; ecx = number/pointer
					; edx = [x]*65536+[y]
					; esi = 0xX0RRGGBB
		mcall	47, 80000h, 12345678, 140172h, 0E0B27Bh



		call	subIfTimeElapsed
		jb	locDrawNextNumber
		mov	[results_table+TEST_REC_SIZE*5], edi
		call	subInitTestTimer

locDrawNextPixel:			; CODE XREF: seg000:00000340j
					; Kolibri - PUT PIXEL
					; ebx = x, ecx = y, edx = color
		mcall	1, 100, 100, 0x0FFFFFF
		call	subIfTimeElapsed
		jb	locDrawNextPixel
		mov	[results_table+TEST_REC_SIZE*6], edi
}
		bts	word[wFlags], 1
		btr	word[wFlags], 0
		; Kolibri - FINISH EXECUTION
		mcall	-1

; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subInitTestTimer	; CODE XREF: seg000:subTestWndProcp
					; seg000:00000249p ...
		xor	edi, edi
		mov	eax, 26
		mov	ebx, 9
		mcall		 ; Kolibri - GET SYSTEM PARAMETERS - TIME COUNTER
					; Return: eax = time counter
		inc	eax
		mov	ecx, eax
		add	eax, 100
		mov	[dwTestEndTime], eax

locWait10ms:				; CODE XREF: subInitTestTimer+22j
		mov	eax, 26
		mcall		 ; Kolibri - GET SYSTEM PARAMETERS - TIME COUNTER
					; Return: eax = time counter
		cmp	eax, ecx
		jb	locWait10ms
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subIfTimeElapsed	; CODE XREF: seg000:0000023Cp
					; seg000:00000264p ...
		inc	edi
		mov	eax, 26
		mov	ebx, 9
		mcall		 ; Kolibri - GET SYSTEM PARAMETERS - TIME COUNTER
					; Return: eax = time counter
		cmp	eax, [dwTestEndTime]
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subSavePattern		; CODE XREF: start:locActionPatternp
;               mov     esi, dwDrawWindowTime
;               mov     edi, dwDrawWindowTime2
;               mov     ecx, 18
;               cld
;               rep movsd
		mov	esi,results_table
		cld
	    @@: lodsd
		mov	[esi],eax
		add	esi,TEST_REC_SIZE-4
		cmp	dword[esi+TEST_REC_SIZE-4],0
		jne	@b
		mov	esi,aComment1
		mov	edi,aComment2
		mov	ecx,44/4
		rep	movsd
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subDrawMainWindow	; CODE XREF: start:locRedrawEventp
		mov	eax, 12
		mov	ebx, 1
		mcall		 ; Kolibri - BEGIN WINDOW REDRAW
		mov	eax,48
		mov	ebx,4
		mcall
		mov	ebx, 100*65536+72*5+14;640145h
		mov	ecx, 80*65536+TESTS_NUM*LINE_HEIGHT+15+20+35
		add	cx, ax
		mov	edx, 34000000h
		mov	edi, aCaption
		xor	eax, eax
		mcall
		mov	eax, 8
		mov	ebx, 050036h+12
		mov	ecx, 5*65536+20
		mov	edx, 2
		mov	esi, 0x00007F7F;702050h

locDrawButtonsLoop:			; CODE XREF: subDrawMainWindow+3Bj
		mcall		 ; Kolibri - DEFINE/DELETE BUTTON
					; ebx = [xstart]*65536+[xsize]
					; ecx = [ystart]*65536+[ysize]
					; edx = 0xXYnnnnnn, esi = color
		add	ebx, 72*65536
		inc	edx
		cmp	edx, 7
		jb	locDrawButtonsLoop

		mov	ecx,31
		mov	edx,0x00007F7F
		mov	esi,(72*5)/2
		call	drawSeparator

		mov	eax, 4
		mov	ebx, 27*65536+12
		mov	ecx, 0x80DDEEFF
		mov	edx, aButtonsText ; "Test    Comment+  Pattern+   Open     "...
		mcall		 ; Kolibri - DRAW STRING
					; ebx = [xstart]*65536+[ystart]
					; ecx = 0xX0RRGGBB, edx -> string
		call	subDrawBars

		mov	ecx,TESTS_NUM*LINE_HEIGHT+15+21
		mov	edx,0x00007F7F
		mov	esi,(72*5)/2
		call	drawSeparator

		mov	eax, 12
		mov	ebx, 2
		mcall		 ; Kolibri - END WINDOW REDRAW
		retn
endp

proc		drawSeparator
		mov	eax,1
		mov	ebx,3
	    @@: mcall
		add	ebx,2
		dec	esi
		jnz	@b
		retn
endp

; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		testDrawWindow
		xor	eax, eax
		mov	ebx, 640145h
		mov	ecx, 4F0190h
		mov	edx, 3000000h
		mcall
		retn
endp

proc		testDrawBar
		mov	eax, 13
		mov	ebx, 0A0064h
		mov	ecx, 1E00FAh
		mov	edx, 6A73D0h
		mcall
		retn
endp

proc		testDrawPicture
		mov	eax, 7
		mov	ebx, 0
		mov	ecx, 90*65536+123
		mov	edx, 15*65536+33
		mcall
		retn
endp

proc		testDrawVertLine
		mov	eax, 38
		mov	ebx, 300*65536+300 ;8C008Ch
		mov	ecx, 30*65536+380  ;1E017Ch
		mov	edx, 1090207Fh
		mcall
		retn
endp

proc		testDrawHorzLine
		mov	eax, 38
		mov	ebx, 30*65536+300  ;20008Ch
		mov	ecx, 380*65536+380 ;17C017Ch
		mov	edx, 1090207Fh
		mcall
		retn
endp

proc		testDrawFreeLine
		mov	eax, 38
		mov	ebx, 30*65536+300  ;20008Ch
		mov	ecx, 380*65536+30  ;17C001Eh
		mov	edx, 1090207Fh
		mcall
		retn
endp

proc		testDrawText1
		mov	eax, 4
		mov	ebx, 0C012Ch
		mov	ecx, 0AA66h
		mov	edx, aTestText
		mov	esi, 34
		mcall
		retn
endp

proc		testDrawText2
		mov	eax, 4
		mov	ebx, 1B013Bh
		mov	ecx, 10E7B850h
		mov	edx, aTestText
		mov	esi, 34
		mcall
		retn
endp

proc		testDrawNumber
		mov	eax, 47
		mov	ebx, 80000h
		mov	ecx, 12345678
		mov	edx, 2A014Ah
		mov	esi, 0E0B27Bh
		mcall
		retn
endp

proc		testDrawPixel
		mov	eax, 1
		mov	ebx, 100
		mov	ecx, 100
		mov	edx, 0FFFFFFh
		mcall
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subDrawBars		; CODE XREF: start:loc_AEp start+16Ep ...
		mov	edi,results_table
		mov	ebx,30+7
  .next_result: cmp	dword[edi+TEST_REC_SIZE-4],0
		je	.exit

		push	ebx
		mov	eax,13
		movzx	ecx,bx
		add	ecx,-2
		shl	ecx,16
		mov	cx,LINE_HEIGHT
		mov	ebx,0*65536+72*5+5
		xor	edx,edx
		mcall
		pop	ebx

		and	ebx,0x0000FFFF
		or	ebx,5*65536
		mov	edx,[edi+TEST_REC_SIZE-4]
		mov	ecx,0x8000CCCC ; 0x00E7E05A
		mov	eax,4
		mcall

		push	'=' 0x00FFFF00 0x00FFFF7F 0x00FFFF7F
		mov	eax,[edi+0]
		cmp	eax,[edi+4]
		je	@f
		jb	.lp1
		mov	dword[esp+0],0x007FFF7F
		mov	dword[esp+4],0x00FF7F7F
		mov	dword[esp+8],0x0000FF00
		mov	byte[esp+12],'>'
	  .lp1: ja	@f
		mov	dword[esp+0],0x00FF7F7F
		mov	dword[esp+4],0x007FFF7F
		mov	dword[esp+8],0x00FF0000
		mov	byte[esp+12],'<'
	    @@:
		pop	ecx
		call	int2str
		add	ebx,(72*5-6*8*2-6-10-5)*65536 ; 196
		mov	edx,APP_MEM_END
		mov	esi,8
		mov	eax,4
		mcall

		pop	ecx
		mov	eax,[edi+4]
		call	int2str
		add	ebx,(6*8+6+10)*65536
		mov	eax,4
		mcall

		pop	ecx
		add	ebx,(-6-5)*65536
		mov	edx,esp
		mov	esi,1
		mcall
		add	esp,4

		add	edi,TEST_REC_SIZE
		add	bx,LINE_HEIGHT
		jmp	.next_result
  .exit:

		mov	eax, 13
		mov	ebx, 0*65536+72*5+5
		mov	ecx, (TESTS_NUM*LINE_HEIGHT+15+25)*65536+26
		xor	edx, edx
		mcall

		mov	eax, 4
		mov	ebx, 5*65536+(TESTS_NUM*LINE_HEIGHT+15+27)
		mov	ecx, 0x8000CCCC
		mov	edx, aLeft
		mcall		 ; Kolibri - DRAW STRING

		add	ebx, (6*10)*65536
		mov	ecx, 0x00FFFF00
		mov	edx, aComment1
		mov	esi, 42
		mcall		 ; Kolibri - DRAW STRING

		mov	eax, 4
		mov	ebx, 5*65536+(TESTS_NUM*LINE_HEIGHT+15+27+12)
		mov	ecx, 0x8000CCCC
		mov	edx, aRight
		mcall		 ; Kolibri - DRAW STRING

		add	ebx, (6*10)*65536
		mov	ecx, 0x00FFFF00
		mov	edx, aComment2
		mov	esi, 42
		mcall		 ; Kolibri - DRAW STRING

		retn
endp

proc		int2str
		push	eax ecx edx edi
		mov	edi,APP_MEM_END+7
		mov	dword[APP_MEM_END+0],'    '
		mov	dword[APP_MEM_END+4],'    '
		mov	ecx,10
	    @@: xor	edx,edx
		div	ecx
		add	dl,'0'
		mov	[edi],dl
		dec	edi
		or	eax,eax
		jnz	@b
		pop	edi edx ecx eax
		retn
endp

; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subDrawResultComments	   ; CODE XREF: subDrawBars+92p
					; subDrawBars+AFp ...
		xor	di, di
		mov	eax, 4

locDrawCommentsLoop:			; CODE XREF: subDrawResultComments+1Cj
		mcall		 ; Kolibri - DRAW STRING
					; ebx = [xstart]*65536+[ystart]
					; ecx = 0xX0RRGGBB, edx -> string
		add	bx, 13;55
		bt	edi, 31
		jnb	loc_52F
		add	edx, esi

loc_52F:				; CODE XREF: subDrawResultComments+12j
		inc	di
		cmp	di, 7
		jb	locDrawCommentsLoop
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subDrawResultNumbers	  ; CODE XREF: subDrawBars+CDp
					; subDrawBars+DCp
		xor	edi, edi
		mov	eax, 47
		mov	esi, 0DDEEFFh

locDrawNumbersLoop:			; CODE XREF: subDrawResultNumbers+1Ej
		call	subGetDigitsCount
		mcall		 ; Kolibri -
		add	dx, 13;55
		add	ecx, 4
		inc	edi
		cmp	edi, 7
		jb	locDrawNumbersLoop
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subGetDigitsCount	; CODE XREF: subDrawResultNumbers:locDrawNumbersLoopp
		xor	ebx, ebx
		mov	bh, 6
		cmp	dword[ecx], 99999
		ja	loc_589
		dec	bh
		cmp	dword[ecx], 9999
		ja	loc_589
		dec	bh
		cmp	dword[ecx], 999
		ja	loc_589
		dec	bh
		cmp	dword[ecx], 99
		ja	loc_589
		dec	bh
		cmp	dword[ecx], 9
		ja	loc_589
		dec	bh

loc_589:				; CODE XREF: subGetDigitsCount+Aj
					; subGetDigitsCount+14j ...
		bswap	ebx
		inc	bl
		retn
endp

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

subOpenSaveDlgProc:			; CODE XREF: seg000:0000059Dj
					; DATA XREF: subCreateOpenSaveDlg+Ao
		call	subDrawOpenSaveDlg

locOSDWaitForEvent:			; CODE XREF: seg000:000005ADj
					; seg000:000005C3j ...
		mov	eax, 10
		mcall		 ; Kolibri -
		cmp	eax, 1
		jz	subOpenSaveDlgProc
		cmp	eax, 2
		jz	locOSDKeyEvent
		cmp	eax, 3
		jz	locOSDButtonEvent
		jmp	locOSDWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locOSDKeyEvent: 			; CODE XREF: seg000:000005A2j
		mov	eax, 2
		mcall		 ; Kolibri - GET KEY CODE
					; Return: ah = keycode
		cmp	ah, 0B3h
		jnz	locOSDNotRightKey
		mov	eax, [dwOSDCaretPos]
		cmp	eax, 41
		ja	locOSDWaitForEvent
		inc	eax
		mov	[dwOSDCaretPos], eax
		call	subDrawOpenSaveDlgControls
		jmp	locOSDWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locOSDNotRightKey:			; CODE XREF: seg000:000005B9j
		cmp	ah, 0B0h
		jnz	locOSDNotLeftKey
		mov	eax, [dwOSDCaretPos]
		test	eax, eax
		jz	locOSDWaitForEvent
		dec	eax
		mov	[dwOSDCaretPos], eax
		call	subDrawOpenSaveDlgControls
		jmp	locOSDWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locOSDNotLeftKey:			; CODE XREF: seg000:000005D5j
		cmp	ah, 0B6h
		jnz	locOSDNotDeleteKey
		call	subOSDDeleteChar
		call	subDrawOpenSaveDlgControls
		jmp	locOSDWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locOSDNotDeleteKey:			; CODE XREF: seg000:000005F0j
		cmp	ah, 8
		jnz	locOSDNotBackspaceKey
		mov	eax, [dwOSDCaretPos]
		test	eax, eax
		jz	locOSDWaitForEvent
		dec	eax
		mov	[dwOSDCaretPos], eax
		call	subOSDDeleteChar
		call	subDrawOpenSaveDlgControls
		jmp	locOSDWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locOSDNotBackspaceKey:			; CODE XREF: seg000:00000601j
		cmp	ah, 0Dh
		jnz	locOSDNotReturnKey

locOSDReturnKey:			; CODE XREF: seg000:000006E1j
		mov	al, ' '
		mov	edi, [dwBufferPtr]
		add	edi,43
		mov	ecx,43
		std
		repe scasb
		cld
		inc	edi
		mov	byte[edi+1], 0
		cmp	[dwBufferPtr], aPatternPath ; "/rd/1/pattern.mgb                             "...
		jnz	locCloseOSD
		cmp	[dwEditLabel], aOpenFile	; "Open file"
		jnz	locSaveFile
		mov	[stFileInfoBlock], 0
		mov	[dwDataSize], TESTS_NUM*4+44
		bts	word[wFlags], 2
		mov	eax,70 ; 58
		mov	ebx,stFileInfoBlock
		mcall
		mov	esi,APP_MEM_END+100
		mov	edi,results_table+4
		cld
	    @@: cmp	dword[edi+TEST_REC_SIZE-8],0
		je	@f
		movsd
		add	edi,TEST_REC_SIZE-4
		jmp	@b
	    @@:
		mov	edi,aComment2
		mov	ecx,44
		rep	movsb
		jmp	locCloseOSD
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locSaveFile:				; CODE XREF: seg000:00000654j
		mov	[stFileInfoBlock], 2 ; 1
		mov	[dwDataSize], TESTS_NUM*4+44
		bts	word[wFlags], 2
		mov	esi,results_table+4
		mov	edi,APP_MEM_END+100
		cld
	    @@: cmp	dword[esi+TEST_REC_SIZE-8],0
		je	@f
		movsd
		add	esi,TEST_REC_SIZE-4
		jmp	@b
	    @@: mov	esi,aComment2
		mov	ecx,44
		rep	movsb
		mov	eax,70 ; 58
		mov	ebx,stFileInfoBlock
		mcall
		jmp	locCloseOSD
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locOSDNotReturnKey:			; CODE XREF: seg000:00000624j
		cmp	[dwOSDCaretPos], 42
		jnb	locOSDWaitForEvent
		mov	edi, [dwBufferPtr]
		add	edi, 42
		mov	esi, edi
		dec	esi
		mov	ecx, 42
		sub	ecx, [dwOSDCaretPos]
		std
		rep movsb
		shr	eax, 8
		mov	esi, [dwBufferPtr]
		add	esi, [dwOSDCaretPos]
		mov	[esi], al
		inc	[dwOSDCaretPos]
		call	subDrawOpenSaveDlgControls
		jmp	locOSDWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locOSDButtonEvent:			; CODE XREF: seg000:000005A7j
		mov	eax, 17
		mcall		 ; Kolibri - GET PRESSED BUTTON
					; Return: ah = button ID
		cmp	ah, 1
		jnz	locNotCloseOSD
		jmp	locOSDReturnKey
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locCloseOSD:				; CODE XREF: seg000:00000644j
					; seg000:0000066Cj ...
		bts	[wFlags], 1
		btr	[wFlags], 0
		mov	eax, -1
		mcall		 ; Kolibri - FINISH EXECUTION

locNotCloseOSD: 			; CODE XREF: seg000:000006DFj
		cmp	ah, 2
		jnz	locNotSetCaretOSD
		mov	eax, 37
		mov	ebx, 1
		mcall		 ; Kolibri - GET MOUSE COORDINATES, WINDOW-RELATIVE
					; Return: eax = [x]*65536 + [y]
		shr	eax, 16
		sub	eax, 21
		xor	edx, edx
		mov	ebx, 6
		div	ebx
		mov	[dwOSDCaretPos], eax
		call	subDrawOpenSaveDlgControls
		jmp	locOSDWaitForEvent
; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

locNotSetCaretOSD:			; CODE XREF: seg000:00000700j
		jmp	locOSDWaitForEvent

; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subOSDDeleteChar	; CODE XREF: seg000:000005F2p
					; seg000:00000612p
		mov	edi, [dwBufferPtr]
		add	edi, [dwOSDCaretPos]
		mov	esi, edi
		inc	esi
		mov	ecx, 43
		sub	ecx, [dwOSDCaretPos]
		cld
		rep movsb
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subDrawOpenSaveDlg	; CODE XREF: seg000:subOpenSaveDlgProcp
		mov	eax, 12
		mov	ebx, 1
		mcall		 ; Kolibri - BEGIN WINDOW REDRAW
		xor	eax, eax
		mov	ebx, 64012Ch
		mov	ecx, 640050h
		mov	edx, 3780078h
		mcall		 ; Kolibri - DEFINE/DRAW WINDOW
					; ebx = [xstart]*65536+[xsize]
					; ecx = [ystart]*65536+[ysize]
		mov	eax, 4
		mov	ebx, 80008h
		mov	ecx, 10DDEEFFh
		mov	edx, [dwEditLabel]
		mov	esi, [dwEditLabelLen]
		mcall		 ; Kolibri - DRAW STRING
					; ebx = [xstart]*65536+[ystart]
					; ecx = 0xX0RRGGBB, edx -> string
		call	subDrawOpenSaveDlgControls
		mov	eax, 12
		mov	ebx, 2
		mcall		 ; Kolibri - END WINDOW REDRAW
		retn
endp


; ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ S U B R O U T I N E ÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛÛ


proc		subDrawOpenSaveDlgControls	; CODE XREF: seg000:000005CBp
					; seg000:000005E6p ...
		pusha
		mov	eax, 8
		mov	ebx, 150102h
		mov	ecx, 28000Fh
		mov	edx, 40000002h
		mcall		 ; Kolibri - DEFINE/DELETE BUTTON
					; ebx = [xstart]*65536+[xsize]
					; ecx = [ystart]*65536+[ysize]
					; edx = 0xXYnnnnnn, esi = color
		mov	eax, 13
		mov	edx, 0E0E0E0h
		mcall		 ; Kolibri - DRAW RECTANGLE
					; ebx = [xstart]*65536+[xsize], ecx = [ystart]*65536+[ysize]
					; edx = 0xRRGGBB or 0x80RRGGBB for gradient
		push	eax
		mov	eax, 60000h
		mul	[dwOSDCaretPos]
		add	eax, 150006h
		mov	ebx, eax
		pop	eax
		mov	edx, 6A73D0h
		mcall		 ; Kolibri -
		mov	eax, 4
		mov	ebx, 15002Ch
		xor	ecx, ecx
		mov	edx, [dwBufferPtr]
		mov	esi, 43
		mcall		 ; Kolibri - DRAW STRING
					; ebx = [xstart]*65536+[ystart]
					; ecx = 0xX0RRGGBB, edx -> string
		popa
		retn
endp

; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

dwMainWndSlot	dd 0			; DATA XREF: start:loc_46w start+7Dr
dwBufferPtr	dd 0			; DATA XREF: start+13Cw start+18Bw ...
dwEditLabel	dd 0			; DATA XREF: start+146w start+195w ...
dwEditLabelLen	dd 0			; DATA XREF: start+150w start+19Fw ...
aComment	db 'Comment'		; DATA XREF: start+146o
aOpenFile	db 'Open file'		; DATA XREF: start+195o
					; seg000:0000064Ao
aSaveAs 	db 'Save as...' 	; DATA XREF: start+1D4o
dwOSDCaretPos	dd 0			; DATA XREF: seg000:000005BBr
					; seg000:000005C6w ...
macro unused {
stFileInfoBlock dd 0			; DATA XREF: start+5Co
					; seg000:00000658w ...
		dd 0
dwDataSize	dd 1			; DATA XREF: seg000:0000065Ew
					; seg000:00000678w
		dd APP_MEM_END+100
		dd APP_MEM_END
aPatternPath	db '/hd0/1/pattern.mgb                           ' ; DATA XREF: start+18Bo
}
align 4
stFileInfoBlock dd 0,0,0
dwDataSize	dd 1
		dd APP_MEM_END+100
aPatternPath	db '/hd0/1/pattern.mgb                           ' ; DATA XREF: start+18Bo

					; start+1CAo ...
wFlags		dd 0			; DATA XREF: start:loc_70w
					; start:loc_87w ...
dwTestEndTime	dd 0			; DATA XREF: subInitTestTimer+14w
					; subIfTimeElapsed+Dr
results_table dd \
  ?,?,testDrawWindow,aDrawingWindow,\
  ?,?,testDrawBar,aDrawingBar,\
  ?,?,testDrawPicture,aDrawingPicture,\
  ?,?,testDrawVertLine,aDrawingVLine,\
  ?,?,testDrawHorzLine,aDrawingHLine,\
  ?,?,testDrawFreeLine,aDrawingFLine,\
  ?,?,testDrawText1,aDrawingText1,\
  ?,?,testDrawText2,aDrawingText2,\
  ?,?,testDrawNumber,aDrawingNumber,\
  ?,?,testDrawPixel,aDrawingPixel,\
  0,0,0,0

LINE_HEIGHT   = 13
TEST_REC_SIZE = 16
TESTS_NUM     = ($ - results_table) / TEST_REC_SIZE - 1

macro cstr name,str {
  local ..end
  name db ..end-name-1,str
  ..end:
}

aDrawingWindow	db 'Window Of Type #3, 325x400 px',0
aDrawingBar	db 'Filled Rectangle, 100x250 px',0
aDrawingPicture db 'Picture, 55x123, px',0
aDrawingVLine	db 'Vertical Line, 350 px',0
aDrawingHLine	db 'Horizontal Line, 270 px',0
aDrawingFLine	db 'Free-angled Line, 350 px',0
aDrawingText1	db 'Fixed-width Text, 34 chars',0
aDrawingText2	db 'Proportional Text, 34 chars',0
aDrawingNumber	db 'Decimal Number, 8 digits',0
aDrawingPixel	db 'Single Pixel',0

aTestText	db 'This is a 34-charachters test text' ; DATA XREF: seg000:000002ADo
					; seg000:000002DAo
aButtonsText	db 'Test      Comment+    Pattern+      Open        Save',0
					; DATA XREF: subDrawMainWindow+5Do
aCaption	db 'Menuet Graphical Benchmark 0.3',0 ; DATA XREF: subDrawMainWindow+4Co

aLeft		db 'Left    :',0
aRight		db 'Right   :',0

aComment1	db 'current                                     ' ; DATA XREF: start+13Co
					; subDrawBars+9Co
;dwDrawWindowTime2 dd 0                  ; DATA XREF: subSubSavePattern+5o
;                                        ; subDrawBars+18o ...
;                dd 0
;                dd 0
;                dd 0
;                dd 0
;                dd 0
;                dd 0
aComment2	db 'no pattern                                  '
					; DATA XREF: subDrawBars+72o
					; subDrawBars+B9o
APP_MEM_END:	rb   30 ;               ; DATA XREF: seg000:off_10o
					; subGetThreadInfo+5o ...
dwMainPID	dd ?			; DATA XREF: start+Ar start:loc_3Br
