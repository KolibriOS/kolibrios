-- Copyright 2010 Paulo Silva
-- This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; version 2 of the License. See <http://www.gnu.org/licenses/>
w,h=getpicturesize();
ok,bitd=inputbox("colourspace from palette","bitdepth:",4,1,8,5);
if ok==true then
  bitd3=(2^bitd);bitd8=(2^(math.floor(bitd/2)));bitd9=(2^((math.floor((bitd-1)/2))+1))
  for y1=0,(bitd8-1),1 do 
    for x1=0,(bitd9-1),1 do 
      for y2=0,(bitd3-1),1 do 
        for x2=0,(bitd3-1),1 do 
          putpicturepixel(x1*bitd3+x2,y1*bitd3+y2,matchcolor((y2*255)/(bitd3-1),((y1*8+x1)*255)/(bitd3-1),(x2*255)/(bitd3-1))) 
          end;end;end;end;end
