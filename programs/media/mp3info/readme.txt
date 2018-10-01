;******************************************************************************
; project name:    SuperMP3                                                     
; target platform: MenuetOS, x86 (IA-32), x86-64 achitectures           
; compiler:        flat assembler 1.64                                        
; version:         0.65
; last update:     5th September 2005                                          
; maintained by:   Sergey Kuzmin aka Wildwest                                 
; e-mail:          kuzmin_serg@list.ru                                        
;******************************************************************************
; Summary:                                                                    
; initial reader for mp3's headers
; can read mostly needed parts from headers of almost all mp3 files
; can be used in tag editor, mp3 player or converter later
; License:  GPL & LGPL
;******************************************************************************

;--------------------------------------------------------------------------------------
;HISTORY: 

;0.7:       MP3INFO                                            23/09/2018
            Sergey Efremenkov aka theonlymirage
            adopted app to use system f70 instead of old f58
            Kiril Lipatov aka Leency make app to open with param, small UI update

;0.65:      SuperMP3
            Madis Kalme rewrited extract_bits (he used his Extracteax macro) 
            and decode_bitrate functions

;0.64:      Xing header reading: correct time, bitrate and number of
            frames for VBR files	
;0.62:      ID3v2 detecting and writing its version
;0.61:      Header search 
;           Added 'Header found at' field 
;           If file has no frames, error message is shown 
;           Russian tag support (in windows-1251 encoding)
;           
;0.6:       SuperMP3 (not finished and its future in the dark)   19/08/2005 
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Additions: Alexei Ershov aka ealex <e-al@yandex.ru>
;Features:  added checking of CRC bit;
;           improved frame_size calculation (thanks to Alexei Ershov for the formula)
;           added ID3v1 tag reading (except Genre field) by Alexei Ershov
;--------------------------------------------------------------------------------------
;0.5:       SuperMP3                                             17/08/2005 
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Additions: Alexei Ershov aka ealex <e-al@yandex.ru>
;Features:  added checking of Padding bit;
;           Added open dialog and optimized macroses Text, Number, DrawLine by Alexei Ershov 
;--------------------------------------------------------------------------------------
;0.4:       SuperMP3                                             05/08/2005 
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>
;Additions: Alexei Ershov aka ealex <e-al@yandex.ru>
;Features:  added Frame_size and Quantity_of_Frames;
;           optimized decode_samplerate and decode_bitrate routines by Alexei Ershov 
;--------------------------------------------------------------------------------------
;0.3:       SuperMP3                                             25/04/2005            
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>                                    
;Features:  added File_size and Duration (rough estimation), improved SampleRate, fixed BitRate
;--------------------------------------------------------------------------------------      
;0.2:       SuperMP3                                             21/04/2005            
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>                                    
;Features:  added SampleRate and BitRate
;--------------------------------------------------------------------------------------
;0.1:       SuperMP3                                             20/04/2005            
;Author:    Sergey Kuzmin aka Wildwest <kuzmin_serg@list.ru>                                    
;Features:  able to detect MPEG Version, Layer, Channels
;----------------------------------------------------------------------------------------  
;Issues:   
; MP3 player needs work of several men during several months(approx. 10 men and 2-4 months, because it is ASM OS). 
; If you want to listen a lot of mp3 in MenuetOS - make a bit of asm!         
;----------------------------------------------------------------------------- 
;CLEAN mp3 - is mp3 file without any info before header. Open mp3 in HEX EDITOR and check it out. 

;Usually normal header's start looks like FFF3 or FFFB.

;If you see TAG (4944 3303 = ID3.) or RIFF (5249 4646 = RIFF) 
;or (0000 0000 - encoded by Lame codec or another shitty program) - it is bad, 
;you must delete this piece until FFFx before testing. Happy testing!

;Note - I test it only on several files (test.mp3
;included in this package) in Bochs and Qemu.
;For testing change name if the end of "supermp3.asm" - section
 fileinfo:
  dd 0, 0, 1, mp3_file, 0x1000
  db "/RD/1/TEST.MP3",0 

Package content:
readme.txt - this file
supermp3 - compiled exacutable file
supermp3.asm - main file - build program's GUI
MACROS.INC  - standard file with macroses
MOS_UZIT.INC - macroses for interface, routines for extracting bits and decoding extracted bits
ASCL.INC - macroses for interface and useful stuff
test.mp3 - test file

Any comments by e-mail or on forums (http://meos.sysbin.com, http://forum.meos.ru, http://menuetos.fastbb.ru, 
http://menuet.2.forumer.com, http://board.flatassembler.net/forum.php?f=12) are appreciated.