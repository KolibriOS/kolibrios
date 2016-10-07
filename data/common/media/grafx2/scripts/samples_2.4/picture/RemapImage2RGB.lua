--SCENE: Remap pic to RGB, diag.dith
--by Richard Fhager 
--http://hem.fyristorg.com/dawnbringer/

-- Copyright 2010 Richard Fhager
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; version 2
-- of the License. See <http://www.gnu.org/licenses/>

-- Set Palette (to a predefined one)

colors = {{  0,  0,  0},
          {255,  0,  0},
          {  0,255,  0},
          {  0,  0,255}
         }


chm = {1,0,0} 

for c = 1, #colors, 1 do
  setcolor(c-1,colors[c][1],colors[c][2],colors[c][3]) 
end

for c = #colors, 255, 1 do
  setcolor(c,0,0,0) 
end



w, h = getpicturesize()

for y = 0, h - 1, 1 do

  for x = 0, w - 1, 1 do
    
    r,g,b = getbackupcolor(getbackuppixel(x,y));

      rn = r * chm[1+(y+0+x)%3]
      gn = g * chm[1+(y+1+x)%3]
      bn = b * chm[1+(y+2+x)%3]

      n = matchcolor(rn,gn,bn);

      putpicturepixel(x, y, n);

  end
end
