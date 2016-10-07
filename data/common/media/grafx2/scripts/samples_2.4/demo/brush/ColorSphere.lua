--BRUSH Scene: Sphere of pencolor v1.0
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

rp,gp,bp = getcolor(getforecolor())

for y = 0, h - 1, 1 do
  for x = 0, w - 1, 1 do

   -- Fractionalize image dimensions
   ox = x / w;
   oy = y / h;

   -- Sphere
   X = 0.5; Y = 0.5; Rd = 0.5 
   a = math.sqrt(math.max(0,Rd*Rd - ((X-ox)*(X-ox)+(Y-oy)*(Y-oy)))) * 1/Rd
  
   r = rp * a
   g = gp * a
   b = bp * a

   putbrushpixel(x, y, matchcolor(r,g,b));

  end
end
