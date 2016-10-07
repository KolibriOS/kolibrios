-- Draw isometric grid - Copyright 2010 Paulo Silva
-- This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 of the License. See <http://www.gnu.org/licenses/>
w,h=getpicturesize();
ok,gsiz,ik=inputbox("draw isometric grid","size",16,0,128,5,"colour",1,0,255,6);
if ok==true then
  for y=0,h-1,gsiz do
    for x=0,w-1,1 do
      putpicturepixel(x,y+(x/2)%gsiz,ik);
      end;end
  for y=0,h-1,gsiz do
    for x=0,w-1,1 do
      putpicturepixel(x+((gsiz/2)-1),y+(gsiz-1)-((x/2)%gsiz),ik);
      end;end;end
