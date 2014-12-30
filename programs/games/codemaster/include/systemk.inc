;;;;;;;;;;;;;;;;;;;;; SYSTEM ;;;;;;;;;;;;;;;;;;;;;

; menuet interrupt: eax, ebx, ecx, edx

macro mint a, b, c, d {
 IF ~a eq
  . r0=a
 END IF
 IF ~b eq
  . r3=b
 END IF
 IF ~c eq
  . r1=c
 END IF
 IF ~d eq
  . r2=d
 END IF
 db 0CDh, 40h
}

; events

EVENT.REDRAW=1
EVENT.KEY=2
EVENT.BUTTON=3

align
integer timer.speed=-1,\
 timer.counter, event.delay=1

macro set.timer n {
  . r0=n, r1=10, r0/r1
  . timer.speed=r0, timer.counter=r0
}

macro get.timer { mint 26, 9 }

macro delay x {
  . r0=x, r0/100, r3=r0
  mint 5
}

macro wait.event { mint 23, event.delay }

;;;;;;;;;;;;;;;;;;;;; WINDOW ;;;;;;;;;;;;;;;;;;;;;

function draw.window
  . r2=0, r6=0, r7=0FFFFFFh
  . r3=WINDOW.X, r3<<16, r3|screen.w
  . r1=WINDOW.Y, r1<<16, r1|screen.h
  mint 0
endf

macro draw.begin { mint 12, 1 }
macro draw.end	 { mint 12, 2 }
macro render	 { call !on.draw } ; mint 15, 3
macro retrace	 { mint 18, 14 }

;;;;;;;;;;;;;;;;;;;;; INPUT ;;;;;;;;;;;;;;;;;;;;;;

align 4

align
integer event.type

integer key, old.key, key.event
text keys.t(128)

numeric KEY.ESCAPE=27, KEY.SPACE=' ',\
 KEY.UP=0B2h, KEY.RIGHT=0B3h, KEY.DOWN=0B1h,\
 KEY.LEFT=0B0h, KEY.A='a', KEY.S='s',\
 KEY.D='d', KEY.W='w'

macro get.key.buffer { mint 26, 2, 1, keys.t }

integer mouse.event,\
 mouse.1, mouse.2, mouse.x, mouse.y,\
 mouse.drag, mouse.drag.x, mouse.drag.y,\
 mouse.drop, mouse.drop.x, mouse.drop.y,\
 old.mouse.1, old.mouse.2,\
 old.mouse.x, old.mouse.y

macro get.mouse.xy	{ mint 37, 0 }
macro get.mouse.buttons { mint 37, 2 }
macro get.mouse.wheel	{ mint 37, 3 }

function update.mouse
  . mouse.event=0
  . old.mouse.1=mouse.1, old.mouse.2=mouse.2
  . old.mouse.x=mouse.x, old.mouse.y=mouse.y

  get.mouse.xy
  . r1=r0, r1&0FFFFh, r0>>>16, r0&0FFFFh
  . mouse.x=r0, mouse.y=r1
  get.mouse.buttons
  . r1=r0, r1&1, mouse.1=r1
  . r1=r0, r1&2, mouse.2=r1

  if mouse.1
    if not old.mouse.1, mouse.event='c'
      . mouse.drop=NO
      callf on.mouse
      return 1
    end
  else.if old.mouse.1, mouse.event='r'
    callf on.mouse
    if mouse.drag
      . mouse.drop=YES,\
      mouse.drop.x=mouse.x,\
      mouse.drop.y=mouse.y,\
      mouse.drag=NO
      . mouse.event='d'
      callf on.mouse
    end
    return 1
  end
  . r0=mouse.x, r1=old.mouse.x
  if r0<>r1
    .mouse.move:
    . mouse.event='m'
    callf on.mouse
    if mouse.1
      if not mouse.drag
	. mouse.drag=YES,\
	mouse.drag.x=mouse.x,\
	mouse.drag.y=mouse.y
      end
    else
      . mouse.drop=NO
    end
    return 1
  else
    . r0=mouse.y, r1=old.mouse.y
    if r0<>r1
      go .mouse.move
    end
  end
endf 0

 function select.box, box
  . r0=mouse.x, r1=mouse.y
  . r0-WINDOW.X, r1-WINDOW.Y
  callf point.inside, box, r0, r1
endf

macro if.select box { !if select.box, box }
macro else.if.select box
 { !else.if select.box, box }

;;;;;;;;;;;;;;;;;;;;; RANDOM ;;;;;;;;;;;;;;;;;;;;;

align integer @seed

; generate unique random number: 0-n

function random, n
  . r0=@seed
  if false
    rdtsc
    . @seed=r0
  end
  . r0*343FDh, r0+269EC3h,\
  @seed=r0, r0>>16, r0&7FFFh,\
  r1=n, r1+1, r0/r1, r0=r2
endf

; random(from-to-2)+from

function random.x, from, to
  . r0=from, r0-to, r0-2
  random r0
  . r0+from
endf

;;;;;;;;;;;;;;;;;;;;;; MAIN ;;;;;;;;;;;;;;;;;;;;;;

; main entry. detect current input states,
; translate and redirect to on.event handlers.

; drawing has slight flicker. no double
; buffer yet. kolibri only supports misaligned
; 24BPP images :( images should be stored in
; memory as 1-8BPP or 16/32BPP for fast access

!main:
  callf set.screen,\
   SCREEN.W, SCREEN.H, SCREEN.BPP
  callf on.create

  get.event:
  wait.event
  . event.type=r0

  update.mouse

  if timer.speed<>-1
    . timer.counter--
    if timer.counter<=0
      . timer.counter=timer.speed
      callf on.timer
    end
  end

  if event.type=EVENT.REDRAW
;    draw.begin
    callf draw.window
    callf on.draw
;    draw.end
;    retrace ; ?

  else.if event.type=EVENT.KEY
    mint 2
    if r0<>1, r0>>8, r0&0FFh, key=r0
      . key.event='k'
      callf on.key
      . key.event='c'
      callf on.key
      if key=KEY.ESCAPE
	callf exit
      end
    end

  else.if event.type=EVENT.BUTTON
    callf exit
  end
go get.event

function exit
  callf on.exit
  mint -1
endf