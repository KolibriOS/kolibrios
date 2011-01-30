filename equ 'kerpack.exe'

virtual at 0
file filename:3Ch,4
load pehea dword from 0
file filename:pehea,0F8h+28h*3
load NumberOfSections word from 4+6
load SizeOfOptionalHeader word from 4+14h
if NumberOfSections<>3
error Expected three sections, .text, .bss and .reloc
end if
if SizeOfOptionalHeader<>0E0h
error Nonstandard PE header
end if
load RelocsRVA dword from 4+0A0h
load RelocsSize dword from 4+0A4h
load ImageBase dword from 4+34h
load TextRVA dword from 4+0F8h+0Ch
load TextSize dword from 4+0F8h+8
load TextOffs dword from 4+0F8h+14h
load BSSSize dword from 4+0F8h+28h+10h
load RelocRVA dword from 4+0F8h+28h*2+0Ch
load RelocOffs dword from 4+0F8h+28h*2+14h
if BSSSize
error Second section expected to be .bss
end if
if RelocRVA<>RelocsRVA
error Third section expected to be .reloc
end if
;file 'test.exe':pehea+0F8h,28h
;load physofs dword from 4+14h
;load mem dword from 4+8
;file 'test.exe':physofs+16,4
;load sz dword from $-4
end virtual

file filename:TextOffs,TextSize

while RelocsSize>8
virtual at 0
file filename:RelocOffs,8
load CurRelocPage dword from 0
load CurRelocChunkSize dword from 4
end virtual
RelocsSize=RelocsSize-CurRelocChunkSize
CurRelocChunkSize = CurRelocChunkSize-8
RelocOffs=RelocOffs+8
while CurRelocChunkSize
virtual at 0
file filename:RelocOffs,2
RelocOffs=RelocOffs+2
CurRelocChunkSize=CurRelocChunkSize-2
load s word from 0
end virtual
CurRelocType = s shr 12
RelocItem = CurRelocPage + (s and 0xFFF)
if CurRelocType=0
else if CurRelocType=3
load z dword from RelocItem-TextRVA
store dword z-(TextRVA+ImageBase) at RelocItem-TextRVA
else
error Unexpected relocation type
end if
end while
end while

store dword TextSize at 10h
store dword RelocRVA-TextRVA at 14h
