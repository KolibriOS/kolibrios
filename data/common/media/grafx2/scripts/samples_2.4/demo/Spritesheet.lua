--ANIM: Sprite Animator v0.15
--Spare page holds data - Plays on current
--by Richard Fhager

run("../libs/memory.lua")

arg=memory.load({XS=16,YS=16,SPACE=1,FRAMES=8,XOFF=0,YOFF=0,FPS=10})

OK, XS, YS, SPACE, FRAMES, XOFF, YOFF, FPS = inputbox("Sprite-Sheet Animator",
 "Sprite X-size",   arg.XS,    1, 256,0,
 "Sprite Y-size",   arg.YS,    1, 256,0,
 "Spacing",         arg.SPACE, 0,  32,0,
 "# of Frames",     arg.FRAMES,2, 100,0,
  "X-offset",        arg.XOFF,  0, 800,0,
  "Y-offset",        arg.YOFF,  0, 800,0,
 "Play Speed (FPS)",arg.FPS,   1,  60,0
);


if OK == true then

memory.save({XS=XS,YS=YS,SPACE=SPACE,FRAMES=FRAMES,XOFF=XOFF,YOFF=YOFF,FPS=FPS})

 MAXPLAYS = 100

 w,h = getpicturesize()
 OX = w / 2 - XS/2
 OY = h / 2 - YS/2

 for play = 1, MAXPLAYS, 1 do

 for f = 0, FRAMES-1, 1 do
  for y = 0, YS-1, 1 do
   for x = 0, XS-1, 1 do
    sx = x + XOFF + f * (XS + SPACE)
    sy = y + YOFF
    putpicturepixel(OX+x, OY+y, getsparepicturepixel(sx, sy))
   end
  end
  updatescreen(); if (waitbreak(1/FPS)==1) then return; end
 end

 end -- plays

end --OK

