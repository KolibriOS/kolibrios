--BRUSH Distortion: FishEye
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

-- This script was adopted from Evalion, a Javascript codecrafting/imageprocessing project
--http://goto.glocalnet.net/richard_fhager/evalion/evalion.html 

w, h = getbrushsize()


for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

         ox = x / w;
         oy = y / h;
         v = (math.cos((ox-0.5)*math.pi)*math.cos((oy-0.5)*math.pi))*0.85;
         ox = (1 + ox - (ox-0.5)*v) % 1; 
         oy = (1 + oy - (oy-0.5)*v) % 1;
     
         c = getbrushbackuppixel(math.floor(ox*w),math.floor(oy*h));
         putbrushpixel(x, y, c);
  end
end

