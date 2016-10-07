--BRUSH Remap: Grayscale (average)
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

w, h = getbrushsize()

for x = 0, w - 1, 1 do
 for y = 0, h - 1, 1 do
   
   r, g, b = getcolor(getbrushpixel(x,y))

   a = (r+g+b)/3

   putbrushpixel(x, y, matchcolor(a,a,a));

 end
end
