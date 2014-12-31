; KOLIBRI PUZZLE CHALLENGE

TILE.W=64
TILE.H=64
MAP.X=TILE.W
MAP.Y=TILE.H
MAP.W=8
MAP.H=8
PUZZLE.W=MAP.W*TILE.W
PUZZLE.H=MAP.H*TILE.H

WINDOW.W=PUZZLE.W+(TILE.W*2)
WINDOW.H=PUZZLE.H+(TILE.H*2)

include 'a.inc'

text title(64)='Kolibri Puzzle Challenge'

align

integer scene
numeric SCENE.*, TITLE, PLAY, SOLVED

integer solved, select.x, select.y

puzzle: db (MAP.W*MAP.H*4) dup(0)
numeric NORMAL, ROTATE.R, INVERT.XY, ROTATE.L

IMAGE piece.i
piece.pixels: db (TILE.W*TILE.H*4) dup(0)

BOX my.box, puzzle.box
integer grid.color=WHITE

IMAGE kolibri.i='kolibri', logo.i='logo2',\
 wood1.i='wood1', wood2.i='wood2',\
 close.i='x', solved.i='solved'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

get.random equ random.x ROTATE.R, ROTATE.L

function erase.puzzle
  memory.zero piece.pixels, (TILE.W*TILE.H*4)
endf

function randomize.puzzle
  locals q, n
  alias p=r0, x=r1
  . q=puzzle, n=(MAP.W*MAP.H)
  loop n
    get x=get.random
    . p=q, (u32) *p=x, q+4
  endl
endf

function reset.game
  randomize.puzzle
endf

function on.create
  erase.puzzle
  . scene=SCENE.TITLE
  set.box puzzle.box,\
   MAP.X, MAP.Y, PUZZLE.W, PUZZLE.H
endf

function is.solved
  locals n
  alias p=r0, x=r1
  . p=puzzle, n=(MAP.W*MAP.H)
  loop n, (u32) x=*p++
    if x<>NORMAL, return 0, end
  endl
endf 1

function get.piece, x, y
  . r0=y, r0*MAP.W, r0+x, r0*4, r0+puzzle
endf

function get.piece.rotate, x, y
  get.piece x, y
  . (u32) r0=*r0
endf

function set.piece.rotate, x, y, r
  if r>ROTATE.L, r=0, end
  get.piece x, y
  . r1=r, (u32) *r0=r1
  is.solved
  if true
    . scene=SCENE.SOLVED
  end
endf

function copy.piece, x, y
  locals w, h, pw
  alias p=r0, s=r1, n=r2
  . p=piece.pixels, piece.i.p=p
  . piece.i.w=TILE.W, piece.i.h=TILE.H
  . n=PUZZLE.W, n*4, pw=n
  . n=y, n*pw, n*TILE.W
  . s=x, s*TILE.W, s*4, n+s
  . s=kolibri.i.p, s+n
  . h=TILE.H
  loop h, w=TILE.W
    loop w, (u32) *p++=*s++
    endl
    . n=TILE.W, n*4, s-n, s+pw
  endl
endf

function draw.piece, px, py, x, y
  locals z
  get z=get.piece.rotate px, py
  copy.piece px, py
  if z=NORMAL
    draw.image piece.i, x, y
  else.if z=ROTATE.R
    draw.image.rr piece.i, x, y
  else.if z=INVERT.XY
    . r0=y, r0+TILE.H
    draw.image.ixy piece.i, x, r0
  else.if z=ROTATE.L
    draw.image.rl piece.i, x, y
  end
endf

function draw.puzzle
  locals x, y
  . y=0
  while y<8, x=0
    while x<8
      . r0=x, r0*TILE.W, r0+MAP.X
      . r1=y, r1*TILE.H, r1+MAP.Y
      draw.piece x, y, r0, r1
      . x++
    endw
    . y++
  endw
endf

function draw.grid
  locals x, y
  . y=0
  while y<8, x=0
    while x<8
      . r0=x, r0*TILE.W, r0+MAP.X
      . r1=y, r1*TILE.H, r1+MAP.Y
      draw.outline r0, r1,\
       TILE.W, TILE.H, grid.color
      . x++
    endw
  . y++
  endw
endf

function draw.wood.frame
  draw.image wood1.i, 0, 0
  draw.image wood1.i, 0, WINDOW.H-TILE.H
  draw.image wood2.i, 0, TILE.H
  draw.image wood2.i, WINDOW.W-TILE.W, TILE.H
  draw.image close.i, WINDOW.W-40, 8
endf

function on.draw
  draw.wood.frame
  draw.puzzle
  if scene=SCENE.TITLE
    draw.box.o puzzle.box, grid.color
  end
  if scene=SCENE.PLAY
    draw.grid
  end
  if scene=SCENE.SOLVED
    draw.image.v solved.i, 132, 13, WHITE
  else
    draw.image.v logo.i, 180, 13, WHITE
  end
endf

function on.key
  ; ...
endf

function get.select.xy
  . r0=mouse.x, r0-MAP.X, r0-WINDOW.X
  . r1=TILE.W, r0/r1, select.x=r0
  . r0=mouse.y, r0-MAP.Y, r0-WINDOW.Y
  . r1=TILE.H, r0/r1, select.y=r0
endf

function on.mouse
  locals r
  if mouse.event='c'
    . r0=&close.i.x
    if.select r0
      exit
    end
    if scene=SCENE.TITLE
      reset.game
      . scene=SCENE.PLAY
      go .draw
    end
    if scene=SCENE.PLAY
      if.select puzzle.box
        get.select.xy
        get r=get.piece.rotate \
         select.x, select.y
        . r++
        set.piece.rotate \
         select.x, select.y, r
        go .draw
      end
    end
    if scene=SCENE.SOLVED
      reset.game
      . scene=SCENE.TITLE
      go .draw
    end
    .draw:
    render
  end
endf

function on.timer
  ; ...
endf

function on.exit
  ; ...
endf