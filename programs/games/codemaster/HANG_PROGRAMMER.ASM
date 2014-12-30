; HANG PROGRAMMER

WINDOW.W=720
WINDOW.H=512

include 'a.inc'
include 'include/words.txt'

text title.t(32)='Hang Programmer'
text t(256), t2(256)

integer scene
numeric SCENE.*, TITLE, PLAY, GAME.OVER

integer guesses, word.index
BOX my.box

text word.t(64), used.letters(16)

align

IMAGE8 \
 board.i='board', logo.i='logo',\
 stand1.i='stand1', stand2.i='stand2',\
 head.i='head', body.i='body',\
 arm1.i='arm1', arm2.i='arm2',\
 leg1.i='leg1', leg2.i='leg2',\
 smile.i='smile', money.i='money',\
 prize.i='1000'

IMAGE close.i='x'

text abc='ABCDEFGHIJKLM', xyz='NOPQRSTUVWXYZ'

text help.t='CLICK TO START',\
 winner.t='WINNER', looser.t='LOOSER'

text example.t='DYNAMIC BINARY TRANSLATION'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

function reset.game
  . guesses=0
  text.zero used.letters
  text.zero word.t
  . r0=word.index, r0*4, r0+words.pa
  . (u32) r0=*r0
  text.copy word.t, r0
  . word.index++
  if word.index>=(N.WORDS-1)
    . word.index=0
  end
endf

; exchange pointer array elements.
; words.pa[a] and [b]

function exchange.words, a, b
  alias pa=r0, i=r0, p=r1, q=r2, x=r3
  . pa=words.pa
  . p=a, p*4, p+pa, q=b, q*4, q+pa
  . (u32) i=*p, (u32) x=*q
  . (u32) *p=x, (u32) *q=i
endf

; do this once when game begins.
; initialize pointer array to words[0-n]
; then randomize indices

function initialize
  locals i, n, j
  alias p=r0, x=r1
  . p=words.pa, i=0, n=N.WORDS
  loop n, x=i, (u32) x=*(words.ta+r1*4)
    . (u32) *p++=x, i++
  endl
  . i=N.WORDS
  loop i
    get j=random (N.WORDS-1)
    exchange.words i, j
  endl
  . word.index=0 ; random (N.WORDS-1)
endf

; is winner? if all characters in
; word.t have been used. return
; winner.t/looser.t

function is.winner
  locals i, q, n, win
  alias p=r0, c=r1
  . q=word.t
  get n=text.n q
  . win=1
  loop n, p=q, c=*p++
    if c<>' '
      text.find used.letters, c
      if false
        . win=0, break
      end
    end
    . q++
  endl
  if win, r0=winner.t
  else, r0=looser.t, end
endf

function on.create
  initialize
  reset.game
endf

function insert.c, c
  if c>='a', c-32, end
  if c<'A', return, end
  if c>'Z', return, end
  text.find used.letters, c
  if true, return, end
  text.attach.c used.letters, c
  is.winner
  if r0=winner.t
    go .reset
  end
  text.find word.t, c
  if true, return, end
  . guesses++
  if guesses=6
    .reset: . scene=SCENE.GAME.OVER
  end
endf

; special "my" font

align
alphabet.p: dd \
 a.i, b.i, c.i, d.i, e.i, f.i, g.i, h.i, i.i,\
 j.i, k.i, l.i, m.i, n.i, o.i, p.i, q.i, r.i,\
 s.i, t.i, u.i, v.i, w.i, x.i, y.i, z.i

irps _,\
 a b c d e f g h i j k l m \
 n o p q r s t u v w x y z {
  IMAGE8 _#.i='a/'#`_
}

function index.my.c, c
  . r0=c, r0-'A', r0*4, r0+alphabet.p
  . (u32) r0=*r0
endf

function get.my.cw, c
  if c=' ', return 0, end
  index.my.c c
  . (u32) r0=*(r0+?image.w)
endf

function get.my.tw, t
  locals n, w
  alias p=r0, c=r1
  get n=text.n t
  . w=0
  loop n, p=t, c=*p
    get.my.cw c
    . w+r0, t++
  endl
endf w

function draw.my.c, c, x, y, co
  locals im, w
  if c=' ', return, end
  get im=index.my.c c
  . (u32) r1=*(r0+?image.w), w=r1
  draw.image.v8 im, x, y, co
endf w

function draw.my.text, t, x, y
  locals p, n
  get n=text.n t
  . p=t
  loop n, r0=p, r0=*r0
    draw.my.c r0, x, y, WHITE
    . x+r0, x+4, p++
  endl
endf

function draw.my.word, t, x, y
  locals p, n, w, c, co
  get n=text.n t
  . p=t
  loop n, r0=p, r0=*r0, c=r0
    get w=get.my.cw c
    text.find used.letters, c
    if true
      . r0=x, r0+18, r1=w, r1/2, r0-r1
      draw.my.c c, r0, y, WHITE
    end
    . r0=x, r0+2, r1=y, r1+48
    draw.box r0, r1, 32, 3, WHITE
    . r0=36, x+r0, p++
  endl
endf

function draw.message
  locals n, x, y, w
  get n=text.count.w word.t
  if n=0, n=1, end
  set.source word.t
  set.token t
  . x=250, y=80
  loop n
    skip.space
    copy.until C.SPACE
    set.box my.box, 250, 65, 390, 220
    text.n t
    . r0*36, w=r0, r0=my.box.x, r0+195
    . r1=w, r1/2, r0-r1
    draw.my.word t, r0, y
    . y+70
  endl
endf

function draw.letters, t, x, y
  locals n, p, c, co
  get n=text.n t
  . p=t
  loop n, r0=p, r0=*r0, c=r0
    index.my.c c
    . r0+?image.box
    memory.copy my.box, r0, 16
    . my.box.x=x, my.box.y=y
    if.select my.box
      if mouse.1
        insert.c c
      end
    end
    . co=WHITE
    text.find used.letters, c
    if true, co=409040h, end
    draw.my.c c, x, y, co
    . r0+4, x+r0, p++
  endl
endf

function draw.alphabet
  draw.letters abc, 255, 308
  draw.letters xyz, 248, 370
endf

function draw.man
  locals x, y
  . x=40, y=62
  . alpha.bias=A.DARK
  draw.image.v.8 stand1.i, x, y, WHITE
  . r0=x, r0+27
  draw.image.v.8 stand2.i, r0, y, WHITE
  . alpha.bias=A.DARKER
  if guesses>0
    . r0=x, r0+80, r1=y, r1+40
    draw.image.v.8 head.i, r0, r1, WHITE
  end
  if guesses>1
    . r0=x, r0+95, r1=y, r1+150
    draw.image.v.8 body.i, r0, r1, WHITE
  end
  if guesses>2
    . r0=x, r0+70, r1=y, r1+150
    draw.image.v.8 arm1.i, r0, r1, WHITE
  end
  if guesses>3
    . r0=x, r0+143, r1=y, r1+150
    draw.image.v.8 arm2.i, r0, r1, WHITE
  end
  if guesses>4
    . r0=x, r0+87, r1=y, r1+228
    draw.image.v.8 leg1.i, r0, r1, WHITE
  end
  if guesses>5
    . r0=x, r0+118, r1=y, r1+228
    draw.image.v.8 leg2.i, r0, r1, WHITE
  end
  . alpha.bias=0
endf

function draw.hang.man
  . guesses=6
  draw.man
  . guesses=0
endf

function draw.winner
  locals x, y
  . alpha.bias=A.DARKER
  . x=40, y=62
  . r0=x, r0+80, r1=y, r1+40
  draw.image.v.8 head.i, r0, r1, WHITE
  . r0=x, r0+95, r1=y, r1+150
  draw.image.v.8 body.i, r0, r1, WHITE
  . r0=x, r0+67, r1=y, r1+87
  draw.image.viy.8 arm1.i, r0, r1, WHITE
  . r0=x, r0+147, r1=y, r1+87
  draw.image.viy.8 arm2.i, r0, r1, WHITE
  . r0=x, r0+87, r1=y, r1+228
  draw.image.v.8 leg1.i, r0, r1, WHITE
  . r0=x, r0+118, r1=y, r1+228
  draw.image.v.8 leg2.i, r0, r1, WHITE
  . alpha.bias=A.LIGHT
  draw.image.v.8 smile.i, 547, 307, WHITE
  draw.image.v.8 smile.i, 310, 307, WHITE
  . alpha.bias=A.DARK
  draw.image.v.8 money.i, 547, 342, 0DDFFDDh
  draw.image.v.8 prize.i, 367, 377, WHITE
  . alpha.bias=0
endf

function draw.title.scene
  draw.my.text help.t, 237, 100
  draw.hang.man
endf

function draw.play.scene
  draw.message
  draw.alphabet
  draw.man
endf

function draw.game.over
  locals p
  get p=is.winner
  draw.my.text p, 362, 308
  draw.message
  if p=winner.t
    draw.winner
  else
    draw.hang.man
  end
endf

function on.draw
  clear.screen 283D25h
  draw.image.8 board.i, 0, 0
  . alpha.bias=A.DARKER
  draw.image.v.8 logo.i, 240, 32, WHITE
  . alpha.bias=0
  if scene=SCENE.TITLE
    draw.title.scene
  else.if scene=SCENE.PLAY
    draw.play.scene
  else.if scene=SCENE.GAME.OVER
    draw.game.over
  end
  draw.image close.i, 663, 28
endf

function on.key
  if key.event='k'
    if scene=SCENE.PLAY
      insert.c key
    else.if scene=SCENE.TITLE
      reset.game
      . scene=SCENE.PLAY
    else.if scene=SCENE.GAME.OVER
      reset.game
      . scene=SCENE.TITLE
    end
    render
  end
endf

function on.mouse
  if mouse.event='c'
    . r0=&close.i.x
    if.select r0
      exit
    end
    if scene=SCENE.TITLE
      reset.game
      . scene=SCENE.PLAY
    else.if scene=SCENE.GAME.OVER
      reset.game
      . scene=SCENE.TITLE
    end
    render
  end
endf

function on.timer
  ; ...
endf

function on.exit
  ; ...
endf