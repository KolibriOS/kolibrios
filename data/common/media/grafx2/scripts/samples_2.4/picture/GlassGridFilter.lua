-- Glass grid filter - Copyright 2010 Paulo Silva
-- This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 of the License. See <http://www.gnu.org/licenses/>
w,h=getpicturesize();
ok,xsiz,ysiz=inputbox("message","xsize",8,0,64,5,"ysize",8,0,64,6);
if ok==true then
  for y1=0,h-1,xsiz do
    for x1=0,w-1,ysiz do
      for y2=0,(ysiz/2)-1,1 do
        for x2=0,xsiz-1,1 do
          c1=getpicturepixel(x1+x2,y1+y2);c2=getpicturepixel(x1+(xsiz-1)-x2,y1+(ysiz-1)-y2)
          putpicturepixel(x1+x2,y1+y2,c2);putpicturepixel(x1+(xsiz-1)-x2,y1+(ysiz-1)-y2,c1)
          end;end;end;end;end
