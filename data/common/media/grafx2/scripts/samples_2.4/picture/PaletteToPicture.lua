-- palette to picture - Copyright 2010 Paulo Silva
-- This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 of the License. See <http://www.gnu.org/licenses/>
w,h=getpicturesize(); 
ok,xsiz,ysiz=inputbox("palette to picture","x size",8,1,16,5,"y size",8,1,16,6);
if ok==true then
  for y1=0,7,1 do 
    for x1=0,31,1 do 
      for y2=0,ysiz-1,1 do 
        for x2=0,xsiz-1,1 do 
          putpicturepixel(x1*xsiz+x2,y1*ysiz+y2,y1+x1*8) 
          end;end;end;end;end
