--PALETTE Set: Full 6 Bit (64 colors)
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>


-- Generate palette of all colors possible with a given number of shades for each channel
-- 2 shades = 1 bit / channel = 3 bit palette = 2^3 colors =  8 colors
-- 4 shades = 2 bit / channel = 6 bit palette = 2^6 colors = 64 colors

-- Channel shades (shades = 2 ^ bit-depth)
shades = 4

mult = 255 / (shades-1)


colors = {}
col = 0
for r = 0, shades-1, 1 do
  for g = 0, shades-1, 1 do
    for b = 0, shades-1, 1 do
       col = col + 1
       colors[col] = { r*mult, g*mult, b*mult } 
    end
  end
end


for c = 1, #colors, 1 do

  setcolor(c-1,colors[c][1],colors[c][2],colors[c][3]) 

end
