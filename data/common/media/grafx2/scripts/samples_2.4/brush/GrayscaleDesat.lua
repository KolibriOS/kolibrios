--BRUSH Remap: Grayscale (desaturate)
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


percent = 100

--
function desaturate(percent,r,g,b) -- V1.0 by Richard Fhager
 p = percent / 100
 a = (math.min(math.max(r,g,b),255) + math.max(math.min(r,g,b),0)) * 0.5 * p
 r = r + (a-r*p)
 g = g + (a-g*p)
 b = b + (a-b*p)
 return r,g,b
end
--


w, h = getbrushsize()

for x = 0, w - 1, 1 do
 for y = 0, h - 1, 1 do
   putbrushpixel(x, y, matchcolor(desaturate(percent,getcolor(getbrushpixel(x,y)))));
 end
end
