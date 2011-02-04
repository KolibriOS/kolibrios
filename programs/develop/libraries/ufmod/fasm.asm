; FASM.ASM
; --------
; uFMOD public source code release. Provided as-is.

; *** This stub allows compiling uFMOD sources using FASM.

; *** CONSTANTS ***

if UF_FREQ eq 44100
	FSOUND_MixRate = 44100
	FREQ_40HZ_p    = 1DB8Bh
	FREQ_40HZ_f    = 3B7160h
	PCM_format     = 3 ; PCM_2_16_44
else
	if UF_FREQ eq 22050
		FSOUND_MixRate = 22050
		FREQ_40HZ_p    = 3B716h
		FREQ_40HZ_f    = 76E2C0h
		PCM_format     = 9 ; PCM_2_16_22
	else
		if UF_FREQ eq 48000
		else
			display 'UF_FREQ not specified (defaulting to 48KHz)',13,10
		end if
		FSOUND_MixRate = 48000
		FREQ_40HZ_p    = 1B4E8h
		FREQ_40HZ_f    = 369D00h
		PCM_format     = 1 ; PCM_2_16_48
	end if
end if

if UF_RAMP eq NONE
	RAMP_NONE   = 1
	RAMP_WEAK   = 0
	RAMP_STRONG = 0
else
	if UF_RAMP eq WEAK
		RAMP_NONE   = 0
		RAMP_WEAK   = 1
		RAMP_STRONG = 0
	else
		if UF_RAMP eq STRONG
		else
			display 'UF_RAMP not specified (defaulting to STRONG)',13,10
		end if
		RAMP_NONE   = 0
		RAMP_WEAK   = 0
		RAMP_STRONG = 1
	end if
end if

UCODE equ 0

if UF_MODE eq UNSAFE
	display 'WARNING! Unsafe mod is ON. Library may crash while loading damaged XM tracks!',13,10
	CHK4VALIDITY = 0
	AC97SND_ON   = 0
else
	CHK4VALIDITY = 1
	if UF_MODE eq AC97SND
		AC97SND_ON = 1
	else
		AC97SND_ON = 0
	end if
end if

if NOLINKER
else
	format MS COFF
	section '.text' code readable executable
end if

; *** STRUCTS ***

; Sample type - contains info on sample
struc FSOUND_SAMPLE{

	; Don't change order .:.
	._length   dd ? ; sample length
	.loopstart dd ? ; loop start
	.looplen   dd ? ; loop length
	.defvol    db ? ; default volume
	.finetune  db ? ; finetune value from -128 to 127
	.bytes     db ? ; type [b 0-1] : 0 - no loop
	                ;                1 - forward loop
	                ;                2 - bidirectional loop (aka ping-pong)
	                ;      [b 4]   : 0 - 8-bit sample data
	                ;                1 - 16-bit sample data
	.defpan    db ? ; default pan value from 0 to 255
	.relative  db ? ; relative note (signed value)
	.Resved    db ? ; reserved, known values: 00h - regular delta packed sample data
	                ;                         ADh - ModPlug 4-bit ADPCM packed sample data
	; .:.

	.loopmode  db ?
	._align    db ?
	.buff      db ?,? ; sound data
}
virtual at 0
FSOUND_SAMPLE FSOUND_SAMPLE
FSOUND_SAMPLE_size = $-FSOUND_SAMPLE
end virtual


; Channel type - contains information on a mixing channel
struc FSOUND_CHANNEL{
	.actualvolume     dd ? ; driver level current volume
	.actualpan        dd ? ; driver level panning value
	.fsampleoffset    dd ? ; sample offset (sample starts playing from here)
	.leftvolume       dd ? ; mixing information. adjusted volume for left channel (panning involved)
	.rightvolume      dd ? ; mixing information. adjusted volume for right channel (panning involved)
	.mixpos           dd ? ; mixing information. high part of 32:32 fractional position in sample
	.speedlo          dd ? ; mixing information. playback rate - low part fractional
	.speedhi          dd ? ; mixing information. playback rate - high part fractional
	.ramp_lefttarget  dw ?
	.ramp_righttarget dw ?
	.ramp_leftspeed   dd ?
	.ramp_rightspeed  dd ?

	; Don't change order .:.
	.fsptr            dd ? ; pointer to FSOUND_SAMPLE currently playing sample
	.mixposlo         dd ? ; mixing information. low part of 32:32 fractional position in sample
	.ramp_leftvolume  dd ?
	.ramp_rightvolume dd ?
	.ramp_count       dw ?
	.speeddir         db ?,? ; mixing information. playback direction - forwards or backwards
	; .:.

}
virtual at 0
FSOUND_CHANNEL FSOUND_CHANNEL
FSOUND_CHANNEL_size = $-FSOUND_CHANNEL
end virtual

; Single note type - contains info on 1 note in a pattern
struc FMUSIC_NOTE{
	.unote   db ? ; note to play at (0-97) 97=keyoff
	.number  db ? ; sample being played (0-128)
	.uvolume db ? ; volume column value (0-64)  255=no volume
	.effect  db ? ; effect number (0-1Ah)
	.eparam  db ? ; effect parameter (0-255)
}
virtual at 0
FMUSIC_NOTE FMUSIC_NOTE
FMUSIC_NOTE_size = $-FMUSIC_NOTE
end virtual

; Pattern data type
struc FMUSIC_PATTERN{
	.rows        dw ?
	.patternsize dw ?
	.data        dd ? ; pointer to FMUSIC_NOTE
}
virtual at 0
FMUSIC_PATTERN FMUSIC_PATTERN
FMUSIC_PATTERN_size = $-FMUSIC_PATTERN
end virtual

; Multi sample extended instrument
struc FMUSIC_INSTRUMENT{
	.sample       rd 16 ; 16 pointers to FSOUND_SAMPLE per instrument

	; Don't change order .:.
	.keymap       rb 96 ; sample keymap assignments
	.VOLPoints    rw 24 ; volume envelope points
	.PANPoints    rw 24 ; panning envelope points
	.VOLnumpoints db ? ; number of volume envelope points
	.PANnumpoints db ? ; number of panning envelope points
	.VOLsustain   db ? ; volume sustain point
	.VOLLoopStart db ? ; volume envelope loop start
	.VOLLoopEnd   db ? ; volume envelope loop end
	.PANsustain   db ? ; panning sustain point
	.PANLoopStart db ? ; panning envelope loop start
	.PANLoopEnd   db ? ; panning envelope loop end
	.VOLtype      db ? ; type of envelope,bit 0:On 1:Sustain 2:Loop
	.PANtype      db ? ; type of envelope,bit 0:On 1:Sustain 2:Loop
	.VIBtype      db ? ; instrument vibrato type
	.VIBsweep     db ? ; time it takes for vibrato to fully kick in
	.iVIBdepth    db ? ; depth of vibrato
	.VIBrate      db ? ; rate of vibrato
	.VOLfade      dw ? ; fade out value
	; .:.

}
virtual at 0
FMUSIC_INSTRUMENT FMUSIC_INSTRUMENT
FMUSIC_INSTRUMENT_size = $-FMUSIC_INSTRUMENT
end virtual

; Channel type - contains information on a mod channel
struc FMUSIC_CHANNEL{
	.note          db ? ; last note set in channel
	.samp          db ? ; last sample set in channel
	.notectrl      db ? ; flags for DoFlags proc
	.inst          db ? ; last instrument set in channel
	.cptr          dd ? ; pointer to FSOUND_CHANNEL system mixing channel
	.freq          dd ? ; current mod frequency period for this channel
	.volume        dd ? ; current mod volume for this channel
	.voldelta      dd ? ; delta for volume commands... tremolo/tremor, etc
	.freqdelta     dd ? ; delta for frequency commands... vibrato/arpeggio, etc
	.pan           dd ? ; current mod pan for this channel

	; Don't change order .:.
	.envvoltick    dd ? ; tick counter for envelope position
	.envvolpos     dd ? ; envelope position
	.envvoldelta   dd ? ; delta step between points
	.envpantick    dd ? ; tick counter for envelope position
	.envpanpos     dd ? ; envelope position
	.envpandelta   dd ? ; delta step between points
	.ivibsweeppos  dd ? ; instrument vibrato sweep position
	.ivibpos       dd ? ; instrument vibrato position
	.keyoff        db ?,? ; flag whether keyoff has been hit or not
	.envvolstopped db ? ; flag to say whether envelope has finished or not
	.envpanstopped db ? ; flag to say whether envelope has finished or not
	; .:.

	.envvolfrac    dd ? ; fractional interpolated envelope volume
	.envvol        dd ? ; final interpolated envelope volume
	.fadeoutvol    dd ? ; volume fade out
	.envpanfrac    dd ? ; fractional interpolated envelope pan
	.envpan        dd ? ; final interpolated envelope pan
	.period        dd ? ; last period set in channel
	.sampleoffset  dd ? ; sample offset for this channel in SAMPLES
	.portatarget   dd ? ; note to porta to
	.patloopno     db ?,?,?,? ; pattern loop variables for effect E6x
	.patlooprow    dd ?
	.realnote      db ? ; last realnote set in channel
	.recenteffect  db ? ; previous row's effect... used to correct tremolo volume
	.portaupdown   db ? ; last porta up/down value
	               db ? ; unused
	.xtraportadown db ? ; last porta down value
	.xtraportaup   db ? ; last porta up value
	.volslide      db ? ; last volume slide value
	.panslide      db ? ; pan slide parameter
	.retrigx       db ? ; last retrig volume slide used
	.retrigy       db ? ; last retrig tick count used
	.portaspeed    db ? ; porta speed
	.vibpos        db ? ; vibrato position
	.vibspeed      db ? ; vibrato speed
	.vibdepth      db ? ; vibrato depth
	.tremolopos    db ? ; tremolo position
	.tremolospeed  db ? ; tremolo speed
	.tremolodepth  db ? ; tremolo depth
	.tremorpos     db ? ; tremor position
	.tremoron      db ? ; remembered parameters for tremor
	.tremoroff     db ? ; remembered parameters for tremor
	.wavecontrol   db ? ; waveform type for vibrato and tremolo (4bits each)
	.finevslup     db ? ; parameter for fine volume slide down
	.fineportaup   db ? ; parameter for fine porta slide up
	.fineportadown db ? ; parameter for fine porta slide down
}
virtual at 0
FMUSIC_CHANNEL FMUSIC_CHANNEL
FMUSIC_CHANNEL_size = $-FMUSIC_CHANNEL
end virtual

; Song type - contains info on song
struc FMUSIC_MODULE{

	; Don't change order .:.
	.pattern              dd ? ; pointer to FMUSIC_PATTERN array for this song
	.instrument           dd ? ; pointer to FMUSIC_INSTRUMENT array for this song
	.mixer_samplesleft    dd ?
	.globalvolume         dd ? ; global mod volume
	.tick                 dd ? ; current mod tick
	.speed                dd ? ; speed of song in ticks per row
	.order                dd ? ; current song order position
	.row                  dd ? ; current row in pattern
	.patterndelay         dd ? ; pattern delay counter
	.nextorder            dd ? ; current song order position
	.nextrow              dd ? ; current row in pattern
	.unused1              dd ?
	.numchannels          dd ? ; number of channels
	.Channels             dd ? ; channel pool
	.uFMOD_Ch             dd ? ; channel array for this song
	.mixer_samplespertick dd ?
	.numorders            dw ? ; number of orders (song length)
	.restart              dw ? ; restart position
	.numchannels_xm       db ?
	.globalvsl            db ? ; global mod volume
	.numpatternsmem       dw ? ; number of allocated patterns
	.numinsts             dw ? ; number of instruments
	.flags                dw ? ; flags such as linear frequency, format specific quirks, etc
	.defaultspeed         dw ?
	.defaultbpm           dw ?
	.orderlist            rb 256 ; pattern playing order list
	; .:.

}
virtual at 0
FMUSIC_MODULE FMUSIC_MODULE
FMUSIC_MODULE_size = $-FMUSIC_MODULE
end virtual

OFFSET equ
PTR    equ
endif  equ end if

include 'ufmod.asm'
include 'core.asm'

if NOLINKER
	uFMOD_IMG_END: ; End of uFMOD's code. BSS follows.
	align 16
else
	section '.bss' readable writeable align 16
end if

; Don't change order!
_mod          rb FMUSIC_MODULE_size ; currently playing track
mmt           rd 3
ufmod_heap    dd ?
              dd ? ; unused
if AC97SND_ON
	extrn hSound
	      dd ?
else
	hSound dd ?
endif
hBuff         dd ?
SW_Exit       dd ?
; mix buffer memory block (align 16!)
MixBuf        rb FSOUND_BlockSize*8
ufmod_noloop  db ?
ufmod_pause_  db ?
mix_endflag   rb 2
mmf           rd 4
ufmod_vol     dd ? ; global volume scale
; * LPCALLBACKS *
uFMOD_fopen   dd ?
uFMOD_fread   dd ?
file_struct   rd 7
cache_offset  dd ?
if INFO_API_ON
	time_ms dd ?
	L_vol   dw ? ; L channel RMS volume
	R_vol   dw ? ; R channel RMS volume
	s_row   dw ?
	s_order dw ?
	szTtl   rb 24
end if
DummySamp rb FSOUND_SAMPLE_size
