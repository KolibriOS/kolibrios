; BINARY MASTER

WINDOW.W=360
WINDOW.H=492

include 'a.inc'

text t(256), title.t='Binary Master: %hh'

text help.t=+\
 'Fun Game for Programmers.' RET\
 'Click BITs. Count in binary.' RET\
 'Match the decimal number' RET\
 'in the red box to make rows' RET\
 'disappear. Press any key.' RET\
 'r=Reset. p=Pause. Esc=exit'

text pause.t=+\
 'Paused. Press p to continue' RET\
 'or r=Reset. Esc=exit'

text game.over.t=+\
 'Game over. Score: %hh.' RET\
 'Press any key'

align

integer scene, score
numeric SCENE.*, TITLE, PLAY,\
 PAUSE, GAME.OVER
numeric EASY=5000, NORMAL=4000, HARD=2000

BOX board, my.box
integer my.n, red.n, magic.n=10101101b
integer cbit.x, cbit.y, bits.h
numeric BIT.*, W=32, H=48

text my.numbers(8+4), red.numbers(8+4)

FONT main.font='font'

IMAGE bits.i='bits', bit1.i='1',\
 bit0.i='0', close.i='x'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

function random.byte
  locals n
  .r:
  random 3
  if r0<2
    random 16
  else.if r0=2
    random 128
  else.if r0=3
    random 255
  end
  . n=r0
  text.find red.numbers, n
  if true, go .r, end
  . r0=n
  if false, r0++, end
endf

function reset.game
  locals n, p
  . score=0, bits.h=1
  memory.zero my.numbers, 12
  memory.zero red.numbers, 12
  . n=8, p=red.numbers
  loop n
    random.byte
    . r1=p, *r1++=r0, p=r1
  endl
  set.box board, 4, 70, BIT.W*8, BIT.H*8
  . scene=SCENE.TITLE
endf

function on.create
  set.font main.font
  set.timer NORMAL
  reset.game
endf

function remove.byte, t, i
  locals n
  alias p=r0, q=r1, x=r2
  if i=7, go .new, end
  . p=t, p+i, q=p, q++, x=7, x-i, n=x
  loop n, *p++=*q++, endl
  .new:
  . p=my.numbers, *(p+7)=0
  random.byte
  . q=red.numbers, *(q+7)=r0
endf

function remove.row, i
  remove.byte my.numbers, i
  remove.byte red.numbers, i
  . bits.h--
  if bits.h<1, bits.h=1, end
endf

function check.numbers
  locals i
  . i=0
  while i<8, r0=my.numbers, r0+i
    . r1=*r0, r0=red.numbers
    . r0+i, r2=*r0
    if r1=r2, score+r1
      remove.row i
      return 1
    end
    . i++
  endw
endf 0

function draw.board
  locals i, n, x, y, w, h
  draw.image bits.i, 4, 35
  draw.image bits.i, 4, 457
   . x=0, y=0, w=32, h=48
  while y<8, x=0
    while x<8
      . r0=x, r0*w, r0+board.x
      . r1=y, r1*h, r1+board.y
      set.box my.box, r0, r1, w, h
      draw.box my.box, BLACK, GRAY
      . x++
    endw
    . r0=x, r0*w, r0+board.x
    . r1=y, r1*h, r1+board.y
    set.box my.box, r0, r1, 48, h
    draw.box.o my.box, WHITE
    . my.box.x+48
    draw.box.o my.box, RED
    . r0=y, r1=8, r1-bits.h
    if r0>=r1
      . r0=my.numbers, r1=y, r2=8
      . r2-bits.h, r1-r2, r0+r1
      . r1=*r0, my.n=r1
      . r0=red.numbers, r1=y, r2=8
      . r2-bits.h, r1-r2, r0+r1
      . r1=*r0, red.n=r1
      u2t my.n, t
      . my.box.x-40, my.box.y+11
      draw.text t, my.box.x, my.box.y
      . my.box.x+44
      u2t red.n, t
      draw.text t, my.box.x, my.box.y
    end
    . y++
  endw
endf

function draw.bit, n, x, y
  if n
    draw.image bit1.i, x, y
  else
    draw.image bit0.i, x, y
  end
endf

function draw.byte, n, x, y
  locals i
  . i=8
  loop i, r0=n, r1=i, r1--, r0>>cl, r0&1
    draw.bit r0, x, y
    . x+BIT.W
  endl
endf

function draw.my.numbers
  locals i, n, y
  . i=bits.h, y=404
  loop i, r0=my.numbers, r0+i, r0--
    . r0=*r0, n=r0
    draw.byte n, 4, y
    . y-BIT.H
  endl
endf

function draw.title.scene
  draw.text help.t, 16, 130
  draw.byte magic.n, 50, 300
endf

function draw.play.scene
  draw.board
  draw.my.numbers
endf

function draw.pause.scene
  draw.text pause.t, 16, 130
  draw.byte magic.n, 50, 300
endf

function draw.game.over
  print t, game.over.t, score
  draw.text t, 44, 170
  draw.byte magic.n, 50, 300
endf

function on.draw
  locals x, y, w, h
  clear.screen BLACK
  print t, title.t, score
  draw.text t, 4, 4
  draw.image close.i, 324, 4
  . r0=screen.w, r0--
  . r1=screen.h, r1--
  draw.outline 0, 0, r0, r1, GRAY
  if scene=SCENE.TITLE
    draw.title.scene
  else.if scene=SCENE.PLAY
    draw.play.scene
  else.if scene=SCENE.PAUSE
    draw.pause.scene
  else.if scene=SCENE.GAME.OVER
    draw.game.over
  end
endf

function on.key
  if key.event='c'
    if scene=SCENE.TITLE
      . scene=SCENE.PLAY
      go .draw
    end
    if scene=SCENE.GAME.OVER
      go .reset
    end
    if key='r'
      .reset:
      reset.game
      go .draw
    end
    if key='p'
      .pause:
      if scene=SCENE.PLAY
	. scene=SCENE.PAUSE
      else.if scene=SCENE.PAUSE
	. scene=SCENE.PLAY
      end
      go .draw
    end
    .draw:
    render
  end
endf

function on.mouse
  if.select board
    . r0=mouse.x, r0-WINDOW.X, r0-board.x
    . r1=BIT.W, r0/r1, cbit.x=r0
    . r0=mouse.y, r0-WINDOW.Y, r0-board.y
    . r1=BIT.H, r0/r1, cbit.y=r0
    if mouse.event='c'
      . r0=cbit.y, r1=8, r1-bits.h
      if r0>=r1, r0=my.numbers, r1=cbit.y
	. r2=8, r2-bits.h, r1-r2, r0+r1
	. r3=*r0, r2=1, r1=7, r1-cbit.x
	. r2<<cl, r3><r2, *r0=r3
      end
    end
  end
  if mouse.event='r'
    check.numbers
    go .draw
  end
  if mouse.event='c'
    . r0=&close.i.x
    if.select r0
      exit
    end
    if scene<>SCENE.PLAY
      reset.game
      . scene=SCENE.PLAY
    end
    .draw:
    render
  end
endf

function on.timer
  if scene<>SCENE.PLAY
    return
  end
  if mouse.1, return, end
  if bits.h<8, bits.h++
  else
    . scene=SCENE.GAME.OVER
  end
  render
endf

function on.exit
  ; ...
endf