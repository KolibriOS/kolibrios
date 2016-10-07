-- flip picture - Copyright 2010 Paulo Silva
-- This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 of the License. See <http://www.gnu.org/licenses/>
w,h=getpicturesize();
ok,flipx,flipy=inputbox("flip picture","flip x",1,0,1,-1,"flip y",0,0,1,-1);
if ok==true then
  if flipx==1 then
    for y=0,h-1,1 do 
      for x=0,w/2,1 do 
        c1=getpicturepixel(x,y);c2=getpicturepixel(w-x-1,y) 
        putpicturepixel(x,y,c2);putpicturepixel(w-x-1,y,c1) 
        end;end 
  else
    for y=0,h/2,1 do 
      for x=0,w-1,1 do 
        c1=getpicturepixel(x,y);c2=getpicturepixel(x,h-y-1) 
        putpicturepixel(x,y,c2);putpicturepixel(x,h-y-1,c1) 
        end;end;end;end
