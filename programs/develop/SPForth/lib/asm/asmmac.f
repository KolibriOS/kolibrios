( Miscellaneous macros for Win32FORTH 486ASM version 1.24 )
( copyright [c] 1994 by Jim Schneider )
( This file version 1.2 )

(    This program is free software; you can redistribute it and/or modify )
(    it under the terms of the GNU General Public License as published by )
(    the Free Software Foundation; either version 2 of the License, or    )
(    <at your option> any later version.                                  )
(                                                                         )
(    This program is distributed in the hope that it will be useful,      )
(    but WITHOUT ANY WARRANTY; without even the implied warranty of       )
(    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        )
(    GNU General Public License for more details.                         )
(                                                                         )
(    You should have received a copy of the GNU General Public License    )
(    along with this program; if not, write to the Free Software          )
(    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            )

MACRO: ;M POSTPONE ;MACRO ENDM IMMEDIATE
MACRO: AL, AL , ;M
MACRO: CL, CL , ;M
MACRO: DL, DL , ;M
MACRO: BL, BL , ;M
MACRO: AH, AH , ;M
MACRO: CH, CH , ;M
MACRO: DH, DH , ;M
MACRO: BH, BH , ;M
MACRO: AX, AX , ;M
MACRO: CX, CX , ;M
MACRO: DX, DX , ;M
MACRO: BX, BX , ;M
MACRO: SP, SP , ;M
MACRO: BP, BP , ;M
MACRO: SI, SI , ;M
MACRO: DI, DI , ;M
MACRO: EAX, EAX , ;M
MACRO: ECX, ECX , ;M
MACRO: EDX, EDX , ;M
MACRO: EBX, EBX , ;M
MACRO: ESP, ESP , ;M
MACRO: EBP, EBP , ;M
MACRO: ESI, ESI , ;M
MACRO: EDI, EDI , ;M
MACRO: [BX+SI], [BX+SI] , ;M
MACRO: [BX+DI], [BX+DI] , ;M
MACRO: [BP+SI], [BP+SI] , ;M
MACRO: [BP+DI], [BP+DI] , ;M
MACRO: [SI], [SI] , ;M
MACRO: [DI], [DI] , ;M
MACRO: [BP], [BP] , ;M
MACRO: [BX], [BX] , ;M
MACRO: [EAX], [EAX] , ;M
MACRO: [ECX], [ECX] , ;M
MACRO: [EDX], [EDX] , ;M
MACRO: [EBX], [EBX] , ;M
MACRO: [ESP], [ESP] , ;M
MACRO: [EBP], [EBP] , ;M
MACRO: [ESI], [ESI] , ;M
MACRO: [EDI], [EDI] , ;M
MACRO: [EAX*2], [EAX*2] , ;M
MACRO: [ECX*2], [ECX*2] , ;M
MACRO: [EDX*2], [EDX*2] , ;M
MACRO: [EBX*2], [EBX*2] , ;M
\ MACRO: [ESP*2], [ESP*2] , ;M
MACRO: [EBP*2], [EBP*2] , ;M
MACRO: [ESI*2], [ESI*2] , ;M
MACRO: [EDI*2], [EDI*2] , ;M
MACRO: [EAX*4], [EAX*4] , ;M
MACRO: [ECX*4], [ECX*4] , ;M
MACRO: [EDX*4], [EDX*4] , ;M
MACRO: [EBX*4], [EBX*4] , ;M
MACRO: [EBP*4], [EBP*4] , ;M
MACRO: [ESI*4], [ESI*4] , ;M
MACRO: [EDI*4], [EDI*4] , ;M
MACRO: [EAX*8], [EAX*8] , ;M
MACRO: [ECX*8], [ECX*8] , ;M
MACRO: [EDX*8], [EDX*8] , ;M
MACRO: [EBX*8], [EBX*8] , ;M
MACRO: [EBP*8], [EBP*8] , ;M
MACRO: [ESI*8], [ESI*8] , ;M
MACRO: [EDI*8], [EDI*8] , ;M
MACRO: ES, ES , ;M
MACRO: CS, CS , ;M
MACRO: SS, SS , ;M
MACRO: DS, DS , ;M
MACRO: FS, FS , ;M
MACRO: GS, GS , ;M
MACRO: CR0, CR0 , ;M
MACRO: CR2, CR2 , ;M
MACRO: CR3, CR3 , ;M
MACRO: CR4, CR4 , ;M
MACRO: DR0, DR0 , ;M
MACRO: DR1, DR1 , ;M
MACRO: DR2, DR2 , ;M
MACRO: DR3, DR3 , ;M
MACRO: DR6, DR6 , ;M
MACRO: DR7, DR7 , ;M
MACRO: TR3, TR3 , ;M
MACRO: TR4, TR4 , ;M
MACRO: TR5, TR5 , ;M
MACRO: TR6, TR6 , ;M
MACRO: TR7, TR7 , ;M
MACRO: ST, ST , ;M
MACRO: ST(0), ST(0) , ;M
MACRO: ST(1), ST(1) , ;M
MACRO: ST(2), ST(2) , ;M
MACRO: ST(3), ST(3) , ;M
MACRO: ST(4), ST(4) , ;M
MACRO: ST(5), ST(5) , ;M
MACRO: ST(6), ST(6) , ;M
MACRO: ST(7), ST(7) , ;M
