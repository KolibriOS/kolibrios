-- cell colour reducer - jan'11, from Paulo Silva, with help from people from GrafX2 google group (DawnBringer, Adrien Destugues (PulkoMandy), and Yves Rizoud)
-- This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 of the License. See <http://www.gnu.org/licenses/>
w,h=getpicturesize()
ok,xcell,ycell=inputbox("Modify cell pixel size","xcell",8,1,16,4,"ycell",8,1,16,4);
if ok==true then
  function grayscaleindexed(c)
    r,g,b=getcolor(c);return math.floor((b*11+r*30+g*59)/100);end
  celcnt={};for n=0,255,1 do celcnt[n+1]=0;end -- Arraycounter must have initial value
  for y1=0,h-1,ycell do
    for x1=0,w-1,xcell do
      for i=0,255,1 do
        celcnt[i+1]=0;end
      for y2=0,ycell-1,1 do
        for x2=0,xcell-1,1 do
           x=x1+x2;y=y1+y2;u=getpicturepixel(x,y)
           celcnt[u+1]=celcnt[u+1]+(1000*xcell*ycell)+math.random(0,950);end;end
      ikattr=0;paattr=0;ikcnt=0;pacnt=0
      for i=0,255,1 do
        if ikcnt<celcnt[i+1] then ikcnt=celcnt[i+1];ikattr=i;end;end
      celcnt[ikattr+1]=0
      for i=0,255,1 do
        if pacnt<celcnt[i+1] then pacnt=celcnt[i+1];paattr=i;end;end
      if grayscaleindexed(ikattr)>grayscaleindexed(paattr) then tmpr=ikattr;ikattr=paattr;paattr=tmpr;end
      wmid=math.floor((grayscaleindexed(paattr)+grayscaleindexed(ikattr))/2)
      for y2=0,ycell-1,1 do
        for x2=0,xcell-1,1 do
          x=x1+x2;y=y1+y2;u=getpicturepixel(x,y)
          if u==ikattr then
            idou=ikattr
          elseif u==paattr then
            idou=paattr
          else
            idou=ikattr
            if grayscaleindexed(u)>wmid then idou=paattr;end
            end
          putpicturepixel(x,y,idou)
          end;end;end;end;end
