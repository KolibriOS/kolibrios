-- draw grid - indexed colour - Copyright 2010 Paulo Silva
-- This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 of the License. See <http://www.gnu.org/licenses/>
w,h=getpicturesize();
ok,xsiz,ysiz,c=inputbox("draw grid - indexed colour)","x size",8,1,64,5,"y size",8,1,64,6,"colour id",0,0,255,6);
if ok==true then
  for y=0,h-1,1 do
    for x=0,w-1,xsiz do
      putpicturepixel(x,y,c);end;end 
  for y=0,h-1,ysiz do 
    for x=0,w-1,1 do 
      putpicturepixel(x,y,c);end;end;end
