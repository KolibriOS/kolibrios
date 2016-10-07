--BRUSH: Halfsize with smoothscaling
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

w, h = getbrushsize()

setbrushsize(math.floor(w/2),math.floor(h/2))

for x = 0, w - 1, 2 do
 for y = 0, h - 1, 2 do
   r1,g1,b1 = getcolor(getbrushbackuppixel(x,y));
   r2,g2,b2 = getcolor(getbrushbackuppixel(x+1,y));
   r3,g3,b3 = getcolor(getbrushbackuppixel(x,y+1));
   r4,g4,b4 = getcolor(getbrushbackuppixel(x+1,y+1));
 
   r = (r1 + r2 + r3 + r4 ) / 4;
   g = (g1 + g2 + g3 + g4 ) / 4;
   b = (b1 + b2 + b3 + b4 ) / 4;
      
   c = matchcolor(r,g,b);

   putbrushpixel(x/2, y/2, c);

 end
end


