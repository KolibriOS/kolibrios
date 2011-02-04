; MASM.ASM
; --------
; uFMOD public source code release. Provided as-is.

; *** This stub allows compiling uFMOD sources using MASM32.

.386
.model flat

ifdef f44100
	FSOUND_MixRate = 44100
	FREQ_40HZ_p    = 1DB8Bh
	FREQ_40HZ_f    = 3B7160h
	PCM_format     = 3
else
	ifdef f22050
		FSOUND_MixRate = 22050
		FREQ_40HZ_p    = 3B716h
		FREQ_40HZ_f    = 76E2C0h
		PCM_format     = 9
	else
		ifndef f48000
			echo UF_FREQ not specified (defaulting to 48KHz)
		endif
		FSOUND_MixRate = 48000
		FREQ_40HZ_p    = 1B4E8h
		FREQ_40HZ_f    = 369D00h
		PCM_format     = 1
	endif
endif

RAMP_NONE   = 0
RAMP_WEAK   = 0
RAMP_STRONG = 0
ifdef NONE
	RAMP_NONE = 1
else
	ifdef WEAK
		RAMP_WEAK = 1
	else
		ifndef STRONG
			echo UF_RAMP not specified (defaulting to STRONG)
		endif
		RAMP_STRONG = 1
	endif
endif

UCODE = 0
DEBUG = 0

CHK4VALIDITY = 1
ifdef UNSAFE
	echo WARNING! Unsafe mod is ON. Library may crash while loading damaged XM tracks!
	CHK4VALIDITY = 0
endif

AC97SND_ON = 0
ifdef AC97SND
	AC97SND_ON = 1
endif

include eff.inc

FSOUND_SAMPLE STRUC
	_length   dd ?
	loopstart dd ?
	looplen   dd ?
	defvol    db ?
	finetune  db ?
	bytes     db ?
	defpan    db ?
	relative  db ?
	Resved    db ?
	loopmode  db ?
	_align    db ?
	buff      db ?,?
FSOUND_SAMPLE ENDS

FSOUND_CHANNEL STRUC
	actualvolume     dd ?
	actualpan        dd ?
	fsampleoffset    dd ?
	leftvolume       dd ?
	rightvolume      dd ?
	mixpos           dd ?
	speedlo          dd ?
	speedhi          dd ?
	ramp_lefttarget  dw ?
	ramp_righttarget dw ?
	ramp_leftspeed   dd ?
	ramp_rightspeed  dd ?
	fsptr            dd ?
	mixposlo         dd ?
	ramp_leftvolume  dd ?
	ramp_rightvolume dd ?
	ramp_count       dw ?
	speeddir         db ?,?
FSOUND_CHANNEL ENDS

FMUSIC_NOTE STRUC
	unote   db ?
	number  db ?
	uvolume db ?
	effect  db ?
	eparam  db ?
FMUSIC_NOTE ENDS

FMUSIC_PATTERN STRUC
	rows        dw ?
	patternsize dw ?
	data        dd ?
FMUSIC_PATTERN ENDS

FMUSIC_INSTRUMENT STRUC
	sample       dd 16 dup (?)
	keymap       db 96 dup (?)
	VOLPoints    dw 24 dup (?)
	PANPoints    dw 24 dup (?)
	VOLnumpoints db ?
	PANnumpoints db ?
	VOLsustain   db ?
	VOLLoopStart db ?
	VOLLoopEnd   db ?
	PANsustain   db ?
	PANLoopStart db ?
	PANLoopEnd   db ?
	VOLtype      db ?
	PANtype      db ?
	VIBtype      db ?
	VIBsweep     db ?
	iVIBdepth    db ?
	VIBrate      db ?
	VOLfade      dw ?
FMUSIC_INSTRUMENT ENDS

FMUSIC_CHANNEL STRUC
	note          db ?
	samp          db ?
	notectrl      db ?
	inst          db ?
	cptr          dd ?
	freq          dd ?
	volume        dd ?
	voldelta      dd ?
	freqdelta     dd ?
	pan           dd ?
	envvoltick    dd ?
	envvolpos     dd ?
	envvoldelta   dd ?
	envpantick    dd ?
	envpanpos     dd ?
	envpandelta   dd ?
	ivibsweeppos  dd ?
	ivibpos       dd ?
	keyoff        db ?,?
	envvolstopped db ?
	envpanstopped db ?
	envvolfrac    dd ?
	envvol        dd ?
	fadeoutvol    dd ?
	envpanfrac    dd ?
	envpan        dd ?
	period        dd ?
	sampleoffset  dd ?
	portatarget   dd ?
	patloopno     db ?,?,?,?
	patlooprow    dd ?
	realnote      db ?
	recenteffect  db ?
	portaupdown   db ?
	              db ?
	xtraportadown db ?
	xtraportaup   db ?
	volslide      db ?
	panslide      db ?
	retrigx       db ?
	retrigy       db ?
	portaspeed    db ?
	vibpos        db ?
	vibspeed      db ?
	vibdepth      db ?
	tremolopos    db ?
	tremolospeed  db ?
	tremolodepth  db ?
	tremorpos     db ?
	tremoron      db ?
	tremoroff     db ?
	wavecontrol   db ?
	finevslup     db ?
	fineportaup   db ?
	fineportadown db ?
FMUSIC_CHANNEL ENDS

FMUSIC_MODULE STRUC
	pattern              dd ?
	instrument           dd ?
	mixer_samplesleft    dd ?
	globalvolume         dd ?
	tick                 dd ?
	speed                dd ?
	order                dd ?
	row                  dd ?
	patterndelay         dd ?
	nextorder            dd ?
	nextrow              dd ?
	unused1              dd ?
	numchannels          dd ?
	Channels             dd ?
	uFMOD_Ch             dd ?
	mixer_samplespertick dd ?
	numorders            dw ?
	restart              dw ?
	numchannels_xm       db ?
	globalvsl            db ?
	numpatternsmem       dw ?
	numinsts             dw ?
	flags                dw ?
	defaultspeed         dw ?
	defaultbpm           dw ?
	orderlist            db 256 dup (?)
FMUSIC_MODULE ENDS

FMUSIC_MODULE_size     = SIZE FMUSIC_MODULE
FSOUND_CHANNEL_size    = SIZE FSOUND_CHANNEL
FMUSIC_CHANNEL_size    = SIZE FMUSIC_CHANNEL
FMUSIC_INSTRUMENT_size = SIZE FMUSIC_INSTRUMENT
FMUSIC_PATTERN_size    = SIZE FMUSIC_PATTERN
FMUSIC_NOTE_size       = SIZE FMUSIC_NOTE

; FPU register stack
st0 TEXTEQU <st(0)>
st1 TEXTEQU <st(1)>

.CODE
include ufmod.asm
include core.asm

.DATA?
_mod          = $         
              FMUSIC_MODULE<>
mmt           dd ?,?,?
ufmod_heap    dd ?,?
if AC97SND_ON
	EXTERN hSound:DWORD
	      dd ?
else
	hSound dd ?
endif
hBuff         dd ?
SW_Exit       dd ?
MixBuf        db FSOUND_BlockSize*8 dup (?)
ufmod_noloop  db ?
ufmod_pause_  db ?
mix_endflag   db ?,?
mmf           dd ?,?,?,?
ufmod_vol     dd ?
uFMOD_fopen   dd ?
uFMOD_fread   dd ?
file_struct   dd 7 dup (?)
cache_offset  dd ?
if INFO_API_ON
	time_ms dd ?
	L_vol   dw ?
	R_vol   dw ?
	s_row   dw ?
	s_order dw ?
	szTtl   db 24 dup (?)
endif
DummySamp FSOUND_SAMPLE<>
end
